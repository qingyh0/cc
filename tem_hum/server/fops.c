#include "fops.h"
#include "msg.h"
#include "get_tem_hum.h"
#include "m74hc595.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>

extern int dt_data[4];
extern int flag_show;

//切换数码管显示
void *toggle_show(void *arg)
{
    int fd = 0;
    int ret;
    char buf;

    while (1)
    {
        fd = open("/dev/test_gpio", O_RDWR);
        if (fd < 0)
        {
            ERR("open /dev/test_gpio error");
            return NULL;
        }

        ret = read(fd, &buf, 1);
        if (ret < 0)
        {
            ERR("read /dev/test_gpio error");
            return NULL;
        }

        //上面解除阻塞就说明key3被按下
        if (flag_show == 1)
        {
            flag_show = 2;
            get_read(flag_show);
        }
        else if (flag_show == 2)
        {
            flag_show = 1;
            get_read(flag_show);
        }
    }

    close(fd);
}

//进程函数，用于数码管显示
void *dt_show(void *arg)
{
    char sbuf[2] = {0};

    //分离自己
    pthread_detach(pthread_self());

    int ret = 0;
    //打开驱动节点
    int fd = open("/dev/mh74hc595", O_RDWR);
    if (fd < 0)
    {
        ERR("open error");
        return NULL;
    }

    //循环显示
    while (1)
    {
        // printf("%d-%d-%d-%d\n", data[0],data[1],data[2],data[3]);
        // p操作
        // ret = sem_wait(&sem);
        // if (ret != 0)
        // {
        //     ERR("sem wait error");
        //     return NULL;
        // }
        //使用ioctl 控制数码管
        for (int j = 0; j < 4; j++)
        {
            sbuf[0] = which[j];
            sbuf[1] = code[dt_data[j]];
            ioctl(fd, CMD_SEND, sbuf);
            usleep(1000);
        }

        // v操作
        // ret = sem_post(&sem);
        // if (ret != 0)
        // {
        //     ERR("sem post error");
        //     return NULL;
        // }
    }

    close(fd);
    return NULL;
}

//解析出数码管的数组
// flag=1温度 2湿度
int dt_arr(double value, int flag)
{
    int n, i;
    char tempdata[10] = {0};

    //处理数据 -1.23
    double temp_value = value;
    if (temp_value < 0)
        temp_value = 0 - temp_value;

    // printf("<>%lf<>\n", temp_value);

    // 2位整数
    i = 4;
    n = (int)temp_value % 10000; //获取整数部分
    while (n)
    {
        //从低位到高位
        if (i == 4)
        {
            tempdata[i--] = n % 10 + 18; //个位数
        }
        else
        {
            tempdata[i--] = n % 10;
        }
        n /= 10;
    }

    // 2位小数
    temp_value = temp_value - (int)temp_value % 10000; //小数部分
    for (i = 5; i <= 6; i++)
    {
        tempdata[i] = (char)(temp_value * 10);
        n = n * 10 - tempdata[i];
    }

    //得到最高位下标
    for (int j = 0; j < 10; j++)
    {
        if (tempdata[j] > 0)
        {
            i = j;
            break;
        }
    }

    if (value < 0)
    {
        i--;
        tempdata[i] = 17; //-号
    }

    // sem_wait(&sem); 先去掉，显示不流畅

    //截取4位到dt_data中
    dt_data[0] = tempdata[i++];
    dt_data[1] = tempdata[i++];
    dt_data[2] = tempdata[i++];
    if (flag == 1)
        dt_data[3] = 12; //温度c
    if (flag == 2)
        dt_data[3] = 16; //湿度h

    // sem_post(&sem);

    return 0;
}

int socket_init(char *ip, char *port)
{
    int fd = 0;
    int ret = 0;

    // 1. 创建socket
    ret = socket(AF_INET, SOCK_STREAM, 0);
    if (ret == -1)
    {
        ERR("socket create");
        perror("");
        return -1;
    }

    fd = ret;

    // 2. 允许端口重用
    int refuse = 0;
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &refuse, sizeof(refuse));
    if (ret == -1)
    {
        ERR("setsockopt");
        perror("");
        return -1;
    }

    // 3. 绑定
    struct sockaddr_in server_addr =
        {
            .sin_family = AF_INET,
            .sin_port = htons(atoi(port)),
            .sin_addr.s_addr = inet_addr(ip),
        };
    ret = bind(fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if (ret != 0)
    {
        ERR("bind");
        perror("");
        return -1;
    }

    // 4. 转化为监听状态
    ret = listen(fd, 1);
    if (ret != 0)
    {
        ERR("listen");
        perror("");
        return -1;
    }

    return fd;
}

int monitor(int fd)
{
    int cfd = 0;
    int ret = 0;
    struct msg_t msg;
    int flag = 0; //客服端是否离线

    struct sockaddr_in client_addr;
    socklen_t addrlen;

    while (1)
    {
        // 5. accept等待客户端连接

        ret = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (ret < 0)
        {
            ERR("accept");
            perror("");
            return -1;
        }
        cfd = ret; //客户端连接后的fd
        printf("%s:%d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
        printf("开始读温度\n");
        try_read(cfd, 3, &client_addr, msg);
        printf("读完了温度\n");

        while (1)
        {
            // 6. 接收消息
            ret = recv(cfd, &msg, sizeof(msg), 0);
            if (ret < 0)
            {
                ERR("recv error");
                perror("");
                return -1;
            }

            LOGIN(&client_addr, &msg); // 记录日志-客服端发送的消息

            // 7. 根据消息，处理不同的工作
            switch (msg.cmd)
            {
            case SET_ALARM:
                try_toggle(cfd, &client_addr, msg);
                break;
            case CLEAR_ALARM:
                try_toggle(cfd, &client_addr, msg);
                break;
            case SET_TEM_MAX:
                set_tem_max(cfd, &client_addr, msg);
                break;
            case SET_TEM_MIN:
                set_tem_min(cfd, &client_addr, msg);
                break;
            case SET_HUM_MAX:
                set_hum_max(cfd, &client_addr, msg);
                break;
            case SET_HUM_MIN:
                set_hum_min(cfd, &client_addr, msg);
                break;
            case SET_AUTO:
                try_auto(cfd, &client_addr, msg);
                break;
            case CLOSE_AUTO:
                try_close_auto(cfd, &client_addr, msg);
                break;
            case GET_VAIUE:
                try_read(cfd, 3, &client_addr, msg);
                break;
            case DISCONNECT:
                //客户端断开连接
                ERR("disconnect");
                flag = 1;
                break;
            default:
                flag = 1;
                ERR("无效命令");
                break;
            }

            if (flag == 1)
            {
                break;
            }
        }
        printf("关闭了fd");
        close(cfd);
    }
    close(fd);
    return 0;
}

//获取温湿度
// flag=1获取湿度    flag=2获取湿度  其他都获取
int try_read(int cfd, int flag, struct sockaddr_in *addr, struct msg_t msg)
{
    int ret = 0;
    float tem = 0, hum = 0;

    printf("调用驱动\n");
    //获取温湿度
    ret = get_tem_hum(&tem, &hum);
    if (ret < 0)
        return ret;
    printf("掉完了驱动\n");
    //封装数据
    memset(&msg, 0, sizeof(struct msg_t));
    t_msg.err = 0;
    t_msg.cmd = 0;
    if (flag == 1)
    {
        memset(t_msg.tem, 0, sizeof(t_msg.tem));
        sprintf(t_msg.tem, "%.2lf", tem); //转成字符串形式
        if (dt_arr(tem, 1) != 0)
            return -1;
    }
    else if (flag == 2)
    {
        memset(t_msg.hum, 0, sizeof(t_msg.hum));
        sprintf(t_msg.hum, "%.2lf", hum); //转成字符串形式
        if (dt_arr(hum, 2) != 0)
            return -1;
    }
    else
    {
        memset(t_msg.tem, 0, sizeof(t_msg.tem));
        memset(t_msg.hum, 0, sizeof(t_msg.hum));
        sprintf(t_msg.tem, "%.2lf", tem); //转成字符串形式
        sprintf(t_msg.hum, "%.2lf", hum); //转成字符串形式
        if (dt_arr(tem, 1) != 0)
            return -1;
    }

    memcpy(t_msg.data, "get tem_hum okay", 16);
    printf("封装完了数据\n");
    //向上位机传输
    ret = send(cfd, &t_msg, sizeof(struct msg_t), 0);
    if (ret < 0)
    {
        ERR("send error");
        perror("");
        return ret;
    }
    printf("发送成功了\n");
    LOGOUT(addr, &t_msg);
    printf("cmd=%d,err=%d,tem=%s,hum=%s,data=%s\n", t_msg.cmd, t_msg.err, t_msg.tem, t_msg.hum, t_msg.data);
    return 0;
}

// 设置温度上限
int set_tem_max(int cfd, struct sockaddr_in *addr, struct msg_t msg)
{

    //设置温度上限
    tem_max = atof(msg.tem);

    memset(&msg, 0, sizeof(msg));
    msg.err = 0;
    memcpy(msg.data, "#set tem_max okay", 16);

    //向上位机传输
    int ret = send(cfd, &msg, sizeof(msg), 0);
    if (ret < 0)
    {
        ERR("send error");
        perror("");
        return ret;
    }

    LOGOUT(addr, &msg);
    printf("tem_max=%lf\n", tem_max);
    return 0;
}

// 设置温度下限
int set_tem_min(int cfd, struct sockaddr_in *addr, struct msg_t msg)
{

    //设置温度下限
    tem_min = atof(msg.tem);

    memset(&msg, 0, sizeof(msg));
    msg.err = 0;
    memcpy(msg.data, "#set tem_min okay", 16);

    //向上位机传输
    int ret = send(cfd, &msg, sizeof(msg), 0);
    if (ret < 0)
    {
        ERR("send error");
        perror("");
        return ret;
    }

    LOGOUT(addr, &msg);
    printf("tem_min=%lf\n", tem_min);
    return 0;
}

// 设置湿度上限
int set_hum_max(int cfd, struct sockaddr_in *addr, struct msg_t msg)
{

    //设置湿度上限
    hum_max = atof(msg.tem);

    memset(&msg, 0, sizeof(msg));
    msg.err = 0;
    memcpy(msg.data, "#set hum_max okay", 16);

    //向上位机传输
    int ret = send(cfd, &msg, sizeof(msg), 0);
    if (ret < 0)
    {
        ERR("send error");
        perror("");
        return ret;
    }

    LOGOUT(addr, &msg);
    printf("hum_max=%lf\n", hum_max);
    return 0;
}

// 设置湿度下限
int set_hum_min(int cfd, struct sockaddr_in *addr, struct msg_t msg)
{

    //设置湿度上限
    hum_min = atof(msg.tem);

    memset(&msg, 0, sizeof(msg));
    msg.err = 0;
    memcpy(msg.data, "#set hum_min okay", 16);

    //向上位机传输
    int ret = send(cfd, &msg, sizeof(msg), 0);
    if (ret < 0)
    {
        ERR("send error");
        perror("");
        return ret;
    }

    LOGOUT(addr, &msg);
    printf("hum_min=%lf\n", hum_min);
    return 0;
}

//翻转电平
int try_toggle(int cfd, struct sockaddr_in *addr, struct msg_t msg)
{
    int fd = open("/dev/test_gpio", O_RDWR);
    if (fd < 0)
    {
        ERR("open /dev/test_gpio error");
        return -1;
    }

    // 翻转电平
    switch (msg.cmd)
    {
    case SET_ALARM:
        ioctl(fd, CMD_SET, 1);
        break;
    case CLEAR_ALARM:
        ioctl(fd, CMD_CLEAR, 1);
        break;
    default:
        break;
    }
    close(fd);

    memset(&msg, 0, sizeof(msg));
    msg.err = 0;
    memcpy(msg.data, "#try toggle okay", 15);

    //向上位机传输
    int ret = send(cfd, &msg, sizeof(msg), 0);
    if (ret < 0)
    {
        ERR("send error");
        perror("");
        return ret;
    }

    LOGOUT(addr, &msg);
    return 0;
}

//定时器信号处理函数
void compare()
{
    //读取一次温度
    try_read(t_cfd, 3, t_addr, t_msg);
    int fd = 0;
    fd = open("/dev/test_gpio", O_RDWR);
    if (fd < 0)
    {
        ERR("open /dev/test_gpio error");
        return;
    }

    if ((atof(t_msg.tem) - tem_max) >= 0 || (atof(t_msg.tem) - tem_min) <= 0 || (atof(t_msg.hum) - hum_max) >= 0 || (atof(t_msg.hum) - hum_min) <= 0)
    {

        // 翻转电平
        ioctl(fd, CMD_SET, 1);
    }
    else
    {
        ioctl(fd, CMD_CLEAR, 1);
    }

    close(fd);
}

void handler(int sig)
{
    if (sig == SIGALRM)
    {
        //时间到了
        compare();
        alarm(t_time);                //重新启动定时器
        printf("timer=%d\n", t_time); //显示下定时器时间
    }
    printf("%d %s\n", sig, strsignal(sig));

    return;
}

// 开启一个定时器，每n秒，获取一次温湿度，对比阈值，
int try_auto(int cfd, struct sockaddr_in *addr, struct msg_t msg)
{
    int ret = 0;
    t_time = msg.data[0];
    t_cfd = cfd;
    t_addr = addr;
    t_msg = msg;

    printf("timer=%d\n", t_time); //显示下定时器时间

    alarm(t_time); //单位 s

    memset(&msg, 0, sizeof(msg));
    msg.err = 0;
    memcpy(msg.data, "#try auto okay", 15);

    //向上位机传输
    ret = send(cfd, &msg, sizeof(msg), 0);
    if (ret < 0)
    {
        ERR("send error");
        perror("");
        return ret;
    }

    LOGOUT(addr, &msg);
    return 0;
}

//关闭这个自动流程
int try_close_auto(int cfd, struct sockaddr_in *addr, struct msg_t msg)
{
    int ret = 0;
    t_time = 0;
    t_cfd = cfd;
    t_addr = addr;
    t_msg = msg;

    alarm(t_time); //单位 s

    memset(&msg, 0, sizeof(msg));
    msg.err = 0;
    memcpy(msg.data, "#try close auto okay", 15);

    //向上位机传输
    ret = send(cfd, &msg, sizeof(msg), 0);
    if (ret < 0)
    {
        ERR("send error");
        perror("");
        return ret;
    }

    LOGOUT(addr, &msg);
    return 0;
}

//开机get温湿度
int get_read(int flag)
{
    int ret = 0;
    float tem = 0, hum = 0;

    //获取温湿度
    ret = get_tem_hum(&tem, &hum);
    if (ret < 0)
        return ret;
    // printf("----------tem=%f,hum=%f\n", tem, hum);

    if (flag == 1)
    {
        //显示温度
        dt_arr(tem, 1);
    }
    else if (flag == 2)
    {
        //显示湿度
        dt_arr(hum, 2);
    }
    printf(">>>tem=%f,hum=%f\n", tem, hum);
    return 0;
}