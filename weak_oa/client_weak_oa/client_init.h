#ifndef _CLIENT_INIT_H_
#define _CLIENT_INIT_H_

#include <termio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "err.h"
#include <signal.h>

typedef void (*sighandler_t)(int);
//信号处理函数
void handler(int sig);
//客户端关闭
void finish(void);
//主界面初始化
int login_init();
//----------------2. 00普通员工界面
int lv00_interface();
//----------------1. 66人事主界面
int lv66_interface();
//----------------2. 88财务界面
int lv88_interface();

//------------------1.1.3 员工入职
int onboarding(void);
//------------------1.1.1 按姓名查询
int find_by_name(void);
//------------------1.1.2 按工号查询
int find_by_id(void);
//------------------1.1.4 离职
int resign(void);
//------------------1.1.5 员工信息修改
int modify(void);
//-----------------2. 账户信息修改
int change_account(void);

int getch(void);
#endif