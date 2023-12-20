/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include "aesdchar.h"

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Yusuf Abdulsttar"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    /**
     * TODO: handle open
     */
     
    struct aesd_dev *dev;
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev;
    
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle read
     */
    struct aesd_dev *dev = filp->private_data;
    size_t entry_offset;
    struct aesd_buffer_entry *entry;
    
    entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->circular_buffer, *f_pos, &entry_offset);
    if (!entry) {
        retval = 0;       // End of file reached, return 0
        goto clean;
    }
    
    if (entry) {
        size_t available_bytes = entry->size - entry_offset;
	
	if (available_bytes < count){
	    count = available_bytes;
	}
	
        if (copy_to_user(buf, entry->buffptr + entry_offset, count)) {
            retval = -EFAULT;   // Error while copying data to user space
            PDEBUG("Error in copy_to_user function");
            goto clean;
        } 
        else {
            *f_pos += count;    // Update file position
            retval = count;    // Set return value to the actual number of bytes read
        }
    }
    
clean:
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle write
     */
     
    // Extracting character device instance from the file structure. 
    struct aesd_dev *dev = filp->private_data;
    char *buffptr_temp;
    const char *free_buffer = NULL;
    
    if (mutex_lock_interruptible(&dev->lock)) {
    	PDEBUG("Error in mutex locking");
        retval = -ERESTARTSYS;
        goto clean;
    }
     
    if (!count) {
        retval = 0;
        goto clean;    // there is no bytes to write
    }
    
    // Allocate or reallocate depending if add entry contains unterminated commands.
    if (!dev->entry_cash.buffptr) {
        buffptr_temp = kmalloc(count, GFP_KERNEL);
    } else {
        buffptr_temp = krealloc(dev->entry_cash.buffptr, dev->entry_cash.size + count, GFP_KERNEL);
    }
    
    if (!buffptr_temp){
        PDEBUG("faild to allocate memory for the buffer");
        goto clean;
    }
    
    // Copy data from user space to kernel space.
    if (copy_from_user(buffptr_temp + dev->entry_cash.size, buf, count)) 
    {
        retval = -EFAULT;
        kfree(buffptr_temp);
        goto clean;
    }
    else{
        retval = count;
    }
    
    dev->entry_cash.buffptr = buffptr_temp;
    dev->entry_cash.size = dev->entry_cash.size + count;

    if (dev->entry_cash.buffptr[dev->entry_cash.size-1] == '\n') {

        // Free next entry if buffer is already full

        free_buffer = aesd_circular_buffer_add_entry(&dev->circular_buffer, &dev->entry_cash);
        
        if (free_buffer) {
            kfree(free_buffer);
        }

        dev->entry_cash.buffptr = NULL;
        dev->entry_cash.size = 0;
    }
  
clean:
    mutex_unlock(&dev->lock);
    return retval;
}

struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */
    aesd_circular_buffer_init(&aesd_device.circular_buffer);
    aesd_device.entry_cash.buffptr = NULL;
    aesd_device.entry_cash.size = 0;
    mutex_init(&aesd_device.lock);

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */
     
    uint8_t index;
    AESD_CIRCULAR_BUFFER_FOREACH(entry, &aesd_device.circular_buffer, index) {  
        kfree(entry->buffptr);
    }

    mutex_destroy(&aesd_device.lock);
    
    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
