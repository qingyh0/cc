#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>

//#define NU 5

char **initZZ(int n, char c);
void printZZ();
void changeData(char **st, char **en);
int hannuo(int n, char **st, char **te, char **en);
void freeptr();

char **ST;
char **TE;
char **EN;
int NU;

int main(int argc, char const *argv[])
{
    printf("输入要初始化的层数:");
    scanf("%d",&NU);
    getchar();

    //..1.初始化3根柱子
    char **st = initZZ(NU, '*');
    char **te = initZZ(NU, ' ');
    char **en = initZZ(NU, ' ');
    ST=st;
    TE=te;
    EN=en;
    //..2.打印三根柱子
    printZZ(st, te, en);
    ////..3.移动一次盘子
    //changeData(st, te, en);

    int cnt = hannuo(NU, st, te, en);
    printf("一共执行%d次\n",cnt);

    freeptr();
    getchar();

    return 0;
}


/*..初始化1个柱子
返回值: 一个柱子的二维数组
n:柱子的层数
c:盘子的标识字符
*/
char **initZZ(int n, char c)
{
    char ** st = (char**)malloc(n*sizeof(char*));
    for(int i=0; i<n; i++)
    {
        st[i]=(char*)malloc((n*2-1)*sizeof(char));
    }

    for(int i=0; i<n; i++)
    {
        for(int j=0; j< n*2 -1; j++)
        {
            if(j>=n-1-i && j<= n-1+i)
            {
                st[i][j]= c ;
                //printf("%c",st[i][j]);
            }
            else
            {
                st[i][j]=' ';
                //printf("%c",st[i][j]);
            }
        }
        //printf("\n");
    }
    return st;
}

/*打印三个柱子
st:起始柱子
te:缓存柱子
en:目的柱子
flag: 123=st te en ; 132= st en te ; 213
*/
void printZZ()
{
    system("cls");//windows清屏
    for(int i=0; i<NU; i++)
    {
        for(int j=0; j<NU*2-1; j++)
        {
            printf("%c",*(*(ST+i)+j));
            //printf("%c",sr[i][j]);
        }
        printf("    |    ");
        for(int j=0; j<NU*2-1; j++)
        {
            printf("%c",*(*(TE+i)+j));
            //printf("%c",sr[i][j]);
        }
        printf("    |    ");
        for(int j=0; j<NU*2-1; j++)
        {
            printf("%c",*(*(EN+i)+j));
            //printf("%c",sr[i][j]);
        }
        printf("\n");
    }
    Sleep(1000);
}

/*交换数组：将st数组的最上方一层移动到en数组最上方一层
st:起始柱子
en:目的柱子
*/
void changeData(char **st, char **en)
{
    int stFlag=0, enFlag=0;
    for(int i=0; i<NU; i++)
    {
        if(st[i][NU-1] != ' ')
        {
            stFlag=i;
            break;
        }
    }
    for(int i=NU-1; i>=0; i--)
    {
        if(en[i][NU-1] == ' ')
        {
            enFlag=i;
            break;
        }
    }

    for(int i=0; i<2*NU-1; i++)
    {
        en[enFlag][i]=st[stFlag][i];
        st[stFlag][i]=' ';
    }
}

/*递归移动汉诺塔

*/
int hannuo(int n, char **st, char **te, char **en)
{
    int res=0;
    if(n ==1 )
    {
        changeData(st, en);
        printZZ();//按顺序真实打印
        res=1;
    }
    else
    {
        res = res + hannuo(n-1, st, en, te);
        //changeData(st, en);
        //printZZ();
        res = res + hannuo(1, st, te, en);
        res = res + hannuo(n-1, te, st, en);
    }
    return res;
}

//释放malloc
void freeptr()
{
    for(int i=0; i<NU; i++)
    {
        free(ST[i]);
    }
}
