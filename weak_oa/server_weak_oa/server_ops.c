#include "server_ops.h"

extern sqlite3 *db;

void *server_func(void *args);

//----服务器端初始化
//-返回sock fd
int server_init(const char *ip, const unsigned short port)
{
    int ret = 0, sfd;

    // 1.创建socket
    ret = socket(AF_INET, SOCK_STREAM, 0);
    if (ret == -1)
    {
        ERR("");
        perror("");
        goto ERR1;
    }

    sfd = ret; //文件描述符

    // 2.允许端口快速重用
    int flag = 1;
    ret = setsockopt(sfd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag));
    if (ret == -1)
    {
        ERR("");
        perror("");
        goto ERR1;
    }

    // 2.绑定ip port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    ret = bind(sfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret == -1)
    {
        ERR("");
        perror("");
        goto ERR2;
    }

    // 3.转化为监听模式
    ret = listen(sfd, 10);
    if (ret == -1)
    {
        ERR("");
        perror("");
        goto ERR2;
    }

    return sfd;

ERR2:
    close(sfd);
ERR1:
    return ret;
}

//--------客户端连接处理
int server_listen(int sfd)
{
    int ret = 0;
    socklen_t client_len = sizeof(struct sockaddr_in);
    pthread_t th;
    struct pth_args args; //向进程传递的数据

    while (1)
    {
        ret = accept(sfd, (struct sockaddr *)(&args.addr), &client_len);
        if (ret < 0)
        {
            PERR(args.addr, "");
            perror("");
            continue;
        }
        args.fd = ret;

        PERR(args.addr, "");
        printf("fd=%d\n", args.fd);

        //为新客户端创建一个线程
        ret = pthread_create(&th, NULL, server_func, (void *)&args);
        if (ret != 0)
        {
            PERR(args.addr, "");
            perror("");
        }
    }

    close(sfd);
    return 0;
}

void *server_func(void *args)
{
    int fd = ((struct pth_args *)args)->fd;
    char *ip = inet_ntoa(((struct pth_args *)args)->addr.sin_addr);
    unsigned short port = htons(((struct pth_args *)args)->addr.sin_port);

    struct info_t info;
    int ret = 0;
    char cmd = 0;

    pthread_detach(pthread_self()); //分离

    while (1)
    {
        //接收
        memset(&info, 0, sizeof(info));
        ret = recv(fd, &info, sizeof(info), 0);
        if (ret < 0)
        {
            PERR(((struct pth_args *)args)->addr, "");
            perror("");
        }

        cmd = info.cmd;                            //命令码
        PERR(((struct pth_args *)args)->addr, ""); //接收到的数据
        printf("cmd=%d\n", cmd);

        switch (cmd)
        {
        case 0x02:
            /* 登录 */
            ret = login((struct pth_args *)args, &info);
            break;
        case 0x03:
            /* 入职 */
            ret = onboarding((struct pth_args *)args, &info);
            break;
        case 0x04:
            /* 离职 */
            ret = resign((struct pth_args *)args, &info);
            break;
        case 0x05:
            /* 信息修改 */
            ret = modify((struct pth_args *)args, &info);
            break;
        case 0x06:
            /* 打卡 */
            break;
        case 0x07:
            /* 请假 */
            break;
        case 0x08:
            /* 信息查询_name */
            ret = find_by_name((struct pth_args *)args, &info);
            break;
        case 0x09:
            /* 信息查询_id */
            ret = find_by_id((struct pth_args *)args, &info);
            break;
        case 0x0A:
            /* 请假审批 */
            break;
        case 0x0B:
            /* 撤销请假 */
            break;
        case 0x0C:
            /* 考勤记录查询 */
            break;
        case 0x0D:
            /* 修改账户信息 */
            ret = change_account((struct pth_args *)args, &info);
            break;
        case 0x00:
            /* 客户端关闭 */
            close(fd);
            printf("fd=%d\n", fd);
            return NULL;
            break;

        default:
            break;
        }
    }
    return NULL;
}

// 0x02登录
int login(struct pth_args *args, struct info_t *info)
{
    char usr[21] = {0};
    char pwd[21] = {0};
    char sql[512] = {0};
    int permission = 0; //权限
    char *msg = NULL;
    int ret;
    char **result = NULL;
    int row, col;

    struct info_t info_back;
    memset(&info_back, 0, sizeof(info_back));

    memcpy(usr, info->usr, 20);
    memcpy(pwd, info->pwd, 20);

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "select * from info where usr=\"%s\" and pwd=\"%s\";", usr, pwd);
    printf("%s\n", sql);
    ret = sqlite3_get_table(db, sql, &result, &row, &col, &msg);
    if (ret != SQLITE_OK)
    {
        ERR(msg);
        return -1;
    }

    // printf("row=%d, col=%d\n", row, col);
    // for(int i=0; i<(row+1)*col; i++)
    //     printf("%s\n", result[i]);

    if (row == 0)
    {
        info_back.errcode = 0x00; //失败
    }
    else
    {
        info_back.errcode = 0x01;                                     //成功
        info_back.isend = 0x00;                                       //无后续
        memcpy(info_back.id, result[13], sizeof(result[13]));         // id
        memcpy(info_back.name, result[14], sizeof(result[14]));       // name
        memcpy(info_back.tel, result[15], sizeof(result[15]));        // tel
        info_back.sex = result[16][0];                                // sex
        memcpy(info_back.birth, result[17], sizeof(result[17]));      // birth
        memcpy(info_back.addr, result[18], sizeof(result[18]));       // addr
        memcpy(info_back.entrytime, result[19], sizeof(result[19]));  // entrytime
        memcpy(info_back.job, result[20], sizeof(result[20]));        // job
        memcpy(info_back.department, result[21], sizeof(result[21])); // department
        memcpy(info_back.salary, result[22], sizeof(result[22]));     // salary
        memcpy(info_back.rank, result[23], sizeof(result[23]));       // rank
    }

    sqlite3_free_table(result);

    //发送结果
    ret = send(args->fd, &info_back, sizeof(info_back), 0);
    if (ret == -1)
    {
        PERR(args->addr, "send");
        perror("");
        return -1;
    }

    PERR(args->addr, "login okay");
    return 0;
}

// 0x03 入职
int onboarding(struct pth_args *args, struct info_t *info)
{
    int ret = 0;
    char *msg = NULL;
    char sql[512] = {0};
    char **result = NULL;
    int row, col;

    struct info_t info_back;

    //..查找usr中有无重复
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "select * from info where usr =\"%s\";", info->usr);
    puts(sql);
    ret = sqlite3_get_table(db, sql, &result, &row, &col, &msg);
    if (ret != SQLITE_OK)
    {
        ERR(msg);
        return -1;
    }

    sqlite3_free_table(result);

    if (row > 0)
    {
        //用户名重复
        PERR(args->addr, "usr repeat");
        info_back.errcode == 0x00; //失败
    }
    else
    {
        sprintf(sql, "insert into info values(\"%s\", \"%s\", \"%s\", \"%c\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\", \"%s\");",
                info->id, info->name, info->tel, info->sex, info->birth, info->addr, info->entrytime, info->job, info->department, info->salary, info->rank, info->usr, info->pwd);
        puts(sql);
        ret = sqlite3_exec(db, sql, NULL, NULL, &msg);
        if (ret != SQLITE_OK)
        {
            ERR(msg);
            return -1;
        }

        info_back.errcode = 0x01; //成功
    }

    //发送结果
    ret = send(args->fd, &info_back, sizeof(info_back), 0);
    if (ret == -1)
    {
        PERR(args->addr, "send");
        perror("");
        return -1;
    }

    PERR(args->addr, "onboarding okay");
    return 0;
}

// 0x08 按照姓名查询
int find_by_name(struct pth_args *args, struct info_t *info)
{
    char sql[512] = {0};
    char *msg = NULL;
    int ret;
    char **result = NULL;
    int row, col;
    struct info_t info_back;

    sprintf(sql, "select * from info where name = \"%s\";", info->name);
    puts(sql);
    ret = sqlite3_get_table(db, sql, &result, &row, &col, &msg);
    if (ret != SQLITE_OK)
    {
        ERR(msg);
        return -1;
    }

    // 0 row=0 col=0
    // 1 row=1 col=13

    if (row == 0) //未找到
    {
        info_back.errcode = 0x00; //未找到
        ret = send(args->fd, &info_back, sizeof(struct info_t), 0);
        if (ret == -1)
        {
            PERR(args->addr, "send error");
            perror("");
            return -1;
        }
        PERR(args->addr, "find_by_name not found");
        SHOW_V(info_back);
    }
    else //找到了
    {
        for (int i = row; i > 0; i--)
        {
            memset(&info_back, 0, sizeof(struct info_t));
            info_back.errcode = 0x01; //找到了
            info_back.isend = i - 1;
            memcpy(info_back.id, result[i * 13 + 0], strlen(result[i * 13 + 0]));
            memcpy(info_back.name, result[i * 13 + 1], strlen(result[i * 13 + 1]));
            memcpy(info_back.tel, result[i * 13 + 2], strlen(result[i * 13 + 2]));
            info_back.sex = *(result[i * 13 + 3]);
            memcpy(info_back.job, result[i * 13 + 7], strlen(result[i * 13 + 7]));
            memcpy(info_back.department, result[i * 13 + 8], strlen(result[i * 13 + 8]));

            if (atoi(info->rank) == 66 || atoi(info->rank) == 99)
            {
                //人事 boss
                memcpy(info_back.birth, result[i * 13 + 4], strlen(result[i * 13 + 4]));
                memcpy(info_back.addr, result[i * 13 + 5], strlen(result[i * 13 + 5]));
                memcpy(info_back.entrytime, result[i * 13 + 6], strlen(result[i * 13 + 6]));
                memcpy(info_back.rank, result[i * 13 + 10], strlen(result[i * 13 + 10]));
            }
            if (atoi(info->rank) == 88 || atoi(info->rank) == 99)
            {
                //财务 boss
                memcpy(info_back.salary, result[i * 13 + 9], strlen(result[i * 13 + 9]));
            }

            ret = send(args->fd, &info_back, sizeof(struct info_t), 0);
            if (ret == -1)
            {
                PERR(args->addr, "send error");
                perror("");
                continue; //不管这次错误，继续下一次
            }
            SHOW_V(info_back);
        }
    }

    sqlite3_free_table(result);
    ERR("find by name okay");
    return 0;
}

// 0x09 按照工号查询
int find_by_id(struct pth_args *args, struct info_t *info)
{
    char sql[512] = {0};
    char *msg = NULL;
    int ret;
    char **result = NULL;
    int row, col;
    struct info_t info_back;

    sprintf(sql, "select * from info where id = \"%s\";", info->id);
    puts(sql);
    ret = sqlite3_get_table(db, sql, &result, &row, &col, &msg);
    if (ret != SQLITE_OK)
    {
        PERR(args->addr, msg);
        return -1;
    }

    // 0 row=0 col=0
    // 1 row=1 col=13

    memset(&info_back, 0, sizeof(struct info_t));
    if (row == 0) //未找到
    {
        info_back.errcode = 0x00;
    }
    else //找到了
    {
        memset(&info_back, 0, sizeof(struct info_t));
        info_back.errcode = 0x01;
        info_back.isend = 0;
        int i = 1;
        memcpy(info_back.id, result[i * 13 + 0], strlen(result[i * 13 + 0]));
        memcpy(info_back.name, result[i * 13 + 1], strlen(result[i * 13 + 1]));
        memcpy(info_back.tel, result[i * 13 + 2], strlen(result[i * 13 + 2]));
        info_back.sex = *(result[i * 13 + 3]);
        memcpy(info_back.job, result[i * 13 + 7], strlen(result[i * 13 + 7]));
        memcpy(info_back.department, result[i * 13 + 8], strlen(result[i * 13 + 8]));

        if (atoi(info->rank) == 66 || atoi(info->rank) == 99)
        {
            //人事 boss
            memcpy(info_back.birth, result[i * 13 + 4], strlen(result[i * 13 + 4]));
            memcpy(info_back.addr, result[i * 13 + 5], strlen(result[i * 13 + 5]));
            memcpy(info_back.entrytime, result[i * 13 + 6], strlen(result[i * 13 + 6]));
            memcpy(info_back.rank, result[i * 13 + 10], strlen(result[i * 13 + 10]));
        }
        if (atoi(info->rank) == 88 || atoi(info->rank) == 99)
        {
            //财务 boss
            memcpy(info_back.salary, result[i * 13 + 9], strlen(result[i * 13 + 9]));
        }
    }

    sqlite3_free_table(result);

    SHOW_V(info_back);
    ret = send(args->fd, &info_back, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        PERR(args->addr, "send error");
        perror("");
        return -1;
    }

    ERR("find by id okay");
    return 0;
}

// 0x04 注销
int resign(struct pth_args *args, struct info_t *info)
{
    int ret=0;
    char *msg=NULL;
    char sql[512] = {0};

    sprintf(sql, "delete from info where id=\"%s\";", info->id);
    puts(sql);
    ret = sqlite3_exec(db, sql, NULL, NULL, &msg);
    if (ret != SQLITE_OK)
    {
        PERR(args->addr, msg);
        return -1;
    }

    struct info_t info_back;
    info_back.errcode=0x01; //成功

    ret=send(args->fd, &info_back, sizeof(struct info_t), 0);
    if(ret == -1)
    {
        PERR(args->addr, "resign send");
        perror("");
        return -1;
    }

    ERR("resign okay");
    return 0;
}


// 0x05 修改信息
int modify(struct pth_args *args, struct info_t *info)
{
    char sql[521]={0};
    int ret=0;
    char *msg=NULL;

    if(info->name[0]>0)
    {
        memset(sql, 0, sizeof(sql));
        sprintf(sql, "update info set name=\"%s\" where id=\"%s\";", info->name, info->id);
    }
    if(info->tel[0]>0)
    {
        memset(sql, 0, sizeof(sql));
        sprintf(sql, "update info set tel=\"%s\" where id=\"%s\";", info->tel, info->id);
    }
    if(info->sex>0)
    {
        memset(sql, 0, sizeof(sql));
        sprintf(sql, "update info set sex=\"%c\" where id=\"%s\";", info->sex, info->id);
    }
    if(info->birth[0]>0)
    {
        memset(sql, 0, sizeof(sql));
        sprintf(sql, "update info set birth=\"%s\" where id=\"%s\";", info->birth, info->id);
    }
    if(info->addr[0]>0)
    {
        memset(sql, 0, sizeof(sql));
        sprintf(sql, "update info set addr=\"%s\" where id=\"%s\";", info->addr, info->id);
    }
    if(info->entrytime[0]>0)
    {
        memset(sql, 0, sizeof(sql));
        sprintf(sql, "update info set entrytime=\"%s\" where id=\"%s\";", info->entrytime, info->id);
    }
    if(info->job[0]>0)
    {
        memset(sql, 0, sizeof(sql));
        sprintf(sql, "update info set job=\"%s\" where id=\"%s\";", info->job, info->id);
    }
    if(info->department[0]>0)
    {
        memset(sql, 0, sizeof(sql));
        sprintf(sql, "update info set department=\"%s\" where id=\"%s\";", info->department, info->id);
    }
    if(info->rank[0]>0)
    {
        memset(sql, 0, sizeof(sql));
        sprintf(sql, "update info set rank=\"%s\" where id=\"%s\";", info->rank, info->id);
    }
    if(info->salary[0]>0)
    {
        memset(sql, 0, sizeof(sql));
        sprintf(sql, "update info set salary=\"%s\" where id=\"%s\";", info->salary, info->id);
    }

    puts(sql);
    ret = sqlite3_exec(db, sql, NULL, NULL, &msg);
    if (ret != SQLITE_OK)
    {
        PERR(args->addr, msg);
        return -1;
    }

    struct info_t info_back;
    info_back.errcode=0x01; //成功


    ret=send(args->fd, &info_back, sizeof(struct info_t), 0);
    if(ret == -1)
    {
        PERR(args->addr, "modify send");
        perror("");
        return -1;
    }

    ERR("modify okay");
    return 0;
}


// 0x0D 修改账户信息
int change_account(struct pth_args *args, struct info_t *info)
{
    int ret=0;
    char sql[512]={0};
    char *msg=NULL;
    char **result = NULL;
    int row, col;
    struct info_t info_back;
    char oldusr[21]={0};

    //..查找usr中有无重复
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "select usr from info where id =\"%s\";", info->id);
    puts(sql);
    ret = sqlite3_get_table(db, sql, &result, &row, &col, &msg);
    if (ret != SQLITE_OK)
    {
        ERR(msg);
        return -1;
    }

    memcpy(oldusr, result[1], strlen(result[1]));
    printf("oldusr=%s\n", oldusr);

    sqlite3_free_table(result);

    memset(sql, 0, sizeof(sql));
    sprintf(sql, "select usr from info where usr =\"%s\";", info->usr);
    puts(sql);
    ret = sqlite3_get_table(db, sql, &result, &row, &col, &msg);
    if (ret != SQLITE_OK)
    {
        ERR(msg);
        return -1;
    }

    if (row > 0 && memcmp(oldusr, info->usr, strlen(oldusr))!=0)
    {
        //用户名重复
        PERR(args->addr, "usr repeat");
        info_back.errcode == 0x00; //失败
    }
    else
    {
        sprintf(sql, "update info set usr=\"%s\",pwd=\"%s\" where id=\"%s\";",\
                    info->usr, info->pwd, info->id);
        puts(sql);
        ret = sqlite3_exec(db, sql, NULL, NULL, &msg);
        if (ret != SQLITE_OK)
        {
            ERR(msg);
            return -1;
        }
        info_back.errcode = 0x01; //成功
    }

    sqlite3_free_table(result);

    ret=send(args->fd, &info_back, sizeof(struct info_t), 0);
    if(ret == -1)
    {
        PERR(args->addr, "modify send");
        perror("");
        return -1;
    }

    ERR("change_account okay");
    return 0;
}
