# Build for the running kernel
# Usage: make -C /lib/modules/$(uname -r)/build M=$(pwd) modules

obj-m := gt911_poll.o

KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean

# For cross-compile on Windows to Pi:
# Set KERNEL_SRC to the Pi's kernel headers path after copying source files
