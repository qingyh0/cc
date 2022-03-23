#ifndef _SERVER_OPS_H_
#define _SERVER_OPS_H_

// #include "err.h"
#include "sql_ops.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>


#define BUFFLEN 512

//向线程中传递的参数
struct pth_args{
    int fd;
    struct sockaddr_in addr;
};

//信息传递结构体
struct info_t{
    char cmd;   //命令码
    char errcode;   //错误码
    char isend;     //是否有后续
    char id[11];    //工号
    char name[21];  //姓名
    char tel[15];   //
    char sex;
    char birth[11];
    char addr[201]; //住址
    char entrytime[11]; //入职时间
    char job[41];       //职位
    char department[41];    //部门
    char salary[10];        //薪资
    char rank[3];           //权限
    char usr[21];
    char pwd[21];
}__attribute__((packed));


//----服务器端初始化
//-返回sock fd
int server_init(const char *ip, const unsigned short port);

//--------客户端连接处理
int server_listen(int sfd);

//0x02登录
int login(struct pth_args *args, struct info_t *info);
//0x03 入职
int onboarding(struct pth_args *args, struct info_t *info);
// 0x08 按照姓名查询
int find_by_name(struct pth_args *args, struct info_t *info);
// 0x09 按照工号查询
int find_by_id(struct pth_args *args, struct info_t *info);
// 0x04 注销
int resign(struct pth_args *args, struct info_t *info);
// 0x05 修改信息
int modify(struct pth_args *args, struct info_t *info);
// 0x0D 修改账户信息
int change_account(struct pth_args *args, struct info_t *info);

#endif