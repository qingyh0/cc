#include <linux/spi/spi.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "m74hc595.h"
#include "msg.h"

#define SPICNAME "mh74hc595"
unsigned int major=0;
struct class *cls;
struct device *dev;

struct spi_device *gspi;


int mh74hc595_open(struct inode *inode, struct file *file)
{
    SHOW("mh74hc595_open");
    return 0;
}
int mh74hc595_close(struct inode *inode, struct file *file)
{
    SHOW("mh74hc595_close");
    return 0;
}
long mh74hc595_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
    int ret=0;
    unsigned char rbuf[2]={0};

    copy_from_user(rbuf, (int*)args, GET_CMD_SIZE(cmd));

    switch(cmd)
    {
        case CMD_SEND:
            // printk("-%x-%x-",rbuf[0], rbuf[1]);
			ret = spi_write(gspi, rbuf ,2);
			break;
		default: 
            SHOW("ioctl error");
            break;
    }
    return 0;
}

const struct file_operations fops={
    .open=mh74hc595_open,
    .release=mh74hc595_close,
    .unlocked_ioctl=mh74hc595_ioctl,
};


int	myspi_probe(struct spi_device *spi)
{
    int ret=0;
    gspi = spi;
    //1.注册字符设备驱动
    ret = register_chrdev(0, SPICNAME, &fops);
    if(ret < 0)
    {
        printk("register spi device error\n");
        goto ERR1;
    }
    major = ret;

    //2.自动注册节点
    cls = class_create(THIS_MODULE, SPICNAME);
    if(IS_ERR(cls))
    {
        printk("class create error\n");
        ret = PTR_ERR(cls);
        goto ERR2;
    }

    dev = device_create(cls, NULL, MKDEV(major, 0), NULL, SPICNAME);
    if(IS_ERR(dev))
    {
        printk("device create error\n");
        ret = PTR_ERR(dev);
        goto ERR3;
    }

    printk("%s:%d>>probe okay\n", __func__, __LINE__);
    return 0;



ERR3:
    class_destroy(cls);
ERR2:
    unregister_chrdev(major, SPICNAME);
ERR1:
    return ret;
}
int	myspi_remove(struct spi_device *spi)
{
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    unregister_chrdev(major, SPICNAME);
    SHOW("myspi_remove okay");
    return 0;
}


struct of_device_id of_match[]={
    {.compatible = "my,mh74hc595",},
    {},
};
MODULE_DEVICE_TABLE(of, of_match);   /* 支持热插拔*/

const struct spi_device_id id_match[] = {
	{"m74hc595",0},
	{},
};
MODULE_DEVICE_TABLE(spi,id_match);

struct spi_driver myspi={
    .probe = myspi_probe,
    .remove = myspi_remove,
    .driver = {
        .name = "m74hc595",     /*设备驱动名*/
        .of_match_table = of_match,  /*设备树匹配*/
    },
    .id_table = id_match,   /*这个驱动支持的设备表*/
};

module_spi_driver(myspi);
MODULE_LICENSE("GPL");