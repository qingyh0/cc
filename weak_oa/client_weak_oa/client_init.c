#include "client_init.h"
#include "client_ops.h"

extern int fd;
extern char name[21];
extern char id[11];
extern char department[41];
extern char rank[3];

int getch(void);

//信号处理函数
void handler(int sig)
{
    if (sig == SIGINT)
    {
        struct info_t info;
        info.cmd = 0x00;
        send(fd, &info, sizeof(info), 0); //向服务器发送退出信号
        close(fd);
        exit(0);
    }
}

void finish(void)
{
    struct info_t info;
    info.cmd = 0x00;
    send(fd, &info, sizeof(info), 0); //向服务器发送退出信号
    close(fd);
}

//主界面初始化
int login_init()
{
    char usr[21] = {0};
    char pwd[21] = {0};
    char ch = 0, i;
    struct info_t info;
    int ret = 0;

    while (1)
    {
        while (1)
        {
            memset(usr, 0, sizeof(usr));
            memset(pwd, 0, sizeof(pwd));
            printf("\033c");
            printf("***********************************\n");
            printf("*             登录                *\n");
            printf("***********************************\n\n\n");
            printf("账号<<");
            scanf("%s", usr);
            getchar(); //吸收\n

            if (strlen(usr) > 20)
            {
                printf(">>用户名超出20位，按任意键重试\n");
                getchar();
                continue;
            }

            printf("密码<<");
            fflush(stdout);
            for (i = 0; i < 21 && (ch = getch()) != '\n'; i++)
            {
                pwd[i] = ch;
                printf("\b*");
                fflush(stdout);
            }
            if (i == 21)
            {
                printf(">>密码超出20位，按任意键重试\n");
                getchar();
                continue;
            }

            break;
        }

        printf("usr=%s,pwd=%s\n", usr, pwd);

        //组包
        memset(&info, 0, sizeof(info));
        memcpy(&(info.usr), usr, 20);
        memcpy(&(info.pwd), pwd, 20);

        //登录
        ret = login(&info);
        if (ret != 0)
        {
            printf("按任意键重试\n");
            getchar();
            continue;
        }
        break;
    }

    return 0;
}

//------------------1.1.1 按姓名查询
int find_by_name(void)
{
    printf("\033c");
    printf("*************************************\n");
    printf("**************按姓名查询**************\n");
    printf("*************************************\n");
    printf("\n");

    struct info_t info;
    char ch = 0;
    int i = 0, ret;
    char *s="*";

    while (1)
    {
        memset(&(info), 0, sizeof(info));
        printf("输入姓名[<=20]<<");
        for (i = 0, ch = getchar(); i < 21; i++, ch = getchar())
        {
            if (ch != 10)
                info.name[i] = ch;
            else
                break;
        }
        printf("-->%s\n", info.name);
        if (i == 21 || i == 0)
        {
            printf("位数错误，请重新输入\n");
            continue;
        }
        break;
    }

    printf("**********************************\n");
    //
    struct info_t *info_back = NULL;
    int cnt = 0;
    ret = c_find_by_name(&info, &info_back, &cnt);
    if (ret != 0)
    {
        printf("查找失败，请重试\n");
        return -1;
    }

    // printf("cnt=%d\n", cnt);

    printf("id\tname\ttel\tsex\tbirth\taddr\tentrytime\tjob\tdepartment\tsalary\trank\n");
    for (int i = 0; i < cnt; i++)
    {
        printf("%s\t%s\t%s\t%c\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",\
               (info_back + i)->id, (info_back + i)->name, (info_back + i)->tel, (info_back + i)->sex,\
               istar((info_back + i)->birth, s), istar((info_back + i)->addr, s), istar((info_back + i)->entrytime, s),\
            (info_back + i)->job, (info_back + i)->department, istar((info_back + i)->salary, s), istar((info_back + i)->rank, s));
    }

    printf("**********************************\n");
    free_find_by_name(&info_back); //释放空间
    printf("按任意键返回<<");
    while (getchar() != 10)
    {
    }

    return 0;
}

//------------------1.1.2 按工号查询
int find_by_id(void)
{
    printf("\033c");
    printf("***********************************\n");
    printf("**************按工号查询************\n");
    printf("***********************************\n");
    printf("\n");

    struct info_t info;
    char ch = 0;
    int i = 0, ret;
    char *s="*";

    while (1)
    {
        memset(&(info), 0, sizeof(info));
        printf("输入工号[<=10]<<");
        for (i = 0, ch = getchar(); i < 11; i++, ch = getchar())
        {
            if (ch != 10)
                info.id[i] = ch;
            else
                break;
        }
        printf("-->%s\n", info.id);
        if (i == 11 || i == 0)
        {
            printf("位数错误，请重新输入\n");
            continue;
        }
        break;
    }

    printf("**********************************\n");
    //
    int cnt = 0;
    ret = c_find_by_id(&info);
    if (ret != 0)
    {
        printf("查找失败，请重试\n");
        return -1;
    }
    // SHOW_V(info);
    printf("id\tname\ttel\tsex\tbirth\taddr\tentrytime\tjob\tdepartment\tsalary\trank\n");
    printf("%s\t%s\t%s\t%c\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
           info.id, info.name, info.tel, info.sex,
           istar(info.birth, s), istar(info.addr, s), istar(info.entrytime, s), info.job,
           info.department, istar(info.salary, s), istar(info.rank, s));

    printf("**********************************\n");
    printf("按任意键返回<<");
    while (getchar() != 10)
    {
    }

    return 0;
}

//------------------1.1.3 员工入职
int onboarding(void)
{
    printf("\033c");
    printf("***************************************\n");
    printf("**************员工入职******************\n");
    printf("***************************************\n");
    printf("\n");
    struct info_t info;
    char ch = 0;
    int i = 0;
    int ret;

B1:
    ch = 0;
    memset(&(info.id), 0, sizeof(info.id));
    printf("输入工号[<=10]<<");
    for (i = 0, ch = getchar(); i < 11; i++, ch = getchar())
    {
        if (ch != 10)
            info.id[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.id);
    if (i == 11 || i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B1;
    }

B2:
    ch = 0;
    memset(&(info.name), 0, sizeof(info.name));
    printf("输入姓名[<=20]<<");
    for (i = 0, ch = getchar(); i < 21; i++, ch = getchar())
    {
        if (ch != 10)
            info.name[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.name);
    if (i == 21 || i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B2;
    }
B3:
    ch = 0;
    memset(&(info.tel), 0, sizeof(info.tel));
    printf("输入电话[<=14]<<");
    for (i = 0, ch = getchar(); i < 15; i++, ch = getchar())
    {
        if (ch != 10)
            info.tel[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.tel);
    if (i == 15 || i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B3;
    }
B4:
    ch = 0;
    memset(&(info.sex), 0, sizeof(info.sex));
    printf("输入性别[m/f]<<");
    ch = getchar();
    if (ch != 'm' && ch != 'f')
    {
        printf("输入错误，请重新输入\n");
        goto B4;
    }
    while (getchar() != 10)
    {
    }
    info.sex = ch;
    printf("-->%c\n", info.sex);
B5:
    ch = 0;
    memset(&(info.birth), 0, sizeof(info.birth));
    printf("输入生日[<=10]<<");
    for (i = 0, ch = getchar(); i < 11; i++, ch = getchar())
    {
        if (ch != 10)
            info.birth[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.birth);
    if (i == 11 || i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B5;
    }
B6:
    ch = 0;
    memset(&(info.addr), 0, sizeof(info.addr));
    printf("输入住址[<=200]<<");
    for (i = 0, ch = getchar(); i < 201; i++, ch = getchar())
    {
        if (ch != 10)
            info.addr[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.addr);
    if (i == 201 || i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B6;
    }
B7:
    ch = 0;
    memset(&(info.entrytime), 0, sizeof(info.entrytime));
    printf("输入入职日期[<=10]<<");
    for (i = 0, ch = getchar(); i < 11; i++, ch = getchar())
    {
        if (ch != 10)
            info.entrytime[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.entrytime);
    if (i == 11 | i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B7;
    }
B8:
    ch = 0;
    memset(&(info.job), 0, sizeof(info.job));
    printf("输入职位[<=40]<<");
    for (i = 0, ch = getchar(); i < 41; i++, ch = getchar())
    {
        if (ch != 10)
            info.job[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.job);
    if (i == 41 || i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B8;
    }
B9:
    ch = 0;
    memset(&(info.department), 0, sizeof(info.department));
    printf("输入部门[<=40]<<");
    for (i = 0, ch = getchar(); i < 41; i++, ch = getchar())
    {
        if (ch != 10)
            info.department[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.department);
    if (i == 41 || i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B9;
    }
B10:
    ch = 0;
    memset(&(info.salary), 0, sizeof(info.salary));
    printf("输入薪水[<=9]<<");
    for (i = 0, ch = getchar(); i < 10; i++, ch = getchar())
    {
        if (ch != 10)
            info.salary[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.salary);
    if (i == 10 || i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B10;
    }
B11:
    ch = 0;
    memset(&(info.rank), 0, sizeof(info.rank));
    printf("输入权限[<=2]<<");
    for (i = 0, ch = getchar(); i < 3; i++, ch = getchar())
    {
        if (ch != 10)
            info.rank[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.rank);
    if (i == 3 || i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B11;
    }
B12:
    ch = 0;
    memset(&(info.usr), 0, sizeof(info.usr));
    printf("输入用户名[<=20]<<");
    for (i = 0, ch = getchar(); i < 21; i++, ch = getchar())
    {
        if (ch != 10)
            info.usr[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.usr);
    if (i == 21 || i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B12;
    }
B13:
    ch = 0;
    memset(&(info.pwd), 0, sizeof(info.pwd));
    printf("输入密码[<=20]<<");
    for (i = 0, ch = getchar(); i < 21; i++, ch = getchar())
    {
        if (ch != 10)
            info.pwd[i] = ch;
        else
            break;
    }
    printf("-->%s\n", info.pwd);
    if (i == 21 || i == 0)
    {
        printf("位数错误，请重新输入\n");
        goto B13;
    }

    ret = c_onboarding(&info);
    if (ret != 0)
    {
        printf("用户名重复，回车重试，其他键返回\n");
        ch = getchar();
        if (ch == 10)
            goto B12;
        else
        {
            while (getchar() != 10)
            {
            }
            return -1;
        }
    }

    printf("注册成功，回车继续\n");
    while (getchar() != 10)
    {
    }

    return 0;
}

//------------------1.1.4 离职
int resign(void)
{
    printf("\033c");
    printf("***********************************\n");
    printf("**************员工离职**************\n");
    printf("***********************************\n");
    printf("\n");

    struct info_t info;
    char ch = 0;
    int i = 0, ret;
    char *s="*";

    while (1)
    {
        memset(&(info), 0, sizeof(info));
        printf("输入工号[<=10]<<");
        for (i = 0, ch = getchar(); i < 11; i++, ch = getchar())
        {
            if (ch != 10)
                info.id[i] = ch;
            else
                break;
        }
        printf("-->%s\n", info.id);
        if (i == 11 || i == 0)
        {
            printf("位数错误，请重新输入\n");
            continue;
        }
        break;
    }

    printf("**********************************\n");
    //
    int cnt = 0;
    ret = c_find_by_id(&info);
    if (ret != 0)
    {
        printf("查找失败，请重试\n");
        return -1;
    }
    printf("id\tname\ttel\tsex\tbirth\taddr\tentrytime\tjob\tdepartment\tsalary\trank\n");
    printf("%s\t%s\t%s\t%c\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
           info.id, info.name, info.tel, info.sex,
           istar(info.birth, s), istar(info.addr, s), istar(info.entrytime,s), info.job,
           info.department, istar(info.salary, s), istar(info.rank,s));

    printf("**********************************\n");
    while (1)
    {
        printf("注销此人账户[y/n]<<");
        ch = getchar();
        while (getchar() != 10)
        {
        }
        switch (ch)
        {
        case 'y':
            /* 注销 */
            if (c_resign(&info) != 0)
            {
                printf("注销失败，按任意键退出\n");
                while (getchar() != 10)
                {
                }
                return -1;
            }
            printf("注销成功，按任意键退出\n");
            while (getchar() != 10)
            {
            }
            return 0;
        case 'n':
            /* 退出 */
            return 0;
            break;
        default:
            break;
        }
    }
    printf("按任意键返回<<");
    while (getchar() != 10)
    {
    }

    return 0;
}

//------------------1.1.5 员工信息修改
int modify(void)
{
    printf("\033c");
    printf("***********************************\n");
    printf("**************信息修改**************\n");
    printf("***********************************\n");
    printf("\n");

    struct info_t info, info_new;
    char ch = 0;
    int i = 0, ret;
    int rank_flag = 0;
    char *s="*";

    while (1)
    {
        memset(&(info), 0, sizeof(info));
        printf("输入工号[<=10]<<");
        for (i = 0, ch = getchar(); i < 11; i++, ch = getchar())
        {
            if (ch != 10)
                info.id[i] = ch;
            else
                break;
        }
        printf("-->%s\n", info.id);
        if (i == 11 || i == 0)
        {
            printf("位数错误，请重新输入\n");
            continue;
        }
        break;
    }

    printf("**********************************\n");
    //
    int cnt = 0;
    ret = c_find_by_id(&info);
    if (ret != 0)
    {
        printf("查找失败，请重试\n");
        return -1;
    }
    printf("id\tname\ttel\tsex\tbirth\taddr\tentrytime\tjob\tdepartment\tsalary\trank\n");
    printf("%s\t%s\t%s\t%c\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
           info.id, info.name, info.tel, info.sex,
           istar(info.birth, s), istar(info.addr, s), istar(info.entrytime,s), info.job,
           info.department, istar(info.salary, s), istar(info.rank,s));

    printf("**********************************\n");

    if (info.errcode == 0x00)
    {
        printf("未找到此id，按任意键退出\n");
        while (getchar() != 10)
        {
        }
        return 0;
    }

    rank_flag = atoi(rank);
    if (rank_flag == 66 || rank_flag == 99)
    {
        // puts("\t[1]id");
        puts("\t[2]name");
        puts("\t[3]tel");
        puts("\t[4]sex");
        puts("\t[5]birth");
        puts("\t[6]addr");
        puts("\t[7]entrytime");
        puts("\t[8]job");
        puts("\t[9]department");
        puts("\t[a]rank");
    }
    if (rank_flag == 88 || rank_flag == 99)
    {
        puts("\t[b]salary");
    }
    printf("选择修改内容<<");
    ch = getchar();
    while (getchar() != 10)
    {
    }
    memset(&info_new, 0, sizeof(struct info_t));
    switch (ch) //以下输入未做检查
    {
    case '2':
        /* name */
        printf("输入新的name[<=20]<<");
        scanf("%s", info_new.name);
        getchar();
        ret = c_modify(&info, &info_new);
        if (ret != 0)
            printf("修改失败，按任意键继续\n");
        printf("修改成功，按任意键继续\n");
        while (getchar() != 10)
        {
        }
        break;
    case '3':
        /* tel */
        printf("tel[<=14]<<");
        scanf("%s", info_new.tel);
        getchar();
        ret = c_modify(&info, &info_new);
        if (ret != 0)
            printf("修改失败，按任意键继续\n");
        printf("修改成功，按任意键继续\n");
        while (getchar() != 10)
        {
        }
        break;
    case '4':
        /* sex */
        printf("sex[m/f]<<");
        scanf("%c", &info_new.sex);
        getchar();
        ret = c_modify(&info, &info_new);
        if (ret != 0)
            printf("修改失败，按任意键继续\n");
        printf("修改成功，按任意键继续\n");
        while (getchar() != 10)
        {
        }
        break;
    case '5':
        /* birth */
        printf("birth[<=10]<<");
        scanf("%s", info_new.birth);
        getchar();
        ret = c_modify(&info, &info_new);
        if (ret != 0)
            printf("修改失败，按任意键继续\n");
        printf("修改成功，按任意键继续\n");
        while (getchar() != 10)
        {
        }
        break;
    case '6':
        /* addr */
        printf("addr[<=200]<<");
        scanf("%s", info_new.addr);
        getchar();
        ret = c_modify(&info, &info_new);
        if (ret != 0)
            printf("修改失败，按任意键继续\n");
        printf("修改成功，按任意键继续\n");
        while (getchar() != 10)
        {
        }
        break;
    case '7':
        /* entrytime */
        printf("entrytime[<=10]<<");
        scanf("%s", info_new.entrytime);
        getchar();
        ret = c_modify(&info, &info_new);
        if (ret != 0)
            printf("修改失败，按任意键继续\n");
        printf("修改成功，按任意键继续\n");
        while (getchar() != 10)
        {
        }
        break;
    case '8':
        /* job */
        printf("job[<=40]<<");
        scanf("%s", info_new.job);
        getchar();
        ret = c_modify(&info, &info_new);
        if (ret != 0)
            printf("修改失败，按任意键继续\n");
        printf("修改成功，按任意键继续\n");
        while (getchar() != 10)
        {
        }
        break;
    case '9':
        /* department */
        printf("department[<=40]<<");
        scanf("%s", info_new.department);
        getchar();
        ret = c_modify(&info, &info_new);
        if (ret != 0)
            printf("修改失败，按任意键继续\n");
        printf("修改成功，按任意键继续\n");
        while (getchar() != 10)
        {
        }
        break;
    case 'a':
        /* rank */
        printf("rank[=2]<<");
        scanf("%s", info_new.rank);
        getchar();
        ret = c_modify(&info, &info_new);
        if (ret != 0)
            printf("修改失败，按任意键继续\n");
        printf("修改成功，按任意键继续\n");
        while (getchar() != 10)
        {
        }
        break;
    case 'b':
        /* salary */
        printf("salary[<=9]<<");
        scanf("%s", info_new.salary);
        getchar();
        ret = c_modify(&info, &info_new);
        if (ret != 0)
            printf("修改失败，按任意键继续\n");
        printf("修改成功，按任意键继续\n");
        while (getchar() != 10)
        {
        }
        break;
    default:
        printf("按任意键返回<<");
        while (getchar() != 10)
        {
        }
        break;
    }

    return 0;
}

//-----------------2. 账户信息修改
int change_account(void)
{
    printf("\033c");
    printf("***********************************\n");
    printf("**************账户修改**************\n");
    printf("***********************************\n");
    printf("\n");

    struct info_t info;
    char ch = 0;
    int i = 0, ret;
    int rank_flag = 0;

    while (1)
    {
        memset(info.usr, 0, sizeof(info.usr));
        printf("输入新的用户名[<=20]<<");
        for (i = 0, ch = getchar(); i < 21; i++, ch = getchar())
        {
            if (ch != 10)
                info.usr[i] = ch;
            else
                break;
        }
        printf("-->%s\n", info.usr);
        if (i == 21 || i == 0)
        {
            printf("位数错误，请重新输入\n");
            continue;
        }
        break;
    }
    while (1)
    {
        memset(info.pwd, 0, sizeof(info.pwd));
        printf("输入新的密码[<=20]<<");
        for (i = 0, ch = getchar(); i < 21; i++, ch = getchar())
        {
            if (ch != 10)
                info.pwd[i] = ch;
            else
                break;
        }
        printf("-->%s\n", info.pwd);
        if (i == 21 || i == 0)
        {
            printf("位数错误，请重新输入\n");
            continue;
        }
        break;
    }

    memcpy(info.id, id, strlen(id));
    ret = c_change_account(&info);
    if (ret != 0)
    {
        printf("用户名重复，按任意键继续\n");
        while (getchar() != 10)
        {
        }
        return -1;
    }

    printf("修改成功，请重新登录\n");
    while (getchar() != 10)
    {
    }

    close(fd);
    exit(0);
}

////////////////////
int getch(void)
{
    struct termios tm, tm_old;
    int fd = 0, ch;
    if (tcgetattr(fd, &tm_old) < 0)
    { //保存现在的终端设置
        return -1;
    }
    tm = tm_old;
    /* ICANON取反，表示关闭输入行编辑模式，这样我们可以直接read字符，不用等着敲回车.
     * ISIG取反，表示禁止信号，这样Crtl+c Crtl+s 就不会产生信号了，当你发现用户键入
     * Crtl+c后，你可以自定义一个动作
     */
    tm.c_lflag &= ~(ICANON | ISIG);
    if (tcsetattr(fd, TCSANOW, &tm) < 0)
    { //设置上更改之后的设置
        return -1;
    }
    read(fd, &ch, 1);
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
    { //更改设置为最初的样子
        return -1;
    }
    return ch;
}
