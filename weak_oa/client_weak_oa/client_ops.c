#include "client_ops.h"

extern int fd;
extern char name[21];
extern char id[11];
extern char department[41];
extern char rank[3];

int sock_init(const char *ip, const unsigned short port)
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
        goto ERR2;
    }

    // 3.连接服务器
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    ret = connect(sfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
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

// 0x02登录
//登录成功返回0x01 权限 信息
int login(struct info_t *info)
{
    int ret = 0;
    info->cmd = 0x02;
    struct info_t info_back;

    ret = send(fd, info, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("send error");
        perror("");
        return ret;
    }

    //接收返回
    ret = recv(fd, &info_back, sizeof(info_back), 0);
    if (ret == -1)
    {
        ERR("recv error");
        perror("");
        return ret;
    }

    if (info_back.errcode == 0x00)
    {
        printf("login error\n");
        return -1;
    }

    memcpy(name, info_back.name, 20);
    memcpy(id, info_back.id, 10);
    memcpy(department, info_back.department, 40);
    memcpy(rank, info_back.rank, 2);

    printf("name=%s,rand=%s\n", info_back.name, info_back.rank);

    return 0;
}

// 0x03 入职
int c_onboarding(struct info_t *info)
{
    info->cmd = 0x03;
    int ret = send(fd, info, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("send");
        perror("");
        return -1;
    }

    struct info_t info_back;
    ret = recv(fd, &info_back, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("recv");
        perror("");
        return -1;
    }

    if (info_back.errcode == 0)
        return -1;

    return 0;
}

// 0x08 按姓名查找
int c_find_by_name(struct info_t *info, struct info_t **info_back, int *cnt)
{
    int i = 0;
    struct info_t tmp;
    *info_back = NULL;

    info->cmd = 0x08;
    memcpy(info->rank, rank, sizeof(info->rank));

    int ret = send(fd, info, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("send");
        perror("");
        return -1;
    }

    for (i = 0;; i++)
    {
        memset(&tmp, 0, sizeof(struct info_t));
        ret = recv(fd, &tmp, sizeof(struct info_t), 0);
        if (ret == -1)
        {
            ERR("recv");
            perror("");
            goto E1;
        }
        
        if (*info_back == NULL) //接收到的第一次信息
        {
            //未找到
            if (tmp.errcode == 0x00)
            {
                *cnt = 0;
                ERR("not found");
                goto E1;
            }
            *cnt = tmp.isend + 1;
            *info_back = (struct info_t *)malloc(sizeof(struct info_t) * (*cnt));

            if (info_back == NULL)
            {
                ERR("malloc error");
                goto E1;
            }
        }

        *(*info_back+i) = tmp;

        // SHOW_V((*(*info_back+i)));

        if (tmp.isend == 0)
            break;
    }

    return 0;

E1:
    free(*info_back);
    return 0;
}

void free_find_by_name(struct info_t **info)
{
    free(*info);
}

// 0x09 按照工号查询
int c_find_by_id(struct info_t *info)
{
    info->cmd = 0x09;
    memcpy(info->rank, rank, sizeof(info->rank));

    int ret = send(fd, info, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("send");
        perror("");
        return -1;
    }

    memset(info, 0, sizeof(struct info_t));
    ret = recv(fd, info, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("recv");
        perror("");
        return -1;
    }

    if(info->errcode ==0)
        return 0;  //未找到


    return 0;
}

// 0x04 离职
int c_resign(struct info_t *info)
{
    info->cmd = 0x04;

    int ret = send(fd, info, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("send");
        perror("");
        return -1;
    }

    memset(info, 0, sizeof(struct info_t));
    ret = recv(fd, info, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("recv");
        perror("");
        return -1;
    }

    if(info->errcode==0)
        return -1;  //失败

    
    return 0;
}


// 0x05 信息修改
int c_modify(struct info_t *info, struct info_t *info_new)
{
    memcpy(info_new->id, info->id, sizeof(info->id));//必须按照id来修改
    info_new->cmd=0x05;

    int ret = send(fd, info_new, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("send");
        perror("");
        return -1;
    }

    memset(info_new, 0, sizeof(struct info_t));
    ret = recv(fd, info_new, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("recv");
        perror("");
        return -1;
    }

    if(info_new->errcode == 0)  //修改失败
        return -1;

    return 0;
}


//0x0D 修改账户名 密码
int c_change_account(struct info_t *info)
{
    info->cmd =0x0D;
    int ret = send(fd, info, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("send");
        perror("");
        return -1;
    }

    memset(info, 0, sizeof(struct info_t));
    ret = recv(fd, info, sizeof(struct info_t), 0);
    if (ret == -1)
    {
        ERR("recv");
        perror("");
        return -1;
    }

    if(info->errcode == 0)  //修改失败
        return -1;

    return 0;
}


//输出str 还是*
char *istar(char *str, char *s)
{
    if(strlen(str)==0)
        return s;
    return str;
}
