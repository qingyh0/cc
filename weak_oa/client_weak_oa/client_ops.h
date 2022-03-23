#ifndef _CLIENT_OPS_H_
#define _CLIENT_OPS_H_

#include "err.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

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

int sock_init(const char *ip, const unsigned short port);

//0x02登录
int login(struct info_t *info);
// 0x03 入职
int c_onboarding(struct info_t *info);
// 0x08 按姓名查找
int c_find_by_name(struct info_t *info, struct info_t **info_back, int *cnt);
void free_find_by_name(struct info_t **info);
// 0x09 按照工号查询
int c_find_by_id(struct info_t *info);
// 0x04 离职
int c_resign(struct info_t *info);
// 0x05 信息修改
int c_modify(struct info_t *info, struct info_t *info_new);
//0x0D 修改账户名 密码
int c_change_account(struct info_t *info);


//输出str 还是*
char *istar(char *str, char *s);
#endif