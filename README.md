# 简介

        uShell是一个极简的Shell，仅支持函数调用、变量赋值功能。变量支持整形变量和浮点型变量。函数仅支持标准main函数形式。

        uShell需要大约14kB的ROM及大约1kB的RAM才可正常运行。通过禁用历史命令记录（默认支持10条，可通过修改宏定义进行增加/减少/禁用，详见uShell.h）可以缩小RAM的占用。

        uShell支持的Shell功能有：

* 自动补全

* 历史记录

* 仅支持CR+LF换行

# 用法

        uShell需要用户实现三个函数，分别为读取字符函数、打印指定长度字符函数、printf函数。函数指针如下：

```c
unsigned char (*pGetc)(char *);          //读取一个字符函数指针 返回0为成功读取
void (*pPutlen)(char *, unsigned short); //打印指定长度字符函数指针
void (*pPrintf)(const char *, ...);      //printf函数指
```

### 需要用户实现的函数样例

以下为函数样例，样例仅供展示，实际使用请依据自己平台进行修改：

```c
/*样例*/
/*读取字符串函数*/
unsigned char USART2_GetC(char *ch)
{
    if(LL_USART_IsActiveFlag_RXNE(USART2) == SET){
        *ch = USART2->DR;
        return 0;
    }
    return 1;
}
/*打印指定长度字符*/
void USART2_putLen(char *str, unsigned short len)
{
    for(unsigned short i=0; i < len; ++i){
        while(LL_USART_IsActiveFlag_TXE(USART2) == RESET){}
        USART2->DR = str[i];
    }
}
/*printf 实现，需要包含<stdarg.h>*/
#include <stdarg.h>
void USART2_printf(const char *fmt, ...)
{
#define BUFSIZE 256
    char myprintf_buf[BUFSIZE];
    va_list args;
    int n;

    va_start(args, fmt);
    n = vsnprintf(myprintf_buf, BUFSIZE, fmt, args);
    va_end(args);
    USART2_putLen(myprintf_buf, n);
}
```

### 具体使用

```c
//... include file
#include "uShell.h"
//...
int blkSpace = 0;
//定义 一个“uShellCmd_StructDef ” 对象，存储命令
//格式为 {命令字符串, 目标函数指针, 目标变量指针, 命令提示字符串, 命令类型}
//其中 当命令为函数类型时, 目标变量指针为NULL
//    当命令为变量类型时, 目标函数指针为NULL

//目前仅支持main原型函数: int (*pFunc)(int, char *[]);
uShellCmd_StructDef shellCmdList[] = {
    //函数命令
    {"clear"    , uShellClearscreen     , NULL, "Clear Screen"                  , FUNCTION_CMD},
    {"help"     , uShellListAll         , NULL, "List All Command and Variable" , FUNCTION_CMD},
    {"listVar"  , uShellListAllVar      , NULL, "List All Variable"             , FUNCTION_CMD},
    {"listCmd"  , uShellListAllcmd      , NULL, "List All Command"              , FUNCTION_CMD},
    {"listAll"  , uShellListAll         , NULL, "List All Command and Variable" , FUNCTION_CMD},
    {"echo"     , uShellEcho            , NULL, "Print String"                  , FUNCTION_CMD},
    {"atoi"     , uShellTestatoi        , NULL, "Test ASCII to Int Func"        , FUNCTION_CMD},
    {"atof"     , uShellTestatof        , NULL, "Test ASCII to float Func"      , FUNCTION_CMD},
    {"strcontain", uShellTestStrcontain , NULL, "Whether String 1 contains String 1"      , FUNCTION_CMD},
    //变量命令
    {"blkSpace"         , NULL  , &blkSpace   , "LED Toggle Space"              , VARIABLE_INT},
    {0x00,NULL, NULL, NULL, INVALIDE_CMD} //不要删除，程序使用检测末尾
};

//....

int main(void)
{
    //... 用户代码
    // 定义一个uShell_InitStructDef 对象
    uShell_InitStructDef uShellInitStruct;

    //... 外设初始化

    //将上面的函数指针及命令列表赋值
    uShellInitStruct.pGetc = USART2_GetC;
    uShellInitStruct.pPrintf = USART2_printf;
    uShellInitStruct.pPutlen = USART2_putlen;
    uShellInitStruct.pCmdList = shellCmdList;
    //Shell初始化
    uShellInit(&uShellInitStruct);
    while (1)
    {   //循环调用uShellCallback()函数
        uShellCallback();
        //用户其他代码
    }
}
```

## 注意事项

因本人水平有限，程序之中一定存在bug及不合理的地方，欢迎大家指正。
