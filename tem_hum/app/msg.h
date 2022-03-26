#ifndef _MSG_H_
#define _MSG_H_

//应用层打印
#define ERR(str) ({printf("[%s:%s:%d]%s\n", __FILE__, __func__, __LINE__, str);})
#define LOGIN(ip, msg) ({printf("<<[%s:%d]<%d><%s><%s><%s>\n", \
                    inet_ntoa((ip)->sin_addr),\
                    htons((ip)->sin_port),\
                    (msg)->cmd, (msg)->tem, (msg)->hum,\
                    (msg)->data);})
#define LOGOUT(ip, msg) ({printf(">>[%s:%d]<%d><%s><%s><%s>\n", \
                    inet_ntoa((ip)->sin_addr),\
                    htons((ip)->sin_port),\
                    (msg)->cmd, (msg)->tem, (msg)->hum,\
                    (msg)->data);})

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



// -------和上位机的控制码
#define SET_TEM_MAX 0x01
#define SET_TEM_MIN 0x02
#define SET_HUM_MAX 0x03
#define SET_HUM_MIN 0x04
#define SET_AUTO 0x05
#define CLOSE_AUTO 0x06
#define DISCONNECT 0x07
#define SET_ALARM 0x08
#define CLEAR_ALARM 0x09
#define GET_VAIUE 0x10

#endif