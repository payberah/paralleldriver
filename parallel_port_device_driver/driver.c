#define MODULE
#define __KERNEL__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/wrapper.h>
#include <asm/uaccess.h>  
#include <asm/io.h>

#define DEVICE_NAME "char_dev"
#define PORT1 0x378
#define PORT2 0x379
#define PORT3 0x37A

static int Device_Open = 0;
char R_DATA;
wait_queue_head_t q;

//--------------------------------------------------------------------
void irq_handle(int irq, void* dev, struct pt_regs* regs) {
  R_DATA = inb(PORT2);
  wake_up_interruptible(&q);
}

//--------------------------------------------------------------------
static int device_open(struct inode *inode, struct file *file) {
  int irq;
  static int counter = 0;
  irq = request_irq(7, irq_handle, SA_INTERRUPT, "bingooli", NULL);
  outb(inb(0x21) & (0x80), 0x21);
  outb(0x20, 0x20);
  outb(inb(PORT3) | 0x10, PORT3);
  printk("\ndevice_open ...\n");

  if (Device_Open)
    return -EBUSY;

  Device_Open++;
  MOD_INC_USE_COUNT;

  return 0;
}

//--------------------------------------------------------------------
static int device_release(struct inode *inode, struct file *file) {
  free_irq(7, NULL);
  Device_Open--;
  MOD_DEC_USE_COUNT;

  return 0;
}

//--------------------------------------------------------------------
static ssize_t device_read(struct file *file, char *buffer, size_t length, loff_t *offset) {
  char ch1, ch2;

  outb(inb(PORT3) | (0x18), PORT3);
  outb(inb(PORT3) & (0xfb), PORT3);

  printk("\nwait ...\n");
  interruptible_sleep_on(&q);
  printk("\nOK, Data was read ...\n");
  ch1 = (128 - (R_DATA & 0x80)) / 16;
  ch2 = R_DATA >> 3;
  ch2 = ch2 & 0x07;
  ch2 = ch2 | ch1;
  put_user(ch2, buffer);
  outb (inb(PORT3) | 0x14, PORT3);

  return 0;
}

//--------------------------------------------------------------------
static ssize_t device_write(struct file *file, const char *buffer, size_t length, loff_t *offset) {
  int i;
  char W_DATA;

  outb(inb(PORT3) | (0x11), PORT3);
  outb(inb(PORT3) & (0xf7), PORT3);
  get_user(W_DATA, buffer);
  outb(W_DATA, PORT1);
  printk("\ndata = %c\n", W_DATA);
  outb(inb(PORT3) & (0xfc), PORT3);

  return -EINVAL;
}

//--------------------------------------------------------------------
static int Major;
struct file_operations Fops = {
  owner : THIS_MODULE,
  read : device_read,
  write : device_write,
  open : device_open,
  release : device_release
};

//--------------------------------------------------------------------
int init_module() {
  Major = register_chrdev(0, DEVICE_NAME, &Fops);

  if (Major < 0) {
    printk("\ndevice failed with %d\n", Major);
    return Major;
  }

  init_waitqueue_head(&q);
  printk ("\nmknod <name> c %d <minor>\n", Major);

  return 0;
}

//--------------------------------------------------------------------
void cleanup_module() {
  int ret;
  outb(0x00, PORT3);
  ret = unregister_chrdev(Major, DEVICE_NAME);

  if (ret < 0)
    printk("Error in unregister_chrdev: %d\n", ret);
}
