#ifndef _MSG_H_
#define _MSG_H_

//内核层打印
#define SHOW(str) ({printk("[%s:%s:%d]%s\n",__FILE__, __func__, __LINE__, str);})


// -------和底层的ioctl
// 获取温度
#define CMD_GET_TEM _IOR('a', 1, int)
// 获取湿度
#define CMD_GET_HUM  _IOR('a', 2, int)

// 设置开
#define CMD_SET  _IOW('a', 3, int)
// 设置关
#define CMD_CLEAR  _IOW('a', 4, int)

// 哪一位
#define CMD_WHICH  _IOW('a', 5, int)
// 那个字
#define CMD_DAT  _IOW('a', 6, int)
// 位+字
#define CMD_SEND  _IOW('a', 7, char [2])

#define GET_CMD_SIZE(cmd) ((cmd>>16) & 0x3FFF) 

#endif