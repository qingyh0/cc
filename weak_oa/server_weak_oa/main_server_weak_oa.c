#include "server_ops.h"
#include "sql_ops.h"

sqlite3 *db=NULL;

int main(int argc, char const *argv[])
{
    int ret=0, sfd;

    if(argc !=3 )
    {
        printf("please input ip&port");
        return -1;
    }

    //初始化数据库
    ret =sql_init();
    if(ret!=0)
        return -1;

    //服务器初始化
    ret = server_init(argv[1], atoi(argv[2]));
    if(ret <=0)
        return ret;

    sfd=ret;

    //监控是否有客户端连接
    ret = server_listen(sfd);



    close(sfd);
    return 0;
}