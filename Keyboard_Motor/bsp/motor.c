#include "motor.h"
#include "Emm_V5.h"
#include "delay.h"

/**********************************************************
 *** Emm_V5 双轴电机控制实现
 ***
 *** Motor_Init: 上电等待 -> 使能双轴 -> Y轴切闭环FOC+电流 -> 设QPos参数
 *** Motor_MoveX/Y: 设QPos参数(每次重设, 确保速度匹配) + 发QPos_Control命令
 ***
 *** 阻塞发送: Emm_V5 内部 usart_SendByte 等 DL_UART_isBusy, 天然串行,
 ***           不需要像 STM32 那样管 DMA 完成回调/双缓冲.
 **********************************************************/

void Motor_Init(void)
{
    delay_ms(500);   /* 等电机驱动板上电初始化 */

    /* 使能双轴 */
    Emm_V5_En_Control(MOTOR_ADDR_X, 1, 0);
    delay_ms(10);
    Emm_V5_En_Control(MOTOR_ADDR_Y, 1, 0);
    delay_ms(10);

    /* Y轴重载补偿: 切闭环FOC + 提电流 (闭环自动补丢步抗重力矩)
       编码器校准(Trig_Encoder_Cal)是出厂一次性操作, 不放这里.
       svF=1 写Flash: 首次配置写一次, 后续调参改 svF=0 只改运行时不写Flash延长寿命. */
    Emm_V5_Modify_Ctrl_Mode(MOTOR_ADDR_Y, 1, 1);        /* ctrl_mode=1 闭环FOC */
    delay_ms(50);
    Emm_V5_Modify_FOC_mA(MOTOR_ADDR_Y, 1, MOTOR_Y_FOC_mA);  /* 闭环电流 */
    delay_ms(50);

    /* 设快速位置模式参数 (raF=2 相对当前位置, 每次发 FC 走指定脉冲) */
    Emm_V5_Set_QPos_Params(MOTOR_ADDR_X, MOTOR_X_SPEED_RPM, MOTOR_X_ACC, 2, 0);
    delay_ms(10);
    Emm_V5_Set_QPos_Params(MOTOR_ADDR_Y, MOTOR_Y_SPEED_RPM, MOTOR_Y_ACC, 2, 0);
    delay_ms(10);
}

void Motor_MoveX(int32_t pulse)
{
    /* 方向反转 */
    if (MOTOR_X_INVERT) pulse = -pulse;

    /* 设QPos参数 (raF=2 相对当前位置) + 发命令 */
    Emm_V5_Set_QPos_Params(MOTOR_ADDR_X, MOTOR_X_SPEED_RPM, MOTOR_X_ACC, 2, 0);
    delay_ms(5);
    Emm_V5_QPos_Control(MOTOR_ADDR_X, pulse);
}

void Motor_MoveY(int32_t pulse)
{
    /* 方向反转 */
    if (MOTOR_Y_INVERT) pulse = -pulse;

    /* Y轴用低速低加速抗重力矩 */
    Emm_V5_Set_QPos_Params(MOTOR_ADDR_Y, MOTOR_Y_SPEED_RPM, MOTOR_Y_ACC, 2, 0);
    delay_ms(5);
    Emm_V5_QPos_Control(MOTOR_ADDR_Y, pulse);
}

void Motor_MoveX_Deg(int16_t deg)
{
    /* 度 -> 脉冲 (16细分 3200脉冲/圈), 四舍五入 */
    int32_t pulse = (int32_t)((float)deg * MOTOR_PULSE_PER_DEG + 0.5f);
    Motor_MoveX(pulse);
}

void Motor_MoveY_Deg(int16_t deg)
{
    int32_t pulse = (int32_t)((float)deg * MOTOR_PULSE_PER_DEG + 0.5f);
    Motor_MoveY(pulse);
}

/**********************************************************
 *** 速度模式 (按住转松开停)
 *** Motor_VelX/Y: Emm_V5_Vel_Control (0xF6), vel=0 即停.
 *** Motor_StopX/Y: Emm_V5_Stop_Now (0xFE) 立即停.
 *** 方向反转: 尊重 MOTOR_X/Y_INVERT (与位置模式一致).
 ***
 *** dir 约定: 0=正转(默认X左转/Y上转), 1=反转(默认X右转/Y下转).
 *** 调用方按住期间持续发同一条命令无害(电机收到相同速度命令保持转),
 *** 但主循环做了状态变化判断, 只在切换时发, 避免总线刷屏.
 **********************************************************/
void Motor_VelX(uint8_t dir, uint16_t vel_rpm)
{
    if (MOTOR_X_INVERT) dir ^= 1;   /* 方向反转 */
    Emm_V5_Vel_Control(MOTOR_ADDR_X, dir, vel_rpm, MOTOR_VEL_ACC, 0);
}

void Motor_VelY(uint8_t dir, uint16_t vel_rpm)
{
    if (MOTOR_Y_INVERT) dir ^= 1;
    Emm_V5_Vel_Control(MOTOR_ADDR_Y, dir, vel_rpm, MOTOR_VEL_ACC, 0);
}

void Motor_StopX(void)
{
    Emm_V5_Stop_Now(MOTOR_ADDR_X, 0);
}

void Motor_StopY(void)
{
    Emm_V5_Stop_Now(MOTOR_ADDR_Y, 0);
}
