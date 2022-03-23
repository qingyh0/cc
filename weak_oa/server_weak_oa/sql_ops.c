#include "sql_ops.h"

extern sqlite3 *db;

int sql_init()
{
    char sql[512]={0};
    char *msg = NULL;
    int ret;
    char **result=NULL;
    int row, col;
    
    //创建数据库
    ret = sqlite3_open("./oa.db", &db);
    if(ret != SQLITE_OK)
    {
        ERR("open oa.db error");
        return -1;
    }

    //创建信息表
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "create table if not exists info(id char primary key, \
                name char, tel char, sex char, birth char, addr char, entrytime char, \
                job char, department char, salary char, rank char, usr char, pwd char);");
    printf("%s\n", sql);
    ret = sqlite3_exec(db, sql, NULL, NULL, &msg);
    if(ret != SQLITE_OK)
    {
        ERR(msg);
        return -1;
    }

    //查表，如果表内没有数据，则新建admin用户
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "select count(id) from info;");
    printf("%s\n", sql);
    ret = sqlite3_get_table(db, sql, &result, &row, &col, &msg);
    if(ret != SQLITE_OK)
    {
        ERR(msg);
        return -1;
    }

    // printf("row=%d, col=%d\n", row, col);
    // for(int i=0; i<(row+1)*col; i++)
    //     printf("%s\n", result[i]);

    if(memcmp(result[1], "0", 1)!=0)    //表内有数据
    {
        return 0;
    }

    sqlite3_free_table(result);


    //插入一条admin用户记录
    memset(sql, 0, sizeof(sql));
    sprintf(sql, "insert into info values(\"0\", \"admin\", \"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"99\",\"admin\",\"admin\");");
    printf("%s\n", sql);
    ret = sqlite3_exec(db, sql, NULL, NULL, &msg);
    if(ret != SQLITE_OK)
    {
        ERR(msg);
        return -1;
    }

    return 0;
}