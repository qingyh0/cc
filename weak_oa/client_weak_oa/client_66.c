#include "client_init.h"
#include "client_ops.h"

extern int fd;
extern char name[21];
extern char id[11];
extern char department[41];
extern char rank[3];


//-------------------1.1员工管理界面
int lv66_m_staff_interface();

//----------------1. 66人事主界面
int lv66_interface()
{
    int ret;
    char ch = 0;
    while (1)
    {
        printf("\033c");
        printf("***********************************\n");
        printf("\t*%s\t%s\t%s*\n", name, id, department);
        printf("***********************************\n");
        printf("\t[1]员工管理\n");
        printf("\t[2]账号密码修改\n");
        // printf("\t[3]考勤记录\n");
        // printf("\t[4]审批\n");
        // printf("\t[5]我的考勤\n");
        printf("***********************************\n");
        printf("\t[数字键选择，q退出]\n");
        printf("***********************************\n");
        printf("<<");

        ch = getchar();
        while (getchar() != 10)
        {
        }
        switch (ch)
        {
        case '1':
            ret = lv66_m_staff_interface();
            break;
        case '2':
            ret = change_account();
            break;

        case 'q':
            return -1; //返回上一级
            break;
        default:
            break;
        }
    }

    return 0;
}


//-------------------1.1员工管理界面
int lv66_m_staff_interface()
{
    int ret;
    char ch = 0;
    while (1)
    {
        printf("\033c");
        printf("***********************************\n");
        printf("**************员工管理**************\n");
        printf("\t[1]按姓名查询员工信息\n");
        printf("\t[2]按工号查询员工信息\n");
        printf("\t[3]员工入职\n");
        printf("\t[4]员工离职\n");
        printf("\t[5]员工信息修改\n");
        printf("***********************************\n");
        printf("\t[数字键选择，q退出]\n");
        printf("***********************************\n");
        printf("<<");

        ch = getchar();
        while (getchar() != 10)
        {
        }

        switch (ch)
        {
        case '1':
            /* 按姓名查询员工信息 */
            ret = find_by_name();
            break;
        case '2':
            /* 按工号查询员工信息 */
            ret = find_by_id();
            break;
        case '3':
            /* 员工入职 */
            ret = onboarding();
            break;
        case '4':
            /* 员工离职 */
            ret = resign();
            break;
        case '5':
            /* 员工信息修改 */
            ret = modify();
            break;
        case 'q':
            /* 退出 */
            return -1; //返回上一级
            break;
        default:
            break;
        }
    }
    return 0;
}