#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct Stack
{
    int a[100000];
    int topp;
}*LStack;

void push(LStack s,int x)//将 加x入栈，保证 x为 int 型整数
{
    s->a[s->topp]=x;
    s->topp++;
}

int pop(LStack s)//输出栈顶，并让栈顶出栈
{
    if(s->topp==0) 
        return -1;
    else return s->a[--s->topp];
}

int top(LStack s)//输出栈顶，栈顶不出栈
{
    if(s->topp==0)  
        return -1;
    else return s->a[s->topp-1];
}

void main()
{
    LStack s=(LStack)malloc(sizeof(struct Stack));//利用结构体在堆内存空间里创建一个数据结构栈
    s->topp=0;
    int n=0;
    scanf("%d",&n);
    while(n--)
    {
        char *str;//
        scanf("%s",str);
        //int strcmp(const char *s1,const char *s2);比较两个字符串并根据比较结果返回整数
        //自左向右逐个按照ASCII码值进行比较，直到出现不同的字符或遇’\0’为止。
        // 如果返回值 < 0，则表示 s1 小于 s2
        // 如果返回值 > 0，则表示 s1 大于 s2
        // 如果返回值 = 0，则表示 s1 等于 s2
        if(!strcmp(str,"push"))
        {
            int num=0;
            scanf("%d",&num);
            push(s,num);
            continue;//continue作用为结束本次循环
            //break可以跳出“循环体”，还可以跳出switch
        }
        if(!strcmp(str,"pop"))
        {
            int num1=pop(s);
            if(num1==-1)
                printf("error\n");
            else printf("%d\n",num1);
            continue;
        }
        if(!strcmp(str,"top"))
        {
            int num2=top(s);
            if(num2==-1)
                printf("error\n");
            else printf("%d\n",num2);
            continue;
        }
    }
    system("pause");
}