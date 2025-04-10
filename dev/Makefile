obj-m += serial_reader.o

# Set the kernel build path
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# All target
all:
	make -C $(KDIR) M=$(PWD) modules

# Clean target
clean:
	make -C $(KDIR) M=$(PWD) clean
