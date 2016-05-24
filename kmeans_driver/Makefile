obj-m := FPGA_driver.o
FPGA_driver-objs := buffer.o FPGA.o
KERNELDIR := /lib/modules/2.6.32/build
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
clean:
	rm FPGA.o buffer.o FPGA_driver.o FPGA_driver.ko FPGA_driver.ko.unsigned FPGA_driver.mod.c FPGA_driver.mod.o  modules.order Module.symvers
