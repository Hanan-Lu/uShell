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

#define SHELL_PREFIX                ("uShell#")   // Shell ǰ׺
#define TERMINAL_CLEAR_SCREEN       ("\033[2J\033[1H") // Shell Terminal Control Character, Clear Screen

#define SHELL_COMMAND_MAX_LENGTH    (60)            //������󳤶�
#define SHELL_HISTORY_NUM           (10+1)           //��ʷ��¼����, ʵ�ʿ�������ΪSHELL_HISTORY_NUM - 1

typedef enum
{
    FUNCTION_CMD = 0,                               //����
    VARIABLE_INT,                                   //���α���
    VARIABLE_FLOAT,                                 //�����ͱ���
    INVALIDE_CMD                                    //�Ƿ�����
} Cmd_EnumTypedef;

typedef struct
{
    char *pCmdStr;                                  //�����ַ���
    int (*pFunc)(int, char *[]);                    //����ָ��
    void *pVar;                                     //����ָ��
    char *description;                              //��������
    Cmd_EnumTypedef type;                           //��������
} uShellCmd_StructDef;

typedef struct
{
    unsigned char (*pGetc)(char *);                 //��ȡ�ַ�����ָ��, �û�ʵ��
    void (*pPutlen)(char *, unsigned short);        //��ӡָ�����Ⱥ���ָ�룬�û�ʵ��
    void (*pPrintf)(const char *, ...);             // printf����ָ�룬�û�ʵ��
    //˽�б���//
    uShellCmd_StructDef *pCmdList;                  //�����б�ָ��
    char cmdBuffer[SHELL_COMMAND_MAX_LENGTH + 1];   //�����������
    unsigned short cmdBufferUsedLen;                //�ѻ��泤��
    unsigned short terminalCursorPosition;          //�ն˹��λ��
#if(SHELL_HISTORY_NUM > 1)
    char historyArray[SHELL_HISTORY_NUM][SHELL_COMMAND_MAX_LENGTH+1];
    short pHistoryWrite;                            //д������
    short pHistoryRead;                             //��ȡ����
    short hsitoryNum;                               //�Ѵ洢��ʷ��¼����
    char historyFlag;                               //flag, Key Down����ʹ��
#endif
} uShell_InitStructDef;

unsigned char uShellInit(uShell_InitStructDef *_init);
void uShellCallback(void);
uShell_InitStructDef *uShellGetcurrent(void);
int uShellSetcurrent(unsigned char index);
/*uShell ������� ִ�к���*/
int uShellCmdExecute(char *pcmdStr);
unsigned char uShellCmdExplain(char *pcmdString, int *argc, char *argv[]);
int uShellCmdFind(char *pcmdStr);
int uShellSetFloatVarByIndex(int index, char *pVarStr);
int uShellSetIntVarByIndex(int index, char *pVarStr);
/*uShell �Զ���ȫ*/
int uShellAutoComplete(char *str);

/*uShell ������ʷ��¼��غ���*/
unsigned char uShellHistoryAdd(char *str);
unsigned char uShellHistoryReadPrev(char *str);
unsigned char uShellHistoryReadNext(char *str);

/*uShell �ַ�������������Щ�����ɹ��ⲿʹ��*/
unsigned short uShellStrlen(char *str);
unsigned char uShellStrcpy(char *dest, char *src, unsigned short len);
unsigned char uShellStrcmp(char *str1, char *str2, unsigned short str1Len);
unsigned char uShellStrContain(char *str1, char *str2);
unsigned char uShellStrInsertc(char *dest, unsigned short maxlen, unsigned short position, char ch);
unsigned char uShellStrDeletelc(char *dest, unsigned short maxlen, unsigned short position);
unsigned char uShellStrDeleterc(char *dest, unsigned short maxlen, unsigned short position);
unsigned char uShellStrClear(char *dest, unsigned short len);
/*uShell ASCII�ַ���ת���������㺯��*/
int uShell_atoi(char *str, int *var);
int uShell_atof(char *str, float *var);

/*uShell �ڽ������*/
int uShellClearscreen(int argc, char *argv[]);
int uShellListAllcmd(int argc, char *argv[]);
int uShellListAllVar(int argc, char *argv[]);
int uShellListAll(int argc, char *argv[]);
int uShellEcho(int argc, char *argv[]);
int uShellTestatoi(int argc, char *argv[]);
int uShellTestatof(int argc, char *argv[]);
int uShellTestStrcontain(int argc, char *argv[]);
#endif
