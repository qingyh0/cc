#include "msg.h"
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/mod_devicetable.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define BUFFLEN 128

#define CNT 1
#define NAME "si7006"

struct cdev *cdev = NULL;
int devno=0;
int major=0;

struct class *cls=NULL;
struct device *dev=NULL;

char kbuf[BUFFLEN]={0};

//
struct i2c_client *gclient=NULL;

int i2c_get_TH(char *data)
{
	int ret=0;
	
	//1.封装消息  ->读取湿度
	//{start} + [slave addr(7bit) + w(0)] + {ack} + [0xE5] + {ack} + 
	//{start} + [slave addr(7bit) + r(1)] + {ack} + [高字节] + {ack} + [低字节] + {nack} + {stop}
	char h_cmd = 0xE5;
	char h_data[2] ={0};
	struct i2c_msg h_msg[]={
	[0]={
			.addr = gclient->addr,
			.flags = 0,//写
			.len = 1,
			.buf = &h_cmd,
		},
	[1]={
		.addr = gclient->addr,
		.flags = 1,//读
		.len = 2,
		.buf = h_data,
	},
	};
	//读取温度
	//{start} + [slave addr(7bit) + w(0)] + {ack} + [0xE0] + {ack} + 
	//{start} + [slave addr(7bit) + r(1)] + {ack} + [高字节] + {ack} + [低字节] + {nack} + {stop}
	char t_cmd = 0xE3;
	char t_data[2] ={0};
	struct i2c_msg t_msg[]={
	[0]={
			.addr = gclient->addr,
			.flags = 0,//写
			.len = 1,
			.buf = &t_cmd,
		},
	[1]={
		.addr = gclient->addr,
		.flags = 1,//读
		.len = 2,
		.buf = t_data,
	},
	};

	
	//2.发送消息
	ret = i2c_transfer(gclient->adapter, h_msg, ARRAY_SIZE(h_msg));
	if(ret <0)
	{
		SHOW("i2c read hum error");
		printk("error=%d\n", ret);
		return ret;
	}

	ret = i2c_transfer(gclient->adapter, t_msg, ARRAY_SIZE(t_msg));
	if(ret <0)
	{
		SHOW("i2c read tem error");
		return ret;
	}

	memcpy(data, h_data, 2);
	memcpy(data+2, t_data, 2);
	
	return 0;
}

int si7006_open(struct inode *inode, struct file *file)
{
	SHOW("si7006_open");
	return 0;
}

int si7006_close(struct inode *inode, struct file *file)
{
	SHOW("si7006_close");
	return 0;
}

ssize_t si7006_read(struct file *file, char __user *ubuf, size_t ulen, loff_t *offs)
{
	int ret=0;
	if(ulen > BUFFLEN)
		ulen = BUFFLEN;

	memset(kbuf, 0, sizeof(kbuf));

	//读取温湿度
	memset(kbuf, 0, sizeof(kbuf));
	ret = i2c_get_TH(kbuf);
	if(ret != 0)
		return ret;

	ulen = strlen(kbuf);
	copy_to_user(ubuf, kbuf, ulen);
	
	SHOW("si7006_read_okay");
	return ulen;
}


const struct file_operations fops={
	.open=si7006_open,
	.release=si7006_close,
	.read=si7006_read,
};

//驱动注册时执行
int si7006_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret=0;
	gclient = client;

	// 1. 申请一个对象
	cdev = cdev_alloc();
	if(cdev == NULL){
		ret = -ENOMEM;
		goto ERR1;
	}

	// 2. 初始化一个对象
	cdev_init(cdev, &fops);

	// 3. 申请设备号
	//int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count,const char *name)
	ret = alloc_chrdev_region(&devno, 0, 1, NAME);
	if(ret != 0)
	{
		SHOW("alloc_chrdev_region");
		ret = -1;	//?
		goto ERR2;
	}

	// 4. 注册
	//int cdev_add(struct cdev *p, dev_t dev, unsigned count)
	ret = cdev_add(cdev, devno, CNT);
	if(ret<0)
	{
		SHOW("cdev_add");
		ret= -EBUSY;
		goto ERR2;
	}

	// 5. 向上层提交一个目录
	cls = class_create(THIS_MODULE, NAME);
	if(IS_ERR(cls)){
		SHOW("class_create_err");
		ret = PTR_ERR(cls);
		goto ERR3;
	}

	// 6. 向上层提交一个节点
	//struct device *device_create(struct class *class, struct device *parent,
	//		     dev_t devt, void *drvdata, const char *fmt, ...)
	dev = device_create(cls, NULL, devno, NULL, NAME);
	if(IS_ERR(dev)){
		SHOW("device_create_err");
		ret = PTR_ERR(dev);
		goto ERR4;
	}


	SHOW("init okay");


	return 0;


// ERR5:
// 	device_destroy(cls, devno);
ERR4:
	class_destroy(cls);
ERR3:
	cdev_del(cdev);
ERR2:
	kfree(cdev);
ERR1:
	return ret;

}

//驱动卸载时执行
int si7006_remove(struct i2c_client *client)
{

	device_destroy(cls, devno);

	class_destroy(cls);

	cdev_del(cdev);

	kfree(cdev);

	return 0;

}



const struct of_device_id oftable[]={
			{.compatible="my,si7006"},
				{},
};
MODULE_DEVICE_TABLE(of, oftable);


struct i2c_driver si7006={
	.probe=si7006_probe,
	.remove=si7006_remove,
	.driver={
			.name="si7006",//不用于匹配，但是必须填
			.of_match_table=oftable,
	},
};


//注册i2c驱动
// i2c_add_driver(si7006);

module_i2c_driver(si7006);
MODULE_LICENSE("GPL");


