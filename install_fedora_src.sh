#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage:
  ./fetch-fedora-kernel-source.sh <kernel-release>

Example:
  ./fetch-fedora-kernel-source.sh 6.19.12-200.fc43

What this script does:
  1. Clones Fedora kernel dist-git for the matching Fedora branch using fedpkg
  2. Downloads lookaside sources with fedpkg sources
  3. Downloads the exact matching kernel src.rpm from Koji
  4. Extracts the src.rpm contents
  5. Runs rpmbuild -bp to produce the prepared full source tree

Output layout:
  ./kernel-source-<kernel-release>/
    distgit/         Fedora kernel dist-git checkout
    srpm/            downloaded src.rpm
    srpm-contents/   files extracted from the src.rpm
    rpmbuild/        rpmbuild tree
    prepared/        prepared kernel source tree path info

Requirements:
  Fedora packages:
    fedpkg
    koji
    rpm-build
    rpmdevtools
    cpio
    xz
    zstd
    git
EOF
}

die() {
    echo "ERROR: $*" >&2
    exit 1
}

need_cmd() {
    command -v "$1" >/dev/null 2>&1 || die "Required command not found: $1"
}

if [[ $# -ne 1 ]]; then
    usage
    exit 1
fi

INPUT_RELEASE="$1"

# Expect something like: 6.19.12-200.fc43
if [[ ! "$INPUT_RELEASE" =~ ^([0-9]+\.[0-9]+(\.[0-9]+)?)-([0-9.]+)\.fc([0-9]+)$ ]]; then
    die "Input must look like 6.19.12-200.fc43"
fi

VERSION_PART="${BASH_REMATCH[1]}"
RELEASE_NUM="${BASH_REMATCH[3]}"
FEDORA_NUM="${BASH_REMATCH[4]}"

FEDPKG_BRANCH="f${FEDORA_NUM}"
NVR="kernel-${INPUT_RELEASE}"

need_cmd fedpkg
need_cmd koji
need_cmd git
need_cmd rpmbuild
need_cmd rpm2cpio
need_cmd cpio
need_cmd find

WORKDIR="${PWD}/kernel-source-${INPUT_RELEASE}"
DISTGIT_DIR="${WORKDIR}/distgit"
SRPM_DIR="${WORKDIR}/srpm"
SRPM_CONTENTS_DIR="${WORKDIR}/srpm-contents"
RPMBUILD_TOPDIR="${WORKDIR}/rpmbuild"
PREPARED_DIR="${WORKDIR}/prepared"

mkdir -p "$WORKDIR" "$SRPM_DIR" "$SRPM_CONTENTS_DIR" "$RPMBUILD_TOPDIR" "$PREPARED_DIR"

echo "==> Input release:        $INPUT_RELEASE"
echo "==> Fedora branch:        $FEDPKG_BRANCH"
echo "==> Expected kernel NVR:  $NVR"
echo "==> Work directory:       $WORKDIR"

if [[ ! -d "$DISTGIT_DIR/.git" ]]; then
    echo "==> Cloning Fedora kernel dist-git branch $FEDPKG_BRANCH with fedpkg"
    (
        cd "$WORKDIR"
        fedpkg clone --anonymous --branch "$FEDPKG_BRANCH" kernel distgit
    )
else
    echo "==> Reusing existing dist-git checkout: $DISTGIT_DIR"
fi

echo "==> Downloading lookaside sources with fedpkg"
(
    cd "$DISTGIT_DIR"
    fedpkg sources
)

echo "==> Downloading exact src.rpm from Koji for $NVR"
(
    cd "$SRPM_DIR"
    rm -f ./*.src.rpm
    koji download-build --arch=src "$NVR"
)

SRPM_PATH="$(find "$SRPM_DIR" -maxdepth 1 -type f -name '*.src.rpm' | head -n1)"
[[ -n "$SRPM_PATH" ]] || die "Could not find downloaded src.rpm in $SRPM_DIR"

echo "==> Downloaded src.rpm:"
echo "    $SRPM_PATH"

echo "==> Extracting src.rpm contents"
rm -rf "${SRPM_CONTENTS_DIR:?}/"*
(
    cd "$SRPM_CONTENTS_DIR"
    rpm2cpio "$SRPM_PATH" | cpio -idmv
)

SPEC_PATH="$(find "$SRPM_CONTENTS_DIR" -maxdepth 1 -type f -name '*.spec' | head -n1)"
[[ -n "$SPEC_PATH" ]] || die "No .spec file found after extracting src.rpm"

echo "==> Found spec file:"
echo "    $SPEC_PATH"

mkdir -p \
    "$RPMBUILD_TOPDIR/BUILD" \
    "$RPMBUILD_TOPDIR/BUILDROOT" \
    "$RPMBUILD_TOPDIR/RPMS" \
    "$RPMBUILD_TOPDIR/SOURCES" \
    "$RPMBUILD_TOPDIR/SPECS" \
    "$RPMBUILD_TOPDIR/SRPMS"

echo "==> Populating rpmbuild tree"

cp -a "$SPEC_PATH" "$RPMBUILD_TOPDIR/SPECS/"

# Copy files extracted from the src.rpm
find "$SRPM_CONTENTS_DIR" -maxdepth 1 -type f ! -name '*.spec' -exec cp -a {} "$RPMBUILD_TOPDIR/SOURCES/" \;

# Also copy fedpkg lookaside sources, which may be needed by the spec prep stage
find "$DISTGIT_DIR" -maxdepth 1 -type f \
    ! -name '*.spec' \
    ! -name '.gitignore' \
    ! -name 'sources' \
    ! -name '*.patch' \
    -exec cp -an {} "$RPMBUILD_TOPDIR/SOURCES/" \; || true

# Copy patches and source-related files from distgit too
find "$DISTGIT_DIR" -maxdepth 1 -type f \
    \( -name '*.patch' -o -name '*.diff' -o -name '*.tar.*' -o -name '*.xz' -o -name '*.gz' -o -name '*.bz2' -o -name '*.zst' -o -name '*.conf' -o -name '*.cfg' -o -name 'sources' \) \
    -exec cp -an {} "$RPMBUILD_TOPDIR/SOURCES/" \; || true

echo "==> Running rpmbuild -bp to prepare the full kernel source tree"
rpmbuild \
    --define "_topdir $RPMBUILD_TOPDIR" \
    -bp "$RPMBUILD_TOPDIR/SPECS/$(basename "$SPEC_PATH")"

BUILD_SUBDIR="$(find "$RPMBUILD_TOPDIR/BUILD" -mindepth 1 -maxdepth 1 -type d | head -n1 || true)"

if [[ -z "$BUILD_SUBDIR" ]]; then
    die "rpmbuild -bp completed, but no prepared source directory was found in $RPMBUILD_TOPDIR/BUILD"
fi

echo "$BUILD_SUBDIR" > "$PREPARED_DIR/source-tree-path.txt"

echo
echo "SUCCESS"
echo "Prepared kernel source tree:"
echo "  $BUILD_SUBDIR"
echo
echo "Other artifacts:"
echo "  dist-git checkout:   $DISTGIT_DIR"
echo "  downloaded src.rpm:  $SRPM_PATH"
echo "  src.rpm contents:    $SRPM_CONTENTS_DIR"
echo "  rpmbuild topdir:     $RPMBUILD_TOPDIR"
echo "  saved path file:     $PREPARED_DIR/source-tree-path.txt"
