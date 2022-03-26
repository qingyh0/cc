#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include "msg.h"

#define NAME "test_gpio"
struct resource *res = NULL;
int irqno[3] = {0};  // 3个按键中断
int gpiono[8] = {0}; // 3个led灯 风扇 蜂鸣器 key1-key3

int major = 0;
struct class *cls = NULL;
struct device *dev = NULL;

char kbuf[10] = {0}; //是否显示温度
//阻塞IO实现
wait_queue_head_t wq;
int condition = 0;
//定时器
struct timer_list mytimer;


//定时器处理函数
void irq_timer_function(struct timer_list *timer)
{
    int i=0;

    if (!gpio_get_value(gpiono[5]))
    {
        // key1按下 清除警报，
        for (i = 0; i < 5; i++)
        {
            gpio_set_value(gpiono[i], 0);
        }
        SHOW("解除警报");
    }

    if (!gpio_get_value(gpiono[6]))
    {
        // key2按下 测试gpio口，
        for (i = 0; i < 5; i++)
        {
            gpio_set_value(gpiono[i], 1);
        }
        SHOW("测试gpio");
    }
    if (!gpio_get_value(gpiono[7]))
    {
        // key3按下，切换温湿度显示
        //条件设置为真，并唤醒等待
        condition = 1;
        wake_up_interruptible(&wq);
        SHOW("切换温湿度显示");
        // mod_timer(&mytimer,jiffies+1);
    }

    return;
}

//中断处理函数
irqreturn_t mykey_irq(int irq, void *args)
{
    if (irq == irqno[0])
    {
        // key1按下 
        mod_timer(&mytimer, jiffies + 1);
    }
    if (irq == irqno[1])
    {
        // key2按下
        mod_timer(&mytimer, jiffies + 1);
    }
    if (irq == irqno[2])
    {
        // key3按下，启动定时器
        mod_timer(&mytimer, jiffies + 1);
    }

    return IRQ_HANDLED;
}

int mygpio_open(struct inode *inode, struct file *file)
{
    SHOW("mygpio_open okay");
    return 0;
}

int mygpio_close(struct inode *inode, struct file *file)
{
    SHOW("mygpio_close okay");
    return 0;
}

ssize_t mygpio_read(struct file *file, char __user *ubuf, size_t ulen, loff_t *off)
{
    int ret = 0;

    if (ulen != sizeof(ubuf))
        ulen = sizeof(ubuf);

    if (file->f_flags & O_NONBLOCK)
    {
        //非阻塞
        SHOW("open error");
        return -1;
    }
    else
    {
        ret = wait_event_interruptible(wq, condition);
        if (ret < 0)
        {
            printk("wait interrupt by signal\n");
            return ret;
        }
    }
    kbuf[0] = 1;
    copy_to_user(ubuf, kbuf, ulen);
    //读取之后，将条件置为假
    condition = 0;

    return ulen;
}

long mygpio_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
    switch (cmd)
    {
        case CMD_SET:
            gpio_set_value(gpiono[0], 1);
            SHOW("led1 on");
            gpio_set_value(gpiono[1], 1);
            SHOW("led2 on");
            gpio_set_value(gpiono[2], 1);
            SHOW("led3 on");
            gpio_set_value(gpiono[3], 1);
            SHOW("fan on");
            gpio_set_value(gpiono[4], 1);
            SHOW("beep on");
            break;
        case CMD_CLEAR:
            gpio_set_value(gpiono[0], 0);
            SHOW("led1 off");
            gpio_set_value(gpiono[1], 0);
            SHOW("led2 off");
            gpio_set_value(gpiono[2], 0);
            SHOW("led3 off");
            gpio_set_value(gpiono[3], 0);
            SHOW("fan off");
            gpio_set_value(gpiono[4], 0);
            SHOW("beep off");
            break;
    default:
        SHOW("unknow cmd");
        break;
    }
    return 0;
}

const struct file_operations fops = {
    .open = mygpio_open,
    .release = mygpio_close,
    .read = mygpio_read,
    .unlocked_ioctl = mygpio_ioctl,
};


// gpio初始化
int my_init(void)
{
    int ret = 0;
    int i = 0;

    //初始化一个定时器
    mytimer.expires = jiffies + 1;
    timer_setup(&mytimer, irq_timer_function, 0);
    add_timer(&mytimer);

    //注册gpio
    for (i = 0; i < 8; i++)
    {
        ret = gpio_request(gpiono[i], NULL);
        if (ret)
        {
            printk("gpio request error\n");
            return ret;
        }

        //初始化gpio
        if(i<5)
            gpio_direction_output(gpiono[i], 0);
    }

    //注册irq
    for (i = 0; i < 3; i++)
    {
        ret = request_irq(irqno[i], mykey_irq, IRQF_TRIGGER_FALLING, NAME, NULL);
        if (ret)
        {
            SHOW("request irq error");
            return ret;
        }
    }

    //注册字符设备驱动
    major = register_chrdev(0, NAME, &fops);
    if (major < 0)
    {
        SHOW("register char device driver error");
        return major;
    }

    // 4.自动创建设备节点
    cls = class_create(THIS_MODULE, NAME);
    if (IS_ERR(cls))
    {
        printk("class create error\n");
        return PTR_ERR(cls);
    }

    dev = device_create(cls, NULL, MKDEV(major, 0), NULL, NAME);
    if (IS_ERR(dev))
    {
        SHOW("device create error");
        return PTR_ERR(dev);
    }

    //初始化等待队列头
    init_waitqueue_head(&wq);

    return 0;
}

//驱动安装时做的事
int mygpio_probe(struct platform_device *pdev)
{
    int ret = 0;
    int i = 0;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (NULL == res)
    {
        SHOW("platform get resource error");
        return -EINVAL;
    }

    for (i = 0; i < 3; i++)
    {
        irqno[i] = platform_get_irq(pdev, i);
        if (irqno[i] < 0)
        {
            SHOW("platform get irq error");
            return irqno[i];
        }
    }
    for (i = 0; i < 3; i++)
    {
        gpiono[i] = of_get_named_gpio(pdev->dev.of_node, "led", i);
        if (gpiono[i] < 0)
        {
            SHOW("of get named gpio led error");
            return gpiono[i];
        }
    }
    gpiono[3] = of_get_named_gpio(pdev->dev.of_node, "fan", 0);
    if (gpiono[i] < 0)
    {
        SHOW("of get named gpio fan error");
        return gpiono[3];
    }
    gpiono[4] = of_get_named_gpio(pdev->dev.of_node, "beep", 0);
    if (gpiono[i] < 0)
    {
        SHOW("of get named gpio beep error");
        return gpiono[4];
    }
    for (i = 5; i < 8; i++)
    {
        gpiono[i] = of_get_named_gpio(pdev->dev.of_node, "key", i-5);
        if (gpiono[i] < 0)
        {
            SHOW("of get named gpio key error");
            return gpiono[i];
        }
    }

    //初始化
    ret = my_init();
    if (ret != 0)
        return ret;

    SHOW("mygpio_probe okay");
    return ret;
}

//驱动卸载事做的事
int mygpio_remove(struct platform_device *pdev)
{
    int i = 0;
    device_destroy(cls, MKDEV(major, 0));
    class_destroy(cls);
    unregister_chrdev(major, NAME);

    del_timer(&mytimer);
    for (i = 0; i < 3; i++)
        free_irq(irqno[i], NULL);
    for (i = 0; i < 5; i++)
        gpio_free(gpiono[i]);
    SHOW("mygpio_remove");
    return 0;
}

struct of_device_id oftable[] = {
    {.compatible = "my,gpio"},
    {},
};
MODULE_DEVICE_TABLE(of, oftable);

static struct platform_driver mygpio = {
    .probe = mygpio_probe,
    .remove = mygpio_remove,
    .driver = {
        .name = "dgpio",
        .of_match_table = oftable,
    },
};

module_platform_driver(mygpio);
MODULE_LICENSE("GPL");
