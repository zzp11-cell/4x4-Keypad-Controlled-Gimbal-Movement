/**
 * @file    main.c  (4x4 矩阵键盘控制双轴云台电机) - 天猛星 MSPM0G3507
 * @brief   UART3(PB12/PB13) -> Emm_V5 双轴步进电机 (115200, 默认32MHz时钟)
 *          4x4 矩阵键盘 8 键速度模式: 按住转, 松开停
 *
 *** 接线:
 ***   电机: PB12(TX)->电机RAH, PB13(RX)<-电机TBL, GND共地, 电机独立供电12V/24V
 ***   键盘行 R1-R4: R1=PA28, R2=PA31, R3=PB4, R4=PB5  (输出扫描)
 ***   键盘列 C1-C4: C1=PA9, C2=PA8, C3=PA1, C4=PA0    (输入上拉)
 ***   LED: PB22 (用户灯, 高有效)
 ***
 *** 按键功能 (速度模式, 只用 S1-S8):
 ***   R1行: S1=X慢左  S2=X慢右  S3=X快左  S4=X快右
 ***   R2行: S5=Y慢上  S6=Y慢下  S7=Y快上  S8=Y快下
 ***   (电机1=X轴左右底座, 电机2=Y轴上下叠在电机1上随之在X轴移动)
 ***   按住->该轴持续转, 松开->该轴停; 跨轴切换自动停旧轴
 ***
 *** 上电流程:
 ***   1. LED闪3次 -> 初始化
 ***   2. 电机使能锁定(手转不动有保持力) -> Motor_Init 完成
 ***   3. LED快闪2次 -> 进入键盘扫描
 ***   4. 按住键->电机持续转+LED亮, 松开->停+LED灭
 ***
 *** 方向不对: 改 motor.h 的 MOTOR_X_INVERT / MOTOR_Y_INVERT (0<->1)
 *** 速度不对: 改 motor.h 的 MOTOR_*_VEL_SLOW/FAST (RPM)
 */
#include "ti_msp_dl_config.h"
#include "delay.h"
#include "clock.h"        /* tick_ms, SysTick_Init */
#include "keypad.h"
#include "motor.h"
#include "fifo.h"        /* fifo_initQueue (UART3 接收缓冲) */
#include <stdbool.h>

/* ==================== LED 指示 (PB22 用户灯, 高有效) ====================
 * 25diansai1 工程已确认天猛星板载用户灯 PB22 为高电平点亮.
 *   LED_ON  = 拉高(setPins)   LED_OFF = 拉低(clearPins)
 * (PA7 不再用作指示灯, config 里已不初始化, 保持上电默认高阻态) */
#define LED_ON()      DL_GPIO_setPins(LED_user_PORT, LED_user_PIN)
#define LED_OFF()     DL_GPIO_clearPins(LED_user_PORT, LED_user_PIN)
#define LED_TOGGLE()  DL_GPIO_togglePins(LED_user_PORT, LED_user_PIN)

/* ==================== 按键 -> 电机动作映射 (速度模式) ====================
 * 键号 1-8 对应 S1-S8, 每个键指定 (电机轴, 方向, 速度档)
 * 按住 -> 该轴以指定方向速度持续转; 松开 -> 该轴停.
 *   轴:  0=电机1(X左右), 1=电机2(Y上下)
 *   方向: 0=正(X左转/Y上转), 1=反(X右转/Y下转)
 *   速度: 0=慢档, 1=快档 (具体 RPM 见 motor.h MOTOR_*_VEL_SLOW/FAST) */
typedef struct {
    uint8_t  axis;      /* 0=X轴(电机1), 1=Y轴(电机2) */
    uint8_t  dir;       /* 0=正转(左/上), 1=反转(右/下) */
    uint8_t  fast;      /* 0=慢档, 1=快档 */
} key_map_t;

static const key_map_t key_map[8] = {
    /* S1 */ { 0, 0, 0 },   /* X轴 慢左转 */
    /* S2 */ { 0, 1, 0 },   /* X轴 慢右转 */
    /* S3 */ { 0, 0, 1 },   /* X轴 快左转 */
    /* S4 */ { 0, 1, 1 },   /* X轴 快右转 */
    /* S5 */ { 1, 0, 0 },   /* Y轴 慢上转 */
    /* S6 */ { 1, 1, 0 },   /* Y轴 慢下转 */
    /* S7 */ { 1, 0, 1 },   /* Y轴 快上转 */
    /* S8 */ { 1, 1, 1 },   /* Y轴 快下转 */
};

/* ==================== 主函数 ==================== */
int main(void)
{
    SYSCFG_DL_init();        /* UART3 + 键盘GPIO + LED + 默认32MHz时钟 */
    SysTick_Init();          /* SysTick 1ms -> tick_ms (delay_ms依赖) */

    /* 使能 UART3 (电机) RX 中断 (收电机响应, usart.c 的 UART_3_INST_IRQHandler) */
    NVIC_ClearPendingIRQ(UART_3_INST_INT_IRQN);
    /* 初始化 UART3 接收 FIFO (电机响应缓冲, usart.c 中断里 fifo_enQueue) */
    fifo_initQueue();
    NVIC_EnableIRQ(UART_3_INST_INT_IRQN);

    /* 上电闪灯 3 次 (初始化指示) */
    for (int i = 0; i < 3; i++) {
        LED_ON();
        delay_ms(150);
        LED_OFF();
        delay_ms(150);
    }

    /* 电机初始化 (使能 + Y轴闭环FOC + QPos参数), 内部阻塞~1.5s */
    Motor_Init();

    /* 初始化完成指示: LED 快闪 2 次 */
    LED_ON();  delay_ms(100);
    LED_OFF(); delay_ms(100);
    LED_ON();  delay_ms(100);
    LED_OFF();

    /* 主循环: 扫描键盘 -> 按键映射 -> 速度命令 (按住转, 松开停) */
    uint8_t last_key = 0;   /* 上次按住的键号, 用于状态变化判断 */

    while (1) {
        uint8_t key = Key_Scan();   /* 当前按住的键 1-8, 0=松开 */

        /* 只在按键状态变化时发命令 (按住同一键不重复发, 松开瞬间发停止) */
        if (key != last_key) {
            if (key >= 1 && key <= 8) {
                /* 按下/切换到新键: 若跨轴切换, 先停旧轴 */
                if (last_key >= 1 && last_key <= 8) {
                    uint8_t old_axis = key_map[last_key - 1].axis;
                    uint8_t new_axis = key_map[key - 1].axis;
                    if (old_axis != new_axis) {
                        /* 跨轴: 停旧轴 */
                        if (old_axis == 0) Motor_StopX();
                        else               Motor_StopY();
                    }
                }
                /* 发新键的速度命令 */
                const key_map_t *m = &key_map[key - 1];
                uint16_t vel = m->fast
                    ? (m->axis == 0 ? MOTOR_X_VEL_FAST : MOTOR_Y_VEL_FAST)
                    : (m->axis == 0 ? MOTOR_X_VEL_SLOW : MOTOR_Y_VEL_SLOW);
                if (m->axis == 0) Motor_VelX(m->dir, vel);
                else              Motor_VelY(m->dir, vel);
            } else {
                /* 松开 (key=0): 停上次按的那个键对应的轴 */
                if (last_key >= 1 && last_key <= 8) {
                    uint8_t old_axis = key_map[last_key - 1].axis;
                    if (old_axis == 0) Motor_StopX();
                    else               Motor_StopY();
                }
            }
            last_key = key;
        }

        /* LED 反馈: 按住任何键时常亮, 松开灭 */
        if (key >= 1 && key <= 8) LED_ON();
        else                       LED_OFF();

        delay_ms(1);   /* 主循环节拍, 避免空转 */
    }
}

/* ==================== SysTick 中断 (1ms tick) ====================
 * clock.c 的 SysTick_Init 用 DL_SYSTICK_config(CPUCLK_FREQ/1000) 配 1ms 中断,
 * 这里累加 tick_ms (供 delay_ms 用). */
void SysTick_Handler(void)
{
    tick_ms++;
}

/* ==================== Error_Handler (简化) ==================== */
void Error_Handler(void) { while (1) { } }
