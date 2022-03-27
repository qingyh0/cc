#ifndef _FOPS_H_
#define _FOPS_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

#define BUFFLEN 128


// char data[4]={0};   //存储数码管数据
sem_t sem;          //信号量，用于data的修改
double tem_min;     //温度下限
double tem_max;     //温度上限
double hum_min;     //湿度下限
double hum_max;     //湿度下限
unsigned char t_time;           //轮询时间
//---------------------------
int t_cfd;
struct sockaddr_in *t_addr;
struct msg_t t_msg;
//---------------------------

//发送信息结构体
struct msg_t{
    char err;
    char cmd;
    char tem[10];//温度
    char hum[10]; //湿度
    char data[BUFFLEN];
}__attribute__((packed));

//解析出数码管的数组
int dt_arr(double value, int flag);
//进程函数，用于数码管显示
void *dt_show(void *arg);

int socket_init(char *ip, char *port);
int monitor(int fd);

//读取温湿度
int try_read(int cfd, int flag, struct sockaddr_in *addr, struct msg_t msg);

int set_tem_max(int cfd, struct sockaddr_in *addr, struct msg_t msg);
int set_tem_min(int cfd, struct sockaddr_in *addr, struct msg_t msg);
int set_hum_max(int cfd, struct sockaddr_in *addr, struct msg_t msg);
int set_hum_min(int cfd, struct sockaddr_in *addr, struct msg_t msg);
//翻转电平
int try_toggle(int cfd, struct sockaddr_in *addr, struct msg_t msg);
// 开启一个定时器，每n秒，获取一次温湿度，对比阈值，
int try_auto(int cfd, struct sockaddr_in *addr, struct msg_t msg);
//关闭这个自动流程
int try_close_auto(int cfd, struct sockaddr_in *addr, struct msg_t msg);

void handler(int sig);

//开机获取温度
int get_read(int flag);

//切换数码管显示
void *toggle_show(void *arg);


#endif