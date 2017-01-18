/*
 * Device driver for the VGA LED Emulator
 *
 * References:
 * http://www.linuxforu.com/tag/linux-device-drivers/
 *
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <asm/io.h>

#include "vga_led.h"
#include "socal/hps.h"
#include "hps_0.h"

/* Memory region visible through the lightweight HPS-FPGA bridge */
#define HW_REGS_BASE ALT_LWFPGASLVS_OFST
#define HW_REGS_SIZE 0x200000
#define HW_REGS_MASK (HW_REGS_SIZE - 1)

#define WRITE_BYTE(addr, val)  ( *(volatile unsigned int *)(addr) = (val))

static void __iomem *registers; /* Base of mapped memory */
static dev_t firstdev;
static struct class *cl;
static struct cdev c_dev;

static void *screen_registers; /* Start of registers for LEDs */

/* Current center state, since the hardware registers can't be read */
static sprite_info sp_info;
static screen background_info;



static void write_screen_sprite(sprite_info sprite) {
    unsigned int spr;
    if (sprite.layer ==  0){
      spr = (((unsigned int)sprite.pos.x) & 0x3ff) |
         ((((unsigned int)sprite.pos.y) & 0x3ff) << 10) | 
         ((((unsigned int)sprite.orientation) & 0x1) << 20) |
         ((((unsigned int)sprite.count) & 0x7) << 21) |
         ((((unsigned int)sprite.id) & 0xf) << 24) |
         ((((unsigned int)sprite.shape) & 0x7) << 28) |
         ((((unsigned int)sprite.layer) & 0x1) << 31);
    }
    else{
      spr = (((unsigned int)sprite.pos.x) & 0x3ff) |
         ((((unsigned int)sprite.pos.y) & 0x3ff) << 10) | 
         ((((unsigned int)sprite.orientation) & 0x1) << 20) |
         ((((unsigned int)sprite.count) & 0x7) << 21) |
         ((((unsigned int)sprite.id) & 0x7) << 24) |
         ((((unsigned int)sprite.shape) & 0xf) << 27) |
         ((((unsigned int)sprite.layer) & 0x1) << 31);  
    }
  	

  	WRITE_BYTE(screen_registers + 0, spr);
  	sp_info = sprite;
}

static void write_screen_back(screen background) {
    unsigned int bgnd;
    

    bgnd =  (((unsigned int)background.life_1) & 0x7) |
           ((((unsigned int)background.life_2) & 0x7) << 3) |
           ((((unsigned int)background.choice) & 0x1) << 6) |
           (((background.background_color) & 0xffffff) << 8);

    WRITE_BYTE(screen_registers + 4, bgnd);
    background_info = background;
}



static int my_open(struct inode *i, struct file *f) {
  return 0;
}

static int my_close(struct inode *i, struct file *f) {
  return 0;
}

/* Handle ioctls(): write to the display registers or read our state */
static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
  vga_screen_arg_t vla;

  switch (cmd) {
  case VGA_LED_WRITE_DIGIT:
    if (copy_from_user(&vla, (vga_screen_arg_t *) arg, sizeof(vga_screen_arg_t)))
      return -EACCES;

  	
    if(vla.option & 0x1)
      write_screen_sprite(vla.sprite);
    if(vla.option & 0x2)
      write_screen_back(vla.background);

    break;
    
  case VGA_LED_READ_DIGIT:
    if (copy_from_user(&vla, (vga_screen_arg_t *) arg, sizeof(vga_screen_arg_t)))
      return -EACCES;
    vla.sprite = sp_info;
    vla.background = background_info;
    if (copy_to_user((vga_screen_arg_t *) arg, &vla, sizeof(vga_screen_arg_t)))
      return -EACCES;
    break;

  default:
    return -EINVAL;
  }

  return 0;
}

static struct file_operations my_fops = {
  .owner = THIS_MODULE,
  .open = my_open,
  .release = my_close,
  .unlocked_ioctl = my_ioctl
};

/* Initialize the driver: map the hardware registers, register the
 * device and our operations, and display a welcome message */
static int __init vga_led_init(void) {

  printk(KERN_INFO "vga_led: init\n");

  if ( (registers = ioremap(HW_REGS_BASE, HW_REGS_SIZE)) == NULL ) {
    printk(KERN_ERR "vga_led: Mapping hardware registers failed\n");
    return -1;
  }

  screen_registers = registers +
    ((unsigned long) VGA_LED_0_BASE & (unsigned long) HW_REGS_MASK);

  if (alloc_chrdev_region(&firstdev, 0, 1, "vgaled") < 0) goto unmap;
  if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL) goto unregister;
  if (device_create(cl, NULL, firstdev, NULL, "vgaled") == NULL) goto del_class;
  cdev_init(&c_dev, &my_fops);
  if (cdev_add(&c_dev, firstdev, 1) == -1) goto del_device;

  /* Display a welcome message */
 //write_center(320,240);
  return 0;

  /* Clean up if something went wrong */
 unmap:      iounmap(registers);
 del_device: device_destroy(cl, firstdev);
 del_class:  class_destroy(cl);
 unregister: unregister_chrdev_region(firstdev, 1);
  return -1;
}

/* Disable the driver; undo the effects of the initialization routine */
static void __exit vga_led_exit(void) {
  printk(KERN_INFO "vga_led: exit\n");

  cdev_del(&c_dev);
  device_destroy(cl, firstdev);
  class_destroy(cl);
  unregister_chrdev_region(firstdev, 1);
  iounmap(registers);
}

module_init(vga_led_init);
module_exit(vga_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stephen A. Edwards, Columbia University");
MODULE_DESCRIPTION("VGA 7-segment LED Emulator");
