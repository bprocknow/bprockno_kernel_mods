# Build brview.ko from multiple translation units.
obj-m += brview.o
brview-y := brview_main.o brview_sysfs.o

KVER ?= $(shell uname -r)

# Path to the prepared kernel build directory.
KDIR ?= /lib/modules/$(KVER)/build

# Path to the full Fedora kernel source tree. Fedora's packaged build tree does
# not install net/bridge/br_private.h, but this module includes it directly.
KSRCDIR ?= /home/mainserver/repo/bprockno_kernel_mods/kernel-source-6.19.12-200.fc43/rpmbuild/BUILD/kernel-6.19.12-build/kernel-6.19.12/linux-6.19.12-200.fc43.x86_64
ccflags-y += -I$(KSRCDIR)/net/bridge

# Default target
all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

# Clean up generated files
clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
