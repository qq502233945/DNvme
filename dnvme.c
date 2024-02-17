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
static ssize_t setup_memory(const void *ptr, size_t len)  
{  
    const unsigned char *buf = ptr;  
    size_t i;  
    char *str = "Hello World";
    strcpy(buf,str);
    printk("\n");  
  
    return 0;  
}   

static int nvme_p2p_submit_io_write(struct nvme_command *c, struct nvme_ns *ns,unsigned length,__u64 offset)
{
    struct request *req;
    struct bio *bio = NULL;
    union nvme_result result;
    int ret;
    printk("here is ok 1\n");
    if(buffer!=NULL)
        setup_memory(buffer,32);
    else
        return -1;
    printk("here is ok 2\n");
    c->rw.opcode = 1; // 1 write, 2 read;
    c->rw.flags = 0;
    c->rw.nsid = cpu_to_le32(ns->head->ns_id);
    printk("ns->head->ns_id is %u\n",ns->head->ns_id);
    c->rw.slba = cpu_to_le64(offset);
    c->rw.length = cpu_to_le16(8);
    c->rw.control = 0;
	c->rw.dsmgmt = 0;
	c->rw.reftag = 0;
	c->rw.apptag = 0;
	c->rw.appmask = 0;
    req = nvme_alloc_request(ns->queue, c, 0);
	if (IS_ERR(req))
		return PTR_ERR(req);
    nvme_req(req)->flags |= 2;
    ret = blk_rq_map_kern(ns->queue, req, buffer, length, GFP_KERNEL);
    printk("map ret is %d\n",ret);
    if (ret)
    {
        return -1;
        printk("map, error!\n");
    }
		
    bio = req->bio;     
    ret = nvme_execute_passthru_rq(req);
    printk("nvme_execute_passthru_rq ret is %d\n",ret);
    result = nvme_req(req)->result;
    printk("result, is %llu\n",result.u64);
    print_memory(buffer,15);
    blk_mq_free_request(req);
    
    return ret;
}

static int nvme_p2p_submit_io_read( struct nvme_command *c, struct nvme_ns *ns,unsigned length,__u64 offset)
{
    struct request *req;
    struct bio *bio = NULL;
    union nvme_result result;
    int ret;
    
    c->rw.opcode = 2; // 1 write, 2 read;
    c->rw.flags = 0;
    c->rw.nsid = cpu_to_le32(ns->head->ns_id);
    c->rw.slba = cpu_to_le64(offset);
    c->rw.length = cpu_to_le16(8);
    c->rw.control = 0;
	c->rw.dsmgmt = 0;
	c->rw.reftag = 0;
	c->rw.apptag = 0;
	c->rw.appmask = 0;
    req = nvme_alloc_request(ns->queue, c, 0);
	if (IS_ERR(req))
		return PTR_ERR(req);
    nvme_req(req)->flags |= 2;

    ret = blk_rq_map_kern(ns->queue, req, buffer, length, GFP_KERNEL);
    printk("map ret is %d\n",ret);
    if (ret)
    {
        return -1;
        printk("map, error!\n");
    }
		
    bio = req->bio;     
    ret = nvme_execute_passthru_rq(req);
    printk("nvme_execute_passthru_rq ret is %d\n",ret);
    result = nvme_req(req)->result;
    printk("result, is %llu\n",result.u64);
    print_memory(buffer,32);
    blk_mq_free_request(req);
    
    return ret;
}

static int __init dnvme_init(void) 
{
    unsigned len;
    int ret;
    struct nvme_ns *ns;
    struct nvme_command *c;
    struct request *req;

    file = filp_open(dev_path,O_RDWR,0777);
	if (IS_ERR(file)) {
		pr_err("dnvme_init error, dev name is %s\n", dev_path);
		return -1;
	}
    bdev = I_BDEV(file->f_mapping->host);
    ns = bdev->bd_disk->private_data;
    len = (8) << ns->lba_shift;


    printk("buffer len is %d\n",len);
    buffer = kzalloc(len*sizeof(char), GFP_KERNEL);
	if (!buffer)
		return -1;
    c = kmalloc(sizeof(struct nvme_command), GFP_KERNEL);
    memset(c, 0, sizeof(struct nvme_command));
    
    
    ret = nvme_p2p_submit_io_write(c,ns,len,100000);
    if(ret<-1)
    {
        filp_close(file, NULL);
        kfree(c);
        kfree(buffer);
        return -1;
    }
    memset(c, 0, sizeof(struct nvme_command));
    memset(buffer, 0, len);
    ret = nvme_p2p_submit_io_read(c,ns,len,100000);
    if(ret<-1)
    {
        filp_close(file, NULL);
        kfree(c);
        kfree(buffer);
        return -1;
    }
    // if()
    // // vfs_ioctl
    kfree(c);
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
