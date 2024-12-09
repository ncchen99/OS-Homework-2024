TARGET_MODULE := My_Kernel

obj-m += $(TARGET_MODULE).o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
CC := gcc

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	$(CC) -o 3_2.out 3_2.c -lpthread

clean:
	@rm -f 3_2.out 3_2_Config.h
	@rm -f *.o *.ko *.mod.* *.symvers *.order *.mod.cmd *.mod .*.mod.* .*.*.cmd
	$(MAKE) -C $(KDIR) M=$(PWD) clean

Prog_1thread:
	@echo "#define THREAD_NUMBER 1" > 3_2_Config.h
	$(MAKE) all
	@sudo ./3_2.out single
	@rm -f 3_2.out 3_2_Config.h

Prog_2thread:
	@echo "#define THREAD_NUMBER 2" > 3_2_Config.h
	$(MAKE) all
	@sudo ./3_2.out multi
	@rm -f 3_2.out 3_2_Config.h

load:
	@sudo insmod $(TARGET_MODULE).ko

unload:
	@sudo rmmod $(TARGET_MODULE).ko
