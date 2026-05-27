# GT911 polling touchscreen driver
# Build anywhere: just run 'make'

obj-m := gt911_poll.o

KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

all: modules

modules:
	@[ -d "$(KERNEL_SRC)" ] || { \
		echo "ERROR: Kernel headers not found at $(KERNEL_SRC)"; \
		echo "Install with: sudo apt install linux-headers-$(shell uname -r)"; \
		exit 1; \
	}
	$(MAKE) -C $(KERNEL_SRC) M=$(CURDIR) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(CURDIR) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(CURDIR) clean
