#include "msg.h"
#include "fops.h"
#include <stdio.h>
#include <pthread.h>

int flag_show=1;//默认显示温度

int dt_data[4]={0,0,0,0};   //存储数码管数据
typedef void (*sighandler_t)(int);

int main(int argc, char const *argv[])
{
    if(argc!=3){
        printf("请输入服务端ip port");
        return 0;
    }

    int fd;
    int ret = socket_init(argv[1], argv[2]);
    if(ret<0)
        return ret;

    fd=ret;


    // 初始化一个信号量, 1个用于修改全局dt_data，数码管内容
    ret = sem_init(&sem, 0, 1);
    if (ret != 0)
    {
        ERR("sem init error");
        return ret;
    }


    // //获取下温度
    // if(get_read(flag_show)!=0){
    //     ERR("get tem/hum error");
    //     return -1;
    // }
    

    //开启一个线程，用于数码管显示
    pthread_t th;
    ret=pthread_create(&th, NULL, dt_show, NULL);
    if(ret!=0)
    {
        ERR("pthread_create dt_show error");
        return -1;
    }

    //开启一个线程，监测key3是否按下
    pthread_t th_key3;
    ret=pthread_create(&th_key3, NULL, toggle_show, NULL);
    if(ret!=0)
    {
        ERR("pthread_create toggle_show error");
        return -1;
    }

    //捕获14)SIGALRM信号
	sighandler_t s = signal(14, handler);
	if(SIG_ERR == s)
	{
		perror("signal");
		return -1;
	}


    //陷入循环
    ret = monitor(fd);
    
    
    return 0;
}
