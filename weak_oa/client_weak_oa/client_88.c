#include "client_init.h"
#include "client_ops.h"

extern int fd;
extern char name[21];
extern char id[11];
extern char department[41];
extern char rank[3];

//----------------2. 88财务界面
int lv88_interface()
{
    int ret;
    char ch = 0;
    while (1)
    {
        printf("\033c");
        printf("***********************************\n");
        printf("\t%s\t%s\t%s*\n", name, id, department);
        printf("***********************************\n");
        printf("\t[1]按姓名查询员工信息\n");
        printf("\t[2]按工号查询员工信息\n");
        printf("\t[3]账号密码修改\n");
        printf("\t[4]修改员工信息\n");
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
            ret = find_by_name();
            break;
        case '2':
            ret = find_by_id();
            break;
        case '3':
            ret = change_account();
            break;
        case '4':
            ret = modify();
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