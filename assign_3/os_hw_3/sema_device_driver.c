#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/sched.h>

//Author and Description
#define AUTHOR "Vaibhav Chauhan"
#define DESCRIPTION "CHARACTER DEVICE"
#define LICENSE "GPL"
MODULE_LICENSE(LICENSE);
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);

static struct miscdevice sema_device;//kernel device implemented

static int num_device_open;//number of devices of type custom device open 
static int buffer_size;//size of buffer
char* sema_device_name;//name of kernel device

static int buffer_read_index = 0;//index for the value to be read
static int buffer_write_index = 0;//index for the array slot where value is to be written
static int buffer_empty_slots;//empty slots in buffer

static struct semaphore full;//semaphore for if buffer is full
static struct semaphore empty;//semaphore for if buffer is empty
static struct semaphore mutex;//semaphore for access of critical section

module_param(buffer_size, int, 0000);//input of buffer size when module is installed
module_param(sema_device_name, charp, 0000);//input of device name when module is installed

int* buffer;//address of buffer will be stored here

static int dev_open(struct inode*, struct file*);
static ssize_t dev_read(struct file*, int*, size_t, loff_t*);
static ssize_t dev_write(struct file*, int*, size_t, loff_t*);
static int dev_release(struct inode*, struct file*);

static struct file_operations sema_device_fops = 
{
	.open = &dev_open,
	.read = &dev_read,
	.write = &dev_write,
	.release = &dev_release
};

//run when open system call is used
static int dev_open(struct inode* _inode, struct file* _file)
{//when open system call is called
	printk(KERN_ALERT "Device opened");
	num_device_open = num_device_open + 1;//total number of devices open
	return 0;
}

//run at time of uninstallation
static int dev_release(struct inode* _inode, struct file* _file)
{//when device is closed
	printk(KERN_ALERT "Device closed");
	num_device_open = num_device_open - 1;
	return 0;
}

//module is run when uninstallation is complete
void cleanup_module()
{
	kfree(buffer);
	misc_deregister(&sema_device);
	printk(KERN_INFO "Device %s Unregistered!\n", sema_device_name);
}

//when module is installed
int init_module()
{
	sema_device.name = sema_device_name;//device name assigned
	sema_device.minor = MISC_DYNAMIC_MINOR;//misc number assigned
	sema_device.fops = &sema_device_fops;//methods assigned
	
	int ret = misc_register(&sema_device);

	if(ret)
	{
		printk(KERN_ERR "Device not registered\n");
		return ret;
	}
	printk(KERN_INFO "Device Registered!\ndevice name: %s\nbuffer size: %d\n",sema_device_name,buffer_size);

	buffer = (int*)kmalloc(buffer_size*sizeof(int), GFP_KERNEL);//allocation of memory for buffer

	num_device_open = 0;//count of number of device opened is initialized 

	sema_init(&full, 0);//semaphore variable full initialized to 0 because no slot is full
	sema_init(&empty, buffer_size);//semaphore variable empty initialized to buffer size because no slot is full 
	sema_init(&mutex, 1);//semaphore variable mutex initialized to 1 so that critical section is available

	buffer_empty_slots = buffer_size;//count of empty slots in buffer

	return 0;
}

//when system call read is used
static ssize_t dev_read(struct file* _file, int* user_buffer, size_t number_of_chars_to_be_read, loff_t* offset)
{
	if(down_interruptible(&full) < 0)
	{//if resource is not taken and kernel device is interrupted
		printk(KERN_ALERT "Interrupted by user");
		return -1;
	}

	if(down_interruptible(&mutex) < 0)
	{//if resource is not taken and kernel device is interrupted
		printk(KERN_ALERT "Interrupted by user");
		return -1;
	}
	
	//start of critical section

	buffer_read_index = buffer_read_index % buffer_size;//mod operation used so that correct buffer index is used

	if(buffer_empty_slots < buffer_size)
	{
		int ret = copy_to_user(user_buffer, &buffer[buffer_read_index], sizeof(buffer[buffer_read_index]));
		
		if(ret < 0)
		{//if there was error in transfer of data
			printk(KERN_INFO "Error in copying from buffer to user");
			return ret;
		}
		printk(KERN_INFO "Reading %d\n", buffer[buffer_read_index]);

		++buffer_read_index;//increment read index to point to next data
		++buffer_empty_slots;//empty slots increased as data is read
	}

	//end of critical section
	up(&mutex);//give up lock	
	up(&empty);//increase empty semaphore

	return sizeof(buffer[buffer_read_index]);
}

//when write system call is called
static ssize_t dev_write(struct file* _file,int* user_buffer, size_t number_of_chars_to_write, loff_t* offset)
{
	if(down_interruptible(&empty) < 0)
	{//if resource is not saved and kernel device is interrupted
		printk(KERN_ALERT "Interrupted by user");
		return -1;
	}

	if(down_interruptible(&mutex) < 0)
	{//if resource is not saved and kernel device is interrupted
		printk(KERN_ALERT "Interrupted by user");
		return -1;
	}
	
	//start of critical section

	buffer_write_index = buffer_write_index % buffer_size;

	if(buffer_empty_slots >= 0)
	{
		int ret = copy_from_user(&buffer[buffer_write_index], user_buffer, sizeof(buffer[buffer_write_index]));

		if(ret < 0)
		{//if there was error in transfer of data
			printk(KERN_INFO "Error in copying from buffer to user");
			return ret;
		}
		printk("Written %d %d",buffer_write_index,buffer[buffer_write_index]);

		++buffer_write_index;//increment write index to point to next empty slot
		--buffer_empty_slots;//decrease empty slot as new data is written
	}

	//end of critical section
	up(&mutex);//give up lock
	up(&full);//increase full semaphore

	return sizeof(buffer[buffer_write_index]);
}
