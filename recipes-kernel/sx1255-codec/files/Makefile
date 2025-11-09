obj-m += sx1255-codec.o

# Default target - let kernel build system handle the module compilation
all:
	$(MAKE) -C $(KERNEL_PATH) M=$(PWD) modules

# Minimal clean target required by BitBake's module class
clean:
	rm -f *.o *.ko *.mod.c *.mod *.order *.symvers .*.cmd
	rm -rf .tmp_versions/

.PHONY: all clean
