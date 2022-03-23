#include "client_init.h"
#include "client_ops.h"
// #include "signal.h"

int fd=-1;
char name[21]={0};
char id[11]={0};
char department[41]={0};
char rank[3]={0};

int main(int argc, char const *argv[])
{
    sighandler_t res;
    int ret, sfd;
    if(argc !=3 )
    {
        printf("please input ip&port");
        return -1;
    }

    ret = sock_init(argv[1], atoi(argv[2]));
    if(ret<=0)
        return ret;

    sfd = ret;
    fd = sfd;

    //捕获信号 SIGINT   ->ctrl+c
    res = signal(SIGINT, handler);
    if(res == SIG_ERR)
    {
        ERR("signal error");
        perror("");
        return -1;
    }

    //登录界面  返回的ret是此账号代表的权限
    ret = login_init(sfd);
    if(ret!=0)
    {
        printf("按任意键退出\n");
        getchar();
        return ret;
    }

    printf("%s %s\n", name, rank);
    
    //主界面
    int flag=atoi(rank);
    if(88 == flag)
        ret= lv88_interface();
    if(66 == flag || 99 == flag)
        ret= lv66_interface();
    if(0 == flag)
        ret= lv00_interface();


    //发送连接断开消息
    finish();
    return 0;
}
