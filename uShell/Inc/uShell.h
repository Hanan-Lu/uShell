#ifndef _USHELL_H__
#define _USHELL_H__

/*********************************************************************************/
#define KEY_TAB                     (0x09)
#define KEY_ENTER                   (0x0D)
#define COMBKEY_CTRL_C              (0x03)
#define KEY_BACKSPACE               (0x08)
#define KEY_UP                      (0x1B5B41)
#define KEY_DOWN                    (0x1B5B42)
#define KEY_RIGHT                   (0x1B5B43)
#define KEY_LEFT                    (0x1B5B44)
#define KEY_END                     (0x1B5B46)
#define KEY_HOME                    (0x1B5B48)
#define KEY_INSERT                  (0x1B5B327E)
#define KEY_DELETE                  (0x1B5B337E)
#define KEY_PAGEUP                  (0x1B5B357E)
#define KEY_PAGEDOWN                (0x1B5B367E)

#define SHELL_PREFIX                ("uShell#")   // Shell 前缀
#define TERMINAL_CLEAR_SCREEN       ("\033[2J\033[1H") // Shell Terminal Control Character, Clear Screen

#define SHELL_COMMAND_MAX_LENGTH    (60)            //命令最大长度
#define SHELL_HISTORY_NUM           (10+1)           //历史记录条数, 实际可用数量为SHELL_HISTORY_NUM - 1

typedef enum
{
    FUNCTION_CMD = 0,                               //函数
    VARIABLE_INT,                                   //整形变量
    VARIABLE_FLOAT,                                 //浮点型变量
    INVALIDE_CMD                                    //非法类型
} Cmd_EnumTypedef;

typedef struct
{
    char *pCmdStr;                                  //命令字符串
    int (*pFunc)(int, char *[]);                    //函数指针
    void *pVar;                                     //变量指针
    char *description;                              //命令描述
    Cmd_EnumTypedef type;                           //命令类型
} uShellCmd_StructDef;

typedef struct
{
    unsigned char (*pGetc)(char *);                 //读取字符函数指针, 用户实现
    void (*pPutlen)(char *, unsigned short);        //打印指定长度函数指针，用户实现
    void (*pPrintf)(const char *, ...);             // printf函数指针，用户实现
    //私有变量//
    uShellCmd_StructDef *pCmdList;                  //命令列表指针
    char cmdBuffer[SHELL_COMMAND_MAX_LENGTH + 1];   //输入命令缓冲区
    unsigned short cmdBufferUsedLen;                //已缓存长度
    unsigned short terminalCursorPosition;          //终端光标位置
#if(SHELL_HISTORY_NUM > 1)
    char historyArray[SHELL_HISTORY_NUM][SHELL_COMMAND_MAX_LENGTH+1];
    short pHistoryWrite;                            //写入索引
    short pHistoryRead;                             //读取索引
    short hsitoryNum;                               //已存储历史记录条数
    char historyFlag;                               //flag, Key Down处理使用
#endif
} uShell_InitStructDef;

unsigned char uShellInit(uShell_InitStructDef *_init);
void uShellCallback(void);
uShell_InitStructDef *uShellGetcurrent(void);
int uShellSetcurrent(unsigned char index);
/*uShell 命令解析 执行函数*/
int uShellCmdExecute(char *pcmdStr);
unsigned char uShellCmdExplain(char *pcmdString, int *argc, char *argv[]);
int uShellCmdFind(char *pcmdStr);
int uShellSetFloatVarByIndex(int index, char *pVarStr);
int uShellSetIntVarByIndex(int index, char *pVarStr);
/*uShell 自动补全*/
int uShellAutoComplete(char *str);

/*uShell 命令历史记录相关函数*/
unsigned char uShellHistoryAdd(char *str);
unsigned char uShellHistoryReadPrev(char *str);
unsigned char uShellHistoryReadNext(char *str);

/*uShell 字符串处理函数，这些函数可供外部使用*/
unsigned short uShellStrlen(char *str);
unsigned char uShellStrcpy(char *dest, char *src, unsigned short len);
unsigned char uShellStrcmp(char *str1, char *str2, unsigned short str1Len);
unsigned char uShellStrContain(char *str1, char *str2);
unsigned char uShellStrInsertc(char *dest, unsigned short maxlen, unsigned short position, char ch);
unsigned char uShellStrDeletelc(char *dest, unsigned short maxlen, unsigned short position);
unsigned char uShellStrDeleterc(char *dest, unsigned short maxlen, unsigned short position);
unsigned char uShellStrClear(char *dest, unsigned short len);
/*uShell ASCII字符串转整数及浮点函数*/
int uShell_atoi(char *str, int *var);
int uShell_atof(char *str, float *var);

/*uShell 内建命令函数*/
int uShellClearscreen(int argc, char *argv[]);
int uShellListAllcmd(int argc, char *argv[]);
int uShellListAllVar(int argc, char *argv[]);
int uShellListAll(int argc, char *argv[]);
int uShellEcho(int argc, char *argv[]);
int uShellTestatoi(int argc, char *argv[]);
int uShellTestatof(int argc, char *argv[]);
int uShellTestStrcontain(int argc, char *argv[]);
#endif
