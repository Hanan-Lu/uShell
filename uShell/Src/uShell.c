/**
 * @file            uShell.c
 * @author          1451148480@qq.com
 * @brief           uShell实现
 * @version         0.1
 * @date            2022-12-04
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../Inc/uShell.h"

#define NULL (0)

uShell_InitStructDef uShellList[1] = {NULL};
unsigned char currentShellIndex = 0;
unsigned char totalShellNum = 1;

/**
 * @brief                       uShell 初始化
 *
 * @param _init                 uShell_InitStructDef  指针
 * @return unsigned char        0x00: Success; Other: Fail
 */
unsigned char uShellInit(uShell_InitStructDef *_init)
{
    char logo[265] =
        {"        _____ _    _ ______ _      _      \r\n"
         "       / ____| |  | |  ____| |    | |     \r\n"
         " _   _| (___ | |__| | |__  | |    | |     \r\n"
         "| | | |\\___ \\|  __  |  __| | |    | |     \r\n"
         "| |_| |____) | |  | | |____| |____| |____ \r\n"
         " \\__,_|_____/|_|  |_|______|______|______|\r\n\0"};

    if (_init == NULL)
        return 0x01;

    uShellGetcurrent()->pCmdList = _init->pCmdList;
    uShellGetcurrent()->pGetc = _init->pGetc;
    uShellGetcurrent()->pPrintf = _init->pPrintf;
    uShellGetcurrent()->pPutlen = _init->pPutlen;
    uShellGetcurrent()->cmdBufferUsedLen = 0;
    uShellGetcurrent()->terminalCursorPosition = 0;
#if(SHELL_HISTORY_NUM > 1)
    uShellGetcurrent()->historyFlag = 0;
    uShellGetcurrent()->hsitoryNum = 0;
    uShellGetcurrent()->pHistoryRead = 0;
    uShellGetcurrent()->pHistoryWrite = 0;
#endif
    uShellGetcurrent()->pPrintf(TERMINAL_CLEAR_SCREEN);
    uShellGetcurrent()->pPrintf("\r\n");
    uShellGetcurrent()->pPutlen(logo, 264);    //打印Logo
    uShellGetcurrent()->pPrintf("\r\n%s ", SHELL_PREFIX); //打印提示符
    uShellStrClear(uShellGetcurrent()->cmdBuffer, SHELL_COMMAND_MAX_LENGTH + 1);
    return 0x00;
}
/**
 * @brief                           获取当前Shell对象，外部调用
 *
 * @return uShell_InitStructDef*    当前Shell对象指针
 */
uShell_InitStructDef *uShellGetcurrent(void)
{
    return &uShellList[currentShellIndex];
}
/**
 * @brief                           设置当前Shell对象
 * 
 * @param index                     目标Shell索引
 * @return int                      -1:Fail, 0:Success
 */
int uShellSetcurrent(unsigned char index){
    if(index >= totalShellNum)
        return -1;
    currentShellIndex = index;
    return 0;
}
/**
 * @brief                           Shell回调函数，此函数读取串口数据并进行解析
 * @todo                            待整理，缩短函数体
 */
void uShellCallback(void)
{
    char specKey[4];       //特殊按键缓冲区
    unsigned char len = 0; //特殊按键解析

    char *pCmdBuffer = uShellGetcurrent()->cmdBuffer;                              //字符串数据缓冲区
    unsigned short *pCmdLen = &uShellGetcurrent()->cmdBufferUsedLen;               //已存储字符串长度
    unsigned short *pCursorPosition = &uShellGetcurrent()->terminalCursorPosition; //光标位置

    if (uShellGetcurrent()->pGetc(&specKey[0])) //读取一个字符
        return;
    if (specKey[0] >= ' ' && specKey[0] <= '~') //读取到的为可打印字符
    {
        if (*pCmdLen >= SHELL_COMMAND_MAX_LENGTH) //数据长度超出
        {
            uShellGetcurrent()->pPrintf("\r\nCommand is too long!\r\n");
            uShellGetcurrent()->pPrintf("\r\n%s ", SHELL_PREFIX);
            uShellGetcurrent()->pPutlen((char *)&pCmdBuffer[0], SHELL_COMMAND_MAX_LENGTH);
        }
        else
        {
            //向缓冲区插入接收到的字符
            uShellStrInsertc(pCmdBuffer, SHELL_COMMAND_MAX_LENGTH, *pCursorPosition, specKey[0]);
            (*pCmdLen)++;
            (*pCursorPosition)++;
            //打印字符
            if (*pCmdLen == *pCursorPosition)
                uShellGetcurrent()->pPutlen((char *)&pCmdBuffer[*pCursorPosition - 1], 1);
            else
            {
                uShellGetcurrent()->pPutlen(&pCmdBuffer[*pCursorPosition - 1],
                                            *pCmdLen - *pCursorPosition + 1);
                uShellGetcurrent()->pPrintf("\033[%dD", *pCmdLen - *pCursorPosition); //移动光标到cursorPosition
            }
        }
    #if(SHELL_HISTORY_NUM > 1)
        uShellGetcurrent()->historyFlag = 0;
    #endif
    }
    else
    { //不可打印字符
        switch (specKey[0])
        {
        case KEY_BACKSPACE: //删除键
            if (*pCursorPosition > 0)
            {
                (*pCmdLen)--;
                (*pCursorPosition)--;
                if (*pCmdLen == *pCursorPosition) //光标处于末尾
                {
                    uShellGetcurrent()->pPutlen("\b \b", 3);
                    pCmdBuffer[(*pCmdLen)] = '\0'; //清空
                }
                else
                {
                    uShellStrDeletelc(pCmdBuffer, SHELL_COMMAND_MAX_LENGTH, *pCursorPosition + 1); //删除光标左侧字符
                    uShellGetcurrent()->pPrintf("\b\033[K");                                       //退格+清空光标到行尾并
                    uShellGetcurrent()->pPutlen(&pCmdBuffer[*pCursorPosition],
                                                *pCmdLen - *pCursorPosition);             //打印光标所在处到末尾
                    uShellGetcurrent()->pPrintf("\033[%dD", *pCmdLen - *pCursorPosition); //移动光标到cursorPosition
                }
            #if(SHELL_HISTORY_NUM > 1)
                uShellGetcurrent()->historyFlag = 0;
            #endif
            }
            break;
        case KEY_ENTER: //回车
            if (*pCmdLen != 0)
            {
            #if(SHELL_HISTORY_NUM > 1)
                uShellHistoryAdd(NULL);
            #endif
                if (uShellCmdExecute(pCmdBuffer) == -1)
                {
                    uShellGetcurrent()->pPrintf("\r\nCommand \"%s\" not found!", pCmdBuffer);
                }
            }
            
            uShellGetcurrent()->pPrintf("\r\n%s ", SHELL_PREFIX);
            uShellStrClear(pCmdBuffer, *pCmdLen + 1);
            *pCmdLen = 0;
            *pCursorPosition = 0;
            break;
        case KEY_TAB:
            uShellAutoComplete(NULL);
            break;
        case COMBKEY_CTRL_C:
            uShellGetcurrent()->pPrintf("^C");
            uShellGetcurrent()->pPrintf("\r\n%s ", SHELL_PREFIX);
            uShellStrClear(pCmdBuffer, *pCmdLen + 1);
            *pCmdLen = 0;
            *pCursorPosition = 0;
            break;
        case 0x1B:
            while (!uShellGetcurrent()->pGetc(&specKey[++len]))
                ;
            if (specKey[1] == 0x5B)
            {
                switch (specKey[2])
                {
                case 0x41: // UP
                #if(SHELL_HISTORY_NUM > 1)
                    if(uShellHistoryReadPrev(NULL))
                    {
                        uShellGetcurrent()->pPrintf("\r\033[K");
                        uShellGetcurrent()->pPrintf("%s ", SHELL_PREFIX);
                        uShellGetcurrent()->pPrintf(uShellGetcurrent()->cmdBuffer);
                    }
                #endif
                    break;
                case 0x42: // DOWN
                #if(SHELL_HISTORY_NUM > 1)
                    uShellHistoryReadNext(NULL);
                #endif
                    break;
                case 0x43: // RIGHT
                    if (*pCursorPosition < *pCmdLen)
                    {
                        uShellGetcurrent()->pPrintf("\033[1C");
                        (*pCursorPosition)++;
                    }
                    break;
                case 0x44: // LEFT
                    if (*pCursorPosition != 0)
                    {
                        uShellGetcurrent()->pPrintf("\033[1D");
                        (*pCursorPosition)--;
                    }
                    break;
                case 0x46: // END
                    if (*pCursorPosition != *pCmdLen)
                    {
                        uShellGetcurrent()->pPrintf("\033[%dC", *pCmdLen - *pCursorPosition);
                        *pCursorPosition = *pCmdLen;
                    }
                    break;
                case 0x48: // HOME
                    if (*pCursorPosition != 0)
                    {
                        uShellGetcurrent()->pPrintf("\033[%dD", *pCursorPosition);
                        *pCursorPosition = 0;
                    }
                    break;
                case 0x32: // INSERT
                    //ToDo
                    break;
                case 0x33: // DELETE
                    if ((*pCursorPosition < *pCmdLen) && (*pCmdLen > 0))
                    {
                        uShellStrDeleterc(pCmdBuffer, SHELL_COMMAND_MAX_LENGTH, *pCursorPosition);
                        (*pCmdLen)--;
                        uShellGetcurrent()->pPrintf("\033[K");
                        uShellGetcurrent()->pPutlen(&pCmdBuffer[*pCursorPosition], *pCmdLen - *pCursorPosition);
                        if (*pCmdLen != *pCursorPosition)
                            uShellGetcurrent()->pPrintf("\033[%dD", *pCmdLen - *pCursorPosition);
                    }
                    break;
                case 0x35: // PAGE UP
                    //uShellGetcurrent()->pPrintf("\r\nKey PAGE-UP!\r\n");
                    break;
                case 0x36: // PAGE DOWN
                    //uShellGetcurrent()->pPrintf("\r\nKey PAGE-DOWN!\r\n");
                    break;
                }
            }
            break;
        }
    }
}
/**
 * @brief                   shell 字符串解析
 *
 * @param pcmdString        待解析字符串
 * @param argc              已解析到的参数数量，包含命令
 *                          例如, 输入"echo "hello world!"", 则argc=2;
 * @param argv              解析出来的数据，argv[0] 为命令
 * @return unsigned char    0:解析失败, 其他: 解析到的参数数量
 */
unsigned char uShellCmdExplain(char *pcmdString, int *argc, char *argv[])
{
    unsigned short cmdLen = uShellStrlen(pcmdString);
    unsigned short i = 0, j = 0, k = 0;
    char state = 0;
    if (cmdLen == 0) //传入字符串为空
        return 0;
    /*提取命令*/
    for (i = 0; i < cmdLen; i++)
    {
        switch(state){
            case 0:                         //命令状态
                if((pcmdString[i] == ' ') || (pcmdString[i] == '='))
                {
                    state = 1;              //转换为数据状态                  
                    j++;
                    k = 0;
                    continue;
                }
                break;
            case 1:                         //数据状态
                if(pcmdString[i] == '\"')   
                {
                    state = 2;              //转换为字符串状态
                    continue;
                }
                else if(pcmdString[i] == ' ')
                {                           //下一个数据
                    j++;
                    k = 0;
                    continue;
                }
                break;
            case 2:         //字符串状态
                if(pcmdString[i] == '\"')
                {
                    state = 1;              //转换为数据状态
                    continue;
                }
                break;
        }
        *((char *)argv + j * SHELL_COMMAND_MAX_LENGTH + k) = pcmdString[i];
        k++;
    }
    *argc = j + 1;
    return *argc;
}
/**
 * @brief                           查找Shell命令，此函数返回索引最小的索引
 *
 * @param pcmdStr                   命令字符串指针
 * @return unsigned char            -1: 未找到指令, Other：已找到指令索引
 */
int uShellCmdFind(char *pcmdStr)
{
    unsigned short i;
    unsigned short len = uShellStrlen(pcmdStr);
    for (i = 0; uShellGetcurrent()->pCmdList[i].type != INVALIDE_CMD; i++)
    {
        if (uShellStrcmp(pcmdStr, uShellGetcurrent()->pCmdList[i].pCmdStr, len))
        {
            return i;
        }
    }
    return -1;
}
/**
 * @brief           执行指令
 *
 * @param pcmdStr   命令字符串，此字符串为终端输入字符串
 * @return int      执行结果，0:Success, Other: Fail
 */
int uShellCmdExecute(char *pcmdStr)
{
    int argc = 0;
    char argv[8][SHELL_COMMAND_MAX_LENGTH] = {0x00};
    int index = 0;
    if (uShellCmdExplain(pcmdStr, &argc, (char **)argv) == 0) //字符串解析，拆分出命令、数据
    {
        return 1; //解析失败
    }
    index = uShellCmdFind(argv[0]); //查找命令
    if (index != -1)                //查找成功
    {
        switch (uShellGetcurrent()->pCmdList[index].type)
        {
            case FUNCTION_CMD:
                uShellGetcurrent()->pCmdList[index].pFunc(argc, (char **)argv); //传递并执行函数
                break;
            case VARIABLE_INT:
                uShellSetIntVarByIndex(index, argv[1]);
                break;
            case VARIABLE_FLOAT:
                uShellSetFloatVarByIndex(index, argv[1]);
                break;
            case INVALIDE_CMD:
            default:
                break;
        }
        return 0;
    }
    return -1;
}

/**
 * @brief               根据索引设定浮点型变量值
 * 
 * @param index         目标索引
 * @param pVarStr       变量值字符串
 * @return int          0:设置成功, -1:设置失败
 */
int uShellSetFloatVarByIndex(int index, char *pVarStr)
{
    float val;
    if(index < 0)                           //非法索引
        return -1;
    if(pVarStr == NULL){
        uShellGetcurrent()->pPrintf("\r\n%s = %.2f, 0x%x", uShellGetcurrent()->pCmdList[index].pCmdStr, \
                                                           *(int *)uShellGetcurrent()->pCmdList[index].pVar, \
                                                           *(int *)uShellGetcurrent()->pCmdList[index].pVar);
        return -1;
    }
    if (uShell_atof(pVarStr, &val) == 0)    //字符串转浮点
    {
        *(float *)uShellGetcurrent()->pCmdList[index].pVar = val;
        return 0;
    }
    else{                                   //字符串转浮点失败
        return -1;
    }
}

/**
 * @brief               根据索引设定整数型变量值
 * 
 * @param index         目标索引
 * @param pVarStr       变量值字符串
 * @return int          0:设置成功, -1:设置失败
 */
int uShellSetIntVarByIndex(int index, char *pVarStr)
{
    int val;
    int retval = -1;
    if(index < 0)
        return -1;
    if(pVarStr == NULL){
        return -1;
    }
    if (uShell_atoi(pVarStr, &val) == 0)    //字符串转整形
    {
        *(int *)uShellGetcurrent()->pCmdList[index].pVar = val;
        retval = 0;
    }

    uShellGetcurrent()->pPrintf("\r\n%s = %d, 0x%x", uShellGetcurrent()->pCmdList[index].pCmdStr, \
                                        *(int *)uShellGetcurrent()->pCmdList[index].pVar, \
                                        *(int *)uShellGetcurrent()->pCmdList[index].pVar);

    return retval;
}
#if(SHELL_HISTORY_NUM > 1)
/**
 * @brief                   添加一条历史指令到历史指令buffer中
 *                          历史指令为一个LIFO环形缓冲区, 并且总是覆盖掉最旧的数据
 * 
 * @param str               保留 未使用
 * @return unsigned char    保留 未使用
 */
unsigned char uShellHistoryAdd(char *str)
{
    (void)(str);

    char *pDest = uShellGetcurrent()->historyArray[uShellGetcurrent()->pHistoryWrite];      //待写入缓冲区地址
    char *pSrc = uShellGetcurrent()->cmdBuffer;                                             //待添加指令字符串
    uShellStrcpy(pDest, pSrc, uShellGetcurrent()->cmdBufferUsedLen);                        //copy当前指令到history缓冲区
    uShellGetcurrent()->pHistoryWrite++;                                                    //指向下一个缓冲区

    if(uShellGetcurrent()->hsitoryNum  < SHELL_HISTORY_NUM)                                 //缓冲区未满
    {
        uShellGetcurrent()->hsitoryNum++;                                                   
    }
    if(uShellGetcurrent()->pHistoryWrite >= SHELL_HISTORY_NUM)                              //缓冲区已满, 覆盖写入
    {
        uShellGetcurrent()->pHistoryWrite = 0;
    }
    uShellGetcurrent()->pHistoryRead = uShellGetcurrent()->pHistoryWrite - 1;               //先入后出
    return 1;
}
/**
 * @brief                   返回当前指令的上一条指令
 * 
 * @param str               保留，未使用
 * @return unsigned char    0:失败
 */
unsigned char uShellHistoryReadPrev(char *str)
{
    (void)(str);
    if(uShellGetcurrent()->hsitoryNum == 0)
        return 0;                                                               //历史记录为空, 直接退出
    if(uShellGetcurrent()->pHistoryRead < 0){                                   //缓冲区最新数据索引溢出
        if(uShellGetcurrent()->hsitoryNum < SHELL_HISTORY_NUM){             
            return 0;                                                           //缓冲区已读空, 退出
        }else{
            uShellGetcurrent()->pHistoryRead = SHELL_HISTORY_NUM - 1;           //环形
        }
    }
    if(uShellGetcurrent()->pHistoryRead == uShellGetcurrent()->pHistoryWrite){
        return 0;                                                               //缓冲区已读空 ToDo:无法判断是满的还是空的。缓冲区可用数量为SHELL_HISTORY_NUM - 1
    }
    /*读出历史命令到命令缓冲区*/
    uShellGetcurrent()->cmdBufferUsedLen = uShellStrlen(uShellGetcurrent()->historyArray[uShellGetcurrent()->pHistoryRead]);
    uShellStrcpy(uShellGetcurrent()->cmdBuffer, \
                 uShellGetcurrent()->historyArray[uShellGetcurrent()->pHistoryRead], \
                 uShellGetcurrent()->cmdBufferUsedLen);
    /*设置光标索引*/
    uShellGetcurrent()->terminalCursorPosition = uShellGetcurrent()->cmdBufferUsedLen;
    uShellGetcurrent()->pHistoryRead--;
    /*key down use*/
    uShellGetcurrent()->historyFlag = 1;
    return 1;
}
/**
 * @brief                   返回当前历史指令的下一条指令,当历史指令为空或当前处于输入模式时
 *                          不做任何操作
 * 
 * @param str               保留 未使用
 * @return unsigned char    保留 未使用
 */
unsigned char uShellHistoryReadNext(char *str)
{   
    (void)(str);
    if(uShellGetcurrent()->historyFlag ==0)
        return 0;
    if(uShellGetcurrent()->hsitoryNum == 0)                     
        return 0;                                                               //历史记录为空 直接退出
    uShellGetcurrent()->pHistoryRead++;
    if(uShellGetcurrent()->pHistoryRead >= SHELL_HISTORY_NUM)
    {
        uShellGetcurrent()->pHistoryRead = 0;                                   //溢出，环形
    }
    if(uShellGetcurrent()->pHistoryRead == uShellGetcurrent()->pHistoryWrite)   //判断是否读空
    {
        uShellGetcurrent()->pHistoryRead--;
        uShellGetcurrent()->cmdBuffer[0] = '\0';
        uShellGetcurrent()->cmdBufferUsedLen = 0;
        uShellGetcurrent()->terminalCursorPosition = 0;
        uShellGetcurrent()->historyFlag = 0;
    }else
    {/*读出历史命令到命令缓冲区*/
        uShellGetcurrent()->cmdBufferUsedLen = uShellStrlen(uShellGetcurrent()->historyArray[uShellGetcurrent()->pHistoryRead+1]);
        uShellStrcpy(uShellGetcurrent()->cmdBuffer,                                             \
                     uShellGetcurrent()->historyArray[uShellGetcurrent()->pHistoryRead+1],    \
                     uShellGetcurrent()->cmdBufferUsedLen);
        uShellGetcurrent()->terminalCursorPosition = uShellGetcurrent()->cmdBufferUsedLen;
    }
    /*打印*/
    uShellGetcurrent()->pPrintf("\r\033[K");
    uShellGetcurrent()->pPrintf("%s ", SHELL_PREFIX);
    uShellGetcurrent()->pPrintf(uShellGetcurrent()->cmdBuffer);

    return 1;
}
#endif
/**
 * @brief           uShell自动完成功能,需要注意的是此函数目前仅支持列出最多10条
 *                  符合的命令
 * 
 * @param str       未使用
 * @return int      未使用
 */
int uShellAutoComplete(char *str){
    (void)(str);
    unsigned short i, index=0;
    unsigned short indexArray[10];
    for (i = 0; uShellGetcurrent()->pCmdList[i].type != INVALIDE_CMD; i++)      //查找符合条件的指令，并将索引存储
    {
        if(uShellStrContain(uShellGetcurrent()->pCmdList[i].pCmdStr, uShellGetcurrent()->cmdBuffer))
        {
            indexArray[index++] = i;
            if(index > 9)
                break;
        }
    }
    if(index == 0){
        return 0;           //无匹配项 退出
    }else if(index==1){     //仅有一个匹配项，直接填充
        uShellGetcurrent()->cmdBufferUsedLen = uShellStrlen(uShellGetcurrent()->pCmdList[indexArray[0]].pCmdStr);
        uShellStrcpy(uShellGetcurrent()->cmdBuffer,                         \
                     uShellGetcurrent()->pCmdList[indexArray[0]].pCmdStr,   \
                     uShellGetcurrent()->cmdBufferUsedLen);
        uShellGetcurrent()->terminalCursorPosition = uShellGetcurrent()->cmdBufferUsedLen;
        uShellGetcurrent()->pPrintf("\r\033[K");
        uShellGetcurrent()->pPrintf("%s ", SHELL_PREFIX);
        uShellGetcurrent()->pPrintf(uShellGetcurrent()->cmdBuffer);
    }else{                  //多个匹配项 分别列出
        for (i = 0; i < index; ++i){
            uShellGetcurrent()->pPrintf("\r\n%s", uShellGetcurrent()->pCmdList[indexArray[i]].pCmdStr);
        }
        uShellGetcurrent()->pPrintf("\r\n%s ", SHELL_PREFIX);
        uShellGetcurrent()->pPrintf(uShellGetcurrent()->cmdBuffer);
    }
    return 1;
}

/**
 * @brief                   字符串拷贝
 *
 * @param  dest             目标字符串，拷入
 * @param  str              源字符串，拷出
 * @param  len              拷贝长度
 * @return unsigned char    0x00 成功
 */
unsigned char uShellStrcpy(char *dest, char *src, unsigned short len)
{
    for (unsigned short i = 0; i < len; ++i)
    {
        dest[i] = src[i];
    }
    dest[len] = '\0';
    return 0;
}
/**
 * @brief                   字符串比较
 *
 * @param  str1             目标字符串1
 * @param  str2             目标字符串2
 * @param  len              目标字符串1长度
 * @return unsigned char    0x01: 字符串相等 0x00字符串不等
 */
unsigned char uShellStrcmp(char *str1, char *str2, unsigned short str1Len)
{
    if (str1Len != uShellStrlen(str2))
        return 0;
    for (unsigned short i = 0; i < str1Len; ++i)
    {
        if (str1[i] != str2[i])
            return 0;
    }
    return 1;
}
/**
 * @brief                   获取字符串长度
 *
 * @param                   字符串首地址
 * @return unsigned char    字符串长度
 */
unsigned short uShellStrlen(char *str)
{
    unsigned short i = 0;
    for (i = 0; str[i] != '\0'; ++i)
    {
    }
    return i;
}
/**
 * @brief                   在指定位置插入字符(光标左侧, Normal Mode)
 *
 * @param dest              目标字符串
 * @param maxlen            目标字符串最大长度
 * @param position          待插入位置
 * @param ch                待插入字符
 * @return unsigned char    0x01:Success; Other:Fail.
 */
unsigned char uShellStrInsertc(char *dest, unsigned short maxlen, unsigned short position, char ch)
{
    unsigned char i = 0;
    unsigned short currentLen = uShellStrlen(dest);
    if ((currentLen >= maxlen - 1) || (position >= maxlen - 1) || (position > currentLen)) //位置不对
        return 0;
    for (i = currentLen; i > position; --i)
    { //搬移字符
        dest[i] = dest[i - 1];
    }
    dest[position] = ch;         //插入字符
    dest[currentLen + 1] = '\0'; //插入结尾符
    return 0x01;
}
/**
 * @brief                   删除光标左侧字符(Backspace效果)
 *
 * @param dest              目标字符串
 * @param maxlen            目标字符串最大长度
 * @param position          光标位置
 * @return unsigned char    0x01:Success; Other:Fail.
 */
unsigned char uShellStrDeletelc(char *dest, unsigned short maxlen, unsigned short position)
{
    unsigned short currentLen = uShellStrlen(dest);
    unsigned char i = 0;
    if ((position > maxlen) || (position > currentLen))
        return 0;
    for (i = position - 1; i < currentLen - 1; ++i)
    {
        dest[i] = dest[i + 1];
    }
    dest[i] = '\0';
    return 1;
}
/**
 * @brief                   删除光标右侧字符(Del效果)
 *
 * @param dest              目标字符串
 * @param maxlen            目标字符串最大长度
 * @param position          光标位置
 * @return unsigned char    0x01:Success; Other:Fail.
 */
unsigned char uShellStrDeleterc(char *dest, unsigned short maxlen, unsigned short position)
{
    unsigned short currentLen = uShellStrlen(dest);
    unsigned char i = 0;
    if ((position >= currentLen - 1) || (position > maxlen))
        return 0;
    for (i = position; i < currentLen - 1; ++i)
    {
        dest[i] = dest[i + 1];
    }
    dest[i] = '\0';
    return 1;
}

/**
 * @brief                   清空字符串
 *
 * @param dest              目标字符串
 * @param len               目标长度
 * @return unsigned char    0x01:Success; Other:Fail.
 */
unsigned char uShellStrClear(char *dest, unsigned short len)
{
    if (dest == NULL)
        return 0;
    for (unsigned short i = 0; i < len; ++i)
    {
        dest[i] = '\0';
    }
    return 1;
}
/**
 * @brief                   str1是否包含str2,此函数仅从开头比对
 * 
 * @param str1              目标字符串
 * @param str2              待查询字符串
 * @return unsigned char    0:不包含, 1:包含
 */
unsigned char uShellStrContain(char *str1, char *str2)
{
    unsigned short len1 = uShellStrlen(str1);
    unsigned short len2 = uShellStrlen(str2);
    if(len1 ==0 || len2 ==0 || len2 > len1)
    {
        return 0;
    }
    for (unsigned short i = 0; i < len2; ++i)
    {
        if(str1[i]  != str2[i])
            return 0;
    }
    return 1;
}
/**
 * @brief           字符串转整形, 支持十六进制(前缀为0x), 正数、负数。
 *                  当传入小数字符串时, 将丢弃小数点后的数字，并返回-1, var中存储已转换的整数部分
 * @param str       待转换字符串
 * @param var       转换后的结果
 * @return int      转换结果。-1: Fail, 0:Success
 */
int uShell_atoi(char *str, int *var)
{
    unsigned short len = uShellStrlen(str);
    unsigned short i = 0;
    char state = 0;
    *var = 0;
    if (str[0] == '-')
    {
        state = 1;              //负数
        i = 1;
    }
    else if ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')))
    {
        state = 2;              //十六进制
        i = 2;
    }
    else if (str[0] == '+' ||((str[0] >= '0') && (str[0] <= '9')))
    {
        state = 0;              //正数
        if(str[0] == '+')
            i=1;   
    }
    else{
        return -1;
    }
    switch(state){
        case 0:
        case 1:
            for (; i < len; ++i)
            {
                if((str[i] < '0' || str[i] > '9'))
                    return -1;
                (*var) *= 10;
                (*var) += str[i] - '0';
            }
            break;
        case 2:
            for (; i < len; ++i)
            {
                if((str[i] >= '0' && str[i] <= '9') || \
                   (str[i] >= 'A' && str[i] <= 'F') || \
                   (str[i] >= 'a' && str[i] <= 'f'))
                {
                    (*var) = (*var) << 4;
                    if(str[i] <= '9')
                        (*var) += str[i] - '0';
                    else if(str[i] <= 'F')
                        (*var) += str[i] - 'A' + 0x0a;
                    else (*var) += str[i] - 'a' + 0x0a;
                }
                else
                    return -1;
            }
            break;
    }
    if(state == 1)
        *var = 0-(*var);
    return 0;
}
int uShell_atof(char *str, float *var)
{
    int integer = 0;                            //整数部分
    int decimal = 0;                            //小数部分
    unsigned short len = uShellStrlen(str);     //字符串长度
    char decimalDigits = 0;                     //小数位数
    char isNeg = 0;
    unsigned short i = 0;                       
    if(len == 0)
        return -1;
    switch(str[0]){
        case '+':
            i = 1;
            isNeg = 0;
            break;
        case '-':
            i = 1;
            isNeg = 1;
            break;
        default:
            isNeg = 0;
            i = 0;
    }
    for (; i < len;++i){
        if(('0' <= str[i] && str[i] <= '9'))
        {
            if(decimalDigits == 0){                 //整数部分
                integer *= 10;
                integer += (str[i] - '0');
            }else{                                  //小数部分
                decimal *= 10;
                decimal += (str[i] - '0');
            }

        }else if(str[i] == '.'){
            if(decimalDigits == 0)
                decimalDigits = i + 1;
            else
                return -1;
            //continue;
        }else{
            return -1;
        }
    }
    *var = (decimal * 1.0);
    for (; decimalDigits < len; decimalDigits++){
        *var = *var / 10;
    }
    *var += integer;
    if(isNeg)
    {
        *var = 0-(*var);
    }
    return 0;
}
/**
 * @brief                   清屏
 *
 * @param                   未使用
 * @return int              总为0
 */
int uShellClearscreen(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);
    uShellGetcurrent()->pPrintf(TERMINAL_CLEAR_SCREEN);
    return 0x00;
}

int uShellListAllcmd(int argc, char *argv[])
{
    (void)(argc);
    (void)(argv);
    unsigned int i;
    uShellGetcurrent()->pPrintf("\r\n********************Function********************\r\n");
    for (i = 0; uShellGetcurrent()->pCmdList[i].type != INVALIDE_CMD; i++)
    {
        if (uShellGetcurrent()->pCmdList[i].type == FUNCTION_CMD)
        {
            uShellGetcurrent()->pPrintf("\r\n");
            uShellGetcurrent()->pPrintf(uShellGetcurrent()->pCmdList[i].pCmdStr);
            uShellGetcurrent()->pPrintf("\033[%dC", 20 - uShellStrlen(uShellGetcurrent()->pCmdList[i].pCmdStr));
            uShellGetcurrent()->pPrintf("--------");
            uShellGetcurrent()->pPrintf("\033[10C");
            uShellGetcurrent()->pPrintf(uShellGetcurrent()->pCmdList[i].description);
        }
    }
    uShellGetcurrent()->pPrintf("\r\n");
    return 0x00;
}
int uShellListAllVar(int argc, char(*argv[]))
{
    (void)(argc);
    (void)(argv);
    unsigned int i;
    uShellGetcurrent()->pPrintf("\r\n********************Variable********************\r\n");
    for (i = 0; uShellGetcurrent()->pCmdList[i].type != INVALIDE_CMD; i++)
    {
        if (uShellGetcurrent()->pCmdList[i].type == VARIABLE_INT)
        {
            uShellGetcurrent()->pPrintf("\r\n");
            uShellGetcurrent()->pPrintf(uShellGetcurrent()->pCmdList[i].pCmdStr);
            uShellGetcurrent()->pPrintf("\033[%dC", 20 - uShellStrlen(uShellGetcurrent()->pCmdList[i].pCmdStr));
            uShellGetcurrent()->pPrintf("val=:%10d", *(int*)uShellGetcurrent()->pCmdList[i].pVar);
            uShellGetcurrent()->pPrintf("\033[2C");
            uShellGetcurrent()->pPrintf("--------");
            uShellGetcurrent()->pPrintf("\033[10C");
            uShellGetcurrent()->pPrintf(uShellGetcurrent()->pCmdList[i].description);
        }
    }
    uShellGetcurrent()->pPrintf("\r\n");
    return 0x00;
}
int uShellListAll(int argc, char *argv[])
{
    uShellListAllcmd(argc, argv);
    uShellListAllVar(argc, argv);
    return 0;
}
int uShellEcho(int argc, char *argv[])
{
    if(argc == 1)
    {
        uShellGetcurrent()->pPrintf("\r\necho: echo [-neE] [arg ...]");
        uShellGetcurrent()->pPrintf("\r\n    Write arguments to the standard output.\r\n");
        uShellGetcurrent()->pPrintf("\r\n    Display the ARGs, separated by a single space character and followed by a newline, on the standard output.");

    }else{
        for(int i = 1; i < argc; ++i)
        {
            uShellGetcurrent()->pPrintf("\r\n");
            uShellGetcurrent()->pPrintf((char*)argv + i * SHELL_COMMAND_MAX_LENGTH);
        }
    }
    return 0;
}

int uShellTestatoi(int argc, char *argv[])
{
    int val = 0;
    char *pChar = (char *)argv + 1 * SHELL_COMMAND_MAX_LENGTH;
    if(argc == 1){
        uShellGetcurrent()->pPrintf("\r\nToo few parameters!\r\n");
    }else 
    {
        if(argc >2){
            uShellGetcurrent()->pPrintf("\r\nToo many parameters!, Extra parameters will be discarded!");
        }
        uShellGetcurrent()->pPrintf("\r\nreturn: %d, intVal = %d",uShell_atoi(pChar, &val), val);
    }
    return 0;
}
int uShellTestatof(int argc, char *argv[])
{
    float val = 0;
    char *pChar = (char *)argv + 1 * SHELL_COMMAND_MAX_LENGTH;
    int retval = 0;
    if(argc == 1){
        uShellGetcurrent()->pPrintf("\r\nToo few parameters!\r\n");
    }else 
    {
        if(argc >2){
            uShellGetcurrent()->pPrintf("\r\nToo many parameters!, Extra parameters will be discarded!");
        }
        retval = uShell_atof(pChar, &val);
        uShellGetcurrent()->pPrintf("\r\nreturn: %d, floatVal = %f",retval, val);
    }
    return 0;
}
int uShellTestStrcontain(int argc, char *argv[])
{
    unsigned short result = 0;
    char *str1 = NULL, *str2 = NULL;
    if(argc < 3){
        uShellGetcurrent()->pPrintf("\r\nToo few parameters!\r\n");
        uShellGetcurrent()->pPrintf("\r\n Usage testStrContain \"string1\"  \"string2\"");
    }else{
        str1 = (char *)argv + 1 * SHELL_COMMAND_MAX_LENGTH;
        str2 = (char *)argv + 2 * SHELL_COMMAND_MAX_LENGTH;
        result = uShellStrContain(str2, str1);
        if(result)
        {
            uShellGetcurrent()->pPrintf("\r\n \"%s\" is contain \"%s\"", str2, str1);
        }else
        {
            uShellGetcurrent()->pPrintf("\r\n \"%s\" isn't contain \"%s\"", str2, str1);
        }
    }
    return 0;
}


