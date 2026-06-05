#include "matrix_key.h"
#include "OLED.h"
#include "SERIAL.h"

sbit BEEP = P2^3;

// 系统状态定义
typedef enum {
    STATE_MAIN_MENU,    // 主菜单界面
    STATE_FACE_UNLOCK,  // 人脸识别开锁中
    STATE_UNLOCK_OK,    // 开锁成功
    STATE_UNLOCK_FAIL,  // 开锁失败
    STATE_FACE_ENROLL,  // 人脸录入中
    STATE_PWD_INPUT,    // 普通密码开锁输入
    STATE_PWD_MODIFY,   // 改密：输入旧密码
    STATE_NEW_PWD       // 改密输入新密码
} SystemState;

SystemState current_state = STATE_MAIN_MENU;
unsigned int delay_counter = 0;

// 密码全局
unsigned char Input_Cnt = 0;
unsigned long Input_Pwd = 0;
unsigned long User_Pwd = 123456;  // 初始密码123456
#define PWD_LEN 6
#define PWD_START_X 6   // 密码起始X坐标

/**
 * @brief 短滴(按键音)：50ms
 */
void Beep_Short(void)
{
    BEEP = 0;
    Delay_ms(50);
    BEEP = 1;
}

/**
 * @brief 成功长鸣：2s（开锁/密码正确）
 */
void Beep_Long(void)
{
    BEEP = 0;
    Delay_ms(2000);
    BEEP = 1;
}

/**
 * @brief 录入成功：滴滴两声
 */
void Beep_DoubleOk(void)
{
    BEEP = 0;
    Delay_ms(150);
    BEEP = 1;
    Delay_ms(150);
    BEEP = 0;
    Delay_ms(150);
    BEEP = 1;
}

/**
 * @brief 错误三连鸣：响500 停500，循环3次
 */
void Beep_ThreeErr(void)
{
    unsigned char i;
    for(i = 0; i < 3; i++)
    {
        BEEP = 0;
        Delay_ms(500);
        BEEP = 1;
        Delay_ms(500);
    }
}

void Serial_SendCmd(unsigned char cmd1, unsigned char cmd2, unsigned char cmd3)
{
    SERIAL_SendByte(cmd1);
    SERIAL_SendByte(cmd2);
    SERIAL_SendByte(cmd3);
}

// 界面：主菜单
void Show_MainMenu(void)
{
    OLED_Clear();
    OLED_ShowString(0, 0, "  SMART LOCK");
    OLED_ShowString(0, 2, "1.FACE UNLOCK");
    OLED_ShowString(0, 4, "2.ENROLL FACE");
    OLED_ShowString(0, 6, "3.PASSWORD");
}

// ==================密码功能封装函数==================
/**
 * @brief 【普通开锁】INPUT PWD：保留16改密提示，EXIT改为15
 */
void Pwd_StartInput(void)
{
    unsigned char i;
    Input_Cnt = 0;
    Input_Pwd = 0;
    OLED_Clear();
    OLED_ShowString(0, 1, "INPUT PWD:");
    for(i=0;i<6;i++)
    {
        OLED_ShowChar(PWD_START_X + i*8,3,'-');
    }
    for(i=0;i<6;i++)
    {
        OLED_ShowChar(PWD_START_X + i*8,2,' ');
    }
    OLED_ShowString(0,5,"11:DELETE 12:OK");
    OLED_ShowString(0,6,"15:EXIT 16:MODIFY PWD");
}

/**
 * @brief 【改密第一步：输入旧密码】INPUT OLD PWD：移除16提示，EXIT15
 */
void Pwd_StartModify(void)
{
    unsigned char i;
    Input_Cnt = 0;
    Input_Pwd = 0;
    OLED_Clear();
    OLED_ShowString(0, 1, "INPUT OLD PWD");
    for(i=0;i<6;i++)
    {
        OLED_ShowChar(PWD_START_X + i*8,3,'-');
    }
    for(i=0;i<6;i++)
    {
        OLED_ShowChar(PWD_START_X + i*8,2,' ');
    }
    OLED_ShowString(0,5,"11:DELETE 12:VERIFY");
    OLED_ShowString(0,6,"15:EXIT");
}

/**
 * @brief 【改密第二步：输入新密码】INPUT NEW PWD：移除16提示，EXIT15
 */
void New_Pwd_Set(void)
{
    unsigned char i;
    Input_Cnt = 0;
    Input_Pwd = 0;
    OLED_Clear();
    OLED_ShowString(0,1,"INPUT NEW PWD");
    for(i=0;i<6;i++)
    {
        OLED_ShowChar(PWD_START_X+i*8,3,'-');
    }
    for(i=0;i<6;i++)
    {
        OLED_ShowChar(PWD_START_X+i*8,2,' ');
    }
    OLED_ShowString(0,5,"11:DELETE 12:SAVE");
    OLED_ShowString(0,6,"15:EXIT");
}

/**
 * @brief 密码输入：数字写在对应'-'正上方(Y=2)
 */
void Pwd_Input_Digit(unsigned char num)
{
    if(Input_Cnt >= PWD_LEN) return;
    Input_Pwd = Input_Pwd * 10 + num;
    OLED_ShowChar(PWD_START_X + Input_Cnt*8,2,num + '0');
    Input_Cnt++;
}

/**
 * @brief 退格：清空上方最后一位数字
 */
void Pwd_Del_One(void)
{
    if(Input_Cnt == 0) return;
    Input_Cnt--;
    Input_Pwd = Input_Pwd / 10;
    OLED_ShowChar(PWD_START_X + Input_Cnt*8,2,' ');
}

/**
 * @brief 保存新密码
 */
void Pwd_SaveNew(unsigned long newpwd)
{
    User_Pwd = newpwd;
}
// ===================================================

void Serial_CheckCmd(void)
{
    static unsigned char rx_count = 0;
    static unsigned char rx_buf[3];
    
    if(SERIAL_Available())
    {
        rx_buf[rx_count++] = SERIAL_ReadByte();
        if(rx_count == 3)
        {
            if(rx_buf[0]==0xA0 && rx_buf[1]==0xA1 && rx_buf[2]==0xFF)
            {
                SERIAL_SendByte(0xA1);
                if(current_state == STATE_FACE_UNLOCK)
                {
                    current_state = STATE_UNLOCK_OK;
                    OLED_Clear();
                    OLED_ShowString(0, 3, "UNLOCK SUCCESS!");
                    delay_counter = 0;
                }
                Beep_Long();
            }
            else if(rx_buf[0]==0xA0 && rx_buf[1]==0xA2 && rx_buf[2]==0xFF)
            {
                SERIAL_SendByte(0xA2);
                if(current_state == STATE_FACE_UNLOCK)
                {
                    current_state = STATE_UNLOCK_FAIL;
                    OLED_Clear();
                    OLED_ShowString(0, 3, "UNLOCK FAILED!");
                    delay_counter = 0;
                }
                Beep_ThreeErr();
            }
            // ========== 新增：人脸录入成功/失败命令处理 ==========
            else if(rx_buf[0]==0xA0 && rx_buf[1]==0xA3 && rx_buf[2]==0xFF)
            {
                SERIAL_SendByte(0xA3);
                if(current_state == STATE_FACE_ENROLL)
                {
                    current_state = STATE_UNLOCK_OK;
                    OLED_Clear();
                    OLED_ShowString(0, 3, "ENROLL SUCCESS!");
                    delay_counter = 0;
                    Beep_DoubleOk();
                }
            }
            else if(rx_buf[0]==0xA0 && rx_buf[1]==0xA4 && rx_buf[2]==0xFF)
            {
                SERIAL_SendByte(0xA4);
                if(current_state == STATE_FACE_ENROLL)
                {
                    current_state = STATE_UNLOCK_FAIL;
                    OLED_Clear();
                    OLED_ShowString(0, 3, "ENROLL FAILED!");
                    delay_counter = 0;
                    Beep_ThreeErr();
                }
            }
            // ==================================================
            else
                SERIAL_SendByte(0xA2);
            rx_count = 0;
        }
    }
}

void MatrixKey_Process(void)
{
    unsigned char key_val;
    unsigned char num;
    key_val = MatrixKey();
    
    if(key_val != 0)
    {
        Beep_Short();

        //16只在主菜单/开锁页有效进入改密，旧密新密页16无效
        if(key_val == 16)
        {
            if(current_state == STATE_MAIN_MENU || current_state == STATE_PWD_INPUT)
            {
                current_state = STATE_PWD_MODIFY;
                Pwd_StartModify();
            }
            Delay_ms(200);
            return;
        }
        
        //=========密码相关页面：开锁/旧密码/新密码=========
        if(current_state == STATE_PWD_INPUT || current_state == STATE_PWD_MODIFY || current_state == STATE_NEW_PWD)
        {
            switch(key_val)
            {
                case 1:num=0;Pwd_Input_Digit(num);break;
                case 2:num=1;Pwd_Input_Digit(num);break;
                case 3:num=2;Pwd_Input_Digit(num);break;
                case 4:num=3;Pwd_Input_Digit(num);break;
                case 5:num=4;Pwd_Input_Digit(num);break;
                case 6:num=5;Pwd_Input_Digit(num);break;
                case 7:num=6;Pwd_Input_Digit(num);break;
                case 8:num=7;Pwd_Input_Digit(num);break;
                case 9:num=8;Pwd_Input_Digit(num);break;
                case 10:num=9;Pwd_Input_Digit(num);break;
                case 11:Pwd_Del_One();break;    //删除
                case 12: //确认/校验/保存
                    //1、普通开锁校验密码
                    if(current_state == STATE_PWD_INPUT)
                    {
                        if(Input_Pwd == User_Pwd)
                        {
                            current_state = STATE_UNLOCK_OK;
                            OLED_Clear();
                            OLED_ShowString(0,3,"UNLOCK SUCCESS!");
                            delay_counter=0;
                            Beep_Long();
                        }
                        else
                        {
                            OLED_Clear();
                            OLED_ShowString(0,3,"PWD ERROR!");
                            delay_counter = 0;
                            current_state=STATE_UNLOCK_FAIL;
                            Beep_ThreeErr();
                        }
                    }
                    //2、改密：校验旧密码
                    else if(current_state == STATE_PWD_MODIFY)
                    {
                        if(Input_Pwd == User_Pwd)
                        {
                            current_state = STATE_NEW_PWD;
                            New_Pwd_Set();
                        }
                        else
                        {
                            OLED_Clear();
                            OLED_ShowString(0,3,"OLD PWD ERR");
                            delay_counter = 0;
                            current_state=STATE_UNLOCK_FAIL;
                            Beep_ThreeErr();
                        }
                    }
                    //3、新密码页面：直接保存
                    else if(current_state == STATE_NEW_PWD)
                    {
                        Pwd_SaveNew(Input_Pwd);
                        current_state = STATE_UNLOCK_OK;
                        OLED_Clear();
                        OLED_ShowString(0,3,"MOD SUCCESS!");
                        delay_counter=0;
                        Beep_Long();
                    }
                    break;
                case 15: //原13退出，改为15
                    current_state=STATE_MAIN_MENU;
                    Show_MainMenu();
                    break;
                case 13: break;
                case 14: break;
                default:break;
            }
            Delay_ms(200);
            return;
        }
        
        //====主菜单按键====
        if(current_state == STATE_MAIN_MENU)
        {
            switch(key_val)
            {
                case 1:
                    current_state = STATE_FACE_UNLOCK;
                    OLED_Clear();
                    OLED_ShowString(0, 3, "SCANNING FACE...");
                    Serial_SendCmd(0xA0,0xB1,0xFF);
                    delay_counter = 0; // 重置超时计数器
                    break;
                case 2:
                    current_state = STATE_FACE_ENROLL;
                    OLED_Clear();
                    OLED_ShowString(0,3,"ENROLLING FACE...");
                    Serial_SendCmd(0xA0,0xB2,0xFF);
                    delay_counter = 0; // 重置超时计数器
                    // 移除了原有的自动延时返回，改为等待模块串口结果
                    break;
                case 3:
                    current_state=STATE_PWD_INPUT;
                    Pwd_StartInput();
                    break;
                case 13: break;
                case 14: break;
                case 15: break;
                default:break;
            }
        }
        else
        {
            current_state=STATE_MAIN_MENU;
            Show_MainMenu();
        }
        Delay_ms(200);
    }
}

void main(void)
{
    BEEP = 1;
    Input_Cnt = 0;
    Input_Pwd = 0;
    SERIAL_Init();
    
    Delay_ms(200);
    OLED_Init();
    
    Show_MainMenu();
    
    while(1)
    {
        Serial_CheckCmd();
        MatrixKey_Process();
        
        switch(current_state)
        {
            case STATE_UNLOCK_OK:
            case STATE_UNLOCK_FAIL:
            case STATE_FACE_UNLOCK:
            case STATE_FACE_ENROLL:
                delay_counter++;
                if(delay_counter>40000) // 全部40s超时（录入/解锁/结果页统一）
                {
                    current_state=STATE_MAIN_MENU;
                    Show_MainMenu();
                    delay_counter=0;
                }
                break;
            default:
                delay_counter = 0;
                break;
        }
        Delay_ms(1);
    }
}
