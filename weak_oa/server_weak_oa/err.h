#ifndef _ERR_H_
#define _ERR_H_

//错误输出
#define PERR(inet, msg) do{printf("[%s:%s:%d][%s:%d]%s\n", __FILE__, __func__, __LINE__, \
                            inet_ntoa((inet).sin_addr),\
                            htons((inet).sin_port),\
                            msg);}while(0) 
#define ERR(x) do{printf("[%s:%s:%d]%s\n", __FILE__, __func__, __LINE__, x);}while(0) 


#define SHOW_V(x) do{printf("cmd=%d\n",x.cmd);\
                    printf("errcode=%d\n",x.errcode);\
                    printf("isend=%d\n",x.isend);\
                    printf("id=%s\n",x.id);\
                    printf("name=%s\n",x.name);\
                    printf("tel=%s\n",x.tel);\
                    printf("sex=%c\n",x.sex);\
                    printf("birth=%s\n",x.birth);\
                    printf("addr=%s\n",x.addr);\
                    printf("entrytime=%s\n",x.entrytime);\
                    printf("job=%s\n",x.job);\
                    printf("department=%s\n",x.department);\
                    printf("salary=%s\n",x.salary);\
                    printf("rank=%s\n",x.rank);\
                    printf("usr=%s\n",x.usr);\
                    printf("pwd=%s\n",x.pwd);\
                    }while(0)

#define SHOW_P(x) do{printf("cmd=%d\n",x->cmd);\
                    printf("errcode=%d\n",x->errcode);\
                    printf("isend=%d\n",x->isend);\
                    printf("id=%s\n",x->id);\
                    printf("name=%s\n",x->name);\
                    printf("tel=%s\n",x->tel);\
                    printf("sex=%c\n",x->sex);\
                    printf("birth=%s\n",x->birth);\
                    printf("addr=%s\n",x->addr);\
                    printf("entrytime=%s\n",x->entrytime);\
                    printf("job=%s\n",x->job);\
                    printf("department=%s\n",x->department);\
                    printf("salary=%s\n",x->salary);\
                    printf("rank=%s\n",x->rank);\
                    printf("usr=%s\n",x->usr);\
                    printf("pwd=%s\n",x->pwd);\
                    }while(0)


#endif