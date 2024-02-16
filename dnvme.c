#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/nvme_ioctl.h>
#include <linux/nvme.h>
#include <linux/fcntl.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/blk_types.h>
#include "../linux/drivers/nvme/host/nvme.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("qs");
MODULE_DESCRIPTION("nvme direct test");
MODULE_VERSION("0.01");
MODULE_IMPORT_NS(NVME_TARGET_PASSTHRU);
static char *dev_path="/dev/nvme0n1";
static int dev_fd;
static struct file *file;
struct block_device *bdev;
static char *buffer;
static ssize_t print_memory(const void *ptr, size_t len)  
{  
    const unsigned char *buf = ptr;  
    size_t i;  
  
    for (i = 0; i < len && i < 32; i++) {  
        printk("%02x ", buf[i]);  
    }  
  
    if (len > 32)  
        printk("...");  
  
    printk("\n");  
  
    return 0;  
}  

static int nvme_p2p_submit_io(struct nvme_ns *ns,unsigned length)
{
    struct nvme_command c;
    struct request *req;
    struct bio *bio = NULL;
    
    int ret;
    memset(&c, 0, sizeof(c));
    c.rw.opcode = 2; // 1 write, 2 read;
    c.rw.flags = 0;
    c.rw.nsid = cpu_to_le32(ns->head->ns_id);
    c.rw.slba = cpu_to_le64(1000);
    c.rw.length = cpu_to_le16(length-1);
    c.rw.control = cpu_to_le16(NVME_RW_FUA);

    req = nvme_alloc_request(ns->queue, &c, 0);
	if (IS_ERR(req))
		return PTR_ERR(req);
    nvme_req(req)->flags |= 2;
    buffer = kzalloc(length*sizeof(char), GFP_KERNEL);
	if (!buffer)
		return -1;
    ret = blk_rq_map_kern(ns->queue, req, buffer, length, GFP_KERNEL);
    printk("map ret is %d\n",ret);
    if (ret)
		return -1;
    bio = req->bio;
    if(ret)
        printk("map, error!\n");
    ret = nvme_execute_passthru_rq(req);
    print_memory(buffer,32);
    blk_rq_unmap_user(bio);
    blk_mq_free_request(req);
    
    return ret;
}

static int __init dnvme_init(void) 
{
    unsigned len;
    int ret;
    struct nvme_ns *ns;
    file = filp_open(dev_path,O_RDWR,0777);
	if (IS_ERR(file)) {
		pr_err("dnvme_init error, dev name is %s\n", dev_path);
		return -1;
	}
    bdev = I_BDEV(file->f_mapping->host);
    ns = bdev->bd_disk->private_data;
    len = (1) << ns->lba_shift;
    printk("buffer len is %d\n",len);
    ret = nvme_p2p_submit_io(ns,len);
        if(ret<-1)
        {
            filp_close(file, NULL);
            kfree(buffer);
        }
    // if()
    // // vfs_ioctl
    printk("Hello, World!\n");
    return 0;
}
static void __exit dnvme_exit(void) 
{
    int ret;
    
    ret = filp_close(file, NULL);
    kfree(buffer);
    if(ret<0)
        printk("Close errir!\n");
    printk("Goodbye, World!\n");
}

module_init(dnvme_init);
module_exit(dnvme_exit);
