CONFIG_MODULE_SIG=n
obj-m = sema_device_driver.o

KBUILD_CFLAGS += -w

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules $(KBUILD_CFLAGS)
	gcc producer_numbers.c -o pro.o
	gcc consumer_numbers.c -o con.o
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
