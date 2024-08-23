//Versiyon çalışan (6 numaralı işlemle poll e girip timer ile uyanan)
//poll yapmazsan kendin sürekli giriş oldu mu tespit etmen lazım. while(1)
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/wait.h> 
#include <linux/time.h>
#include <linux/delay.h>
//#include <include\inttypes.h>
//#include <scull.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/slab.h>


#define DEVICE_NAME "alperen"
#define BUFFER_SIZE 150000 //cat /dev/alperen komutu stackn overflow hatası veriyordu. stacki büyüttüm.
#define WR_VALUE  _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

static char kernel_buffer[BUFFER_SIZE];
static int temperature ;
static int timer_msec=5*1000 ;
static int majorNumber;
static struct class* myCharClass = NULL;
static struct device* myCharDevice = NULL;


static struct timer_list my_timer; 
static wait_queue_head_t my_queue;//  POLL İÇİN EKLENELER
static int flag = 0;





static int dev_open(struct inode *inodep, struct file *filep){
	
    printk(KERN_INFO "alperen: Device has been opened\n");
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep){
	del_timer(&my_timer);
    printk(KERN_INFO "alperen: Device successfully closed\n");
    
    return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){

    printk(KERN_INFO "alperen: Reading from device\n");	    
    int i = copy_to_user( buffer , kernel_buffer , 100 );
    if ( i!=0 ) 
    {
    	printk(KERN_ERR "Okuma islemi basarisiz\n");
   	 return -EFAULT;
    }
    else
    printk(KERN_INFO "%lu byte basarili bir sekilde okundu\n", len);
    printk(KERN_INFO "%s",kernel_buffer);
    return 0;
}

static ssize_t dev_write (struct file *filep, const char *buffer, size_t len, loff_t *offset){
	
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(timer_msec));
	
    printk(KERN_INFO "alperen: Writing to device\n");
    int k = copy_from_user( kernel_buffer , buffer , len );
    printk("kernel_buffer icerisinde bulunan veri: %s",kernel_buffer);
    
    if( k !=0 )
    {
    	printk(KERN_ERR "Kullanicidan veri yazilamadi.\n");
    	return -EFAULT;
    };
    
     // kernel_buffer içerisine bufferdan gelen(app de write komutu icindeki ikinci parametre torba buradaki buffer'a karsilik geliyor. Len de sizeof(torba)dır aslında)
    printk(KERN_INFO "%lu byte basarili bir sekilde yazildi\n",len);
   
   
    return len;
} 

static long dev_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case WR_VALUE :  //kullanıcıdan yazma ..Şuan çalışmıyor
                        printk(KERN_INFO "alperen: Timer data write ioctl");
                        if( copy_from_user( &timer_msec ,(int *) arg, sizeof(timer_msec)) )
                        {
                        printk(KERN_ERR "alperen: write error!");
                         pr_err(" Data Write : Err!\n");
                        }
                        
                        pr_info("Yeni timer degeri  %d yapilmistir!\n", timer_msec);
                        break;
                        
                case RD_VALUE: //kullanıcıya yazma
                        printk(KERN_INFO "alperen: Timer Data read\n");
                        if( copy_to_user(  (int32_t *) arg, &timer_msec, sizeof(timer_msec)) )
                        {
                                pr_err("timer Read by ioctl : Err!\n");
                        }
                        else
                        pr_info("Timer degeri basarili bir sekilde user space'e aktarildi");
                        break;
                
        }
        return 0;
}

static void timer_callback(struct timer_list *timer) {
	flag = 1;
	wake_up_interruptible(&my_queue); //wait_queue yi boşaltır. Tekrar polle sokar
	printk(KERN_INFO "alperen: timer callback çalıştı.");
}
	 
static unsigned int mychardev_poll(struct file *file, poll_table *wait) {
	
	__poll_t mask =0;

	poll_wait(file, &my_queue, wait);	
    if (flag) 
    {
        flag = 0;
        mask |= (POLLIN | POLLRDNORM); // yazılıp yazılmadığını sen okuyabilirsin!!
        return mask;
    }
    else
    {
	mask |= 0;
	return  mask; //return 0 nereye  döndürüyor seni?
    }	
}


static struct file_operations fops =
{
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
    .unlocked_ioctl = dev_ioctl,
    .poll = mychardev_poll,
      
};

static int __init mychar_init(void){
    printk(KERN_INFO "alperen: Initializing the MyChar LKM\n");

	

    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0){
        printk(KERN_ALERT "alperen failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "alperen: registered correctly with major number %d\n", majorNumber);

    myCharClass = class_create(DEVICE_NAME);
    if (IS_ERR(myCharClass)){
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(myCharClass);
    }
    printk(KERN_INFO "alperen: device class registered correctly\n");

    myCharDevice = device_create(myCharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(myCharDevice)){
        class_destroy(myCharClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(myCharDevice);
    }
    printk(KERN_INFO "alperen: device class created correctly\n");
    
     init_waitqueue_head(&my_queue);

     timer_setup(&my_timer, timer_callback, 0); // timer setup sayacı başlatmaz 
    
   
    
    return 0;
}

static void __exit mychar_exit(void){
    device_destroy(myCharClass, MKDEV(majorNumber, 0));
    class_unregister(myCharClass);
    class_destroy(myCharClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "alperen: Goodbye from the LKM!\n");
    del_timer(&my_timer); 
}

module_init(mychar_init);
module_exit(mychar_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Linux char driver");
MODULE_VERSION("0.1");

