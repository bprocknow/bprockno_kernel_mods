# Define the object file to be built
obj-m += brview.o

KVER ?= 6.19.10-200.fc43.x86_64

# Path to the prepared kernel build directory.
KDIR ?= /lib/modules/$(KVER)/build

# Path to the full Fedora kernel source tree. Fedora's packaged build tree does
# not install net/bridge/br_private.h, but this module includes it directly.
KSRCDIR ?= /home/mainserver/repo/fedora_kernels/linux-6.19.10
ccflags-y += -I$(KSRCDIR)/net/bridge

# Default target
all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

# Clean up generated files
clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
