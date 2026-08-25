#ifndef __MOTOR_H
#define __MOTOR_H

#include "ti_msp_dl_config.h"
#include <stdint.h>
#include <stdbool.h>

/**********************************************************
 *** Emm_V5 双轴闭环步进电机控制模块 (天猛星 MSPM0G3507)
 ***
 *** 硬件: UART3 (PB12=TX, PB13=RX) -> Emm_V5 驱动板, 115200-8N1
 ***   电机1 (ID=1) = X轴 (左右, 底座水平旋转)
 ***   电机2 (ID=2) = Y轴 (上下, 俯仰, 叠在电机1上随之在X轴移动)
 ***
 *** 16细分, 3200脉冲/圈 = 360°
 ***   15° = 133 脉冲,  30° = 267 脉冲
 ***
 *** 方向定义 (Emm_V5_QPos_Control 有符号脉冲, 正=CW顺时针, 负=CCW逆时针):
 ***   电机1: 左转 = +脉冲, 右转 = -脉冲  (实测后改 MOTOR_X_INVERT)
 ***   电机2: 上转 = +脉冲, 下转 = -脉冲  (实测后改 MOTOR_Y_INVERT)
 ***
 *** Y轴重载补偿: 闭环FOC + 低速(120RPM) + 低加速(60) + 电流3000mA
 ***   (加电池+开发板后抬不动, 调 MOTOR_Y_FOC_mA 加到5000, 发烫再降)
 **********************************************************/

/* 电机地址 */
#define MOTOR_ADDR_X   1   /* 电机1 = X轴(左右) */
#define MOTOR_ADDR_Y   2   /* 电机2 = Y轴(上下) */

/* 角度->脉冲换算 (16细分, 3200脉冲/圈) */
#define MOTOR_PULSE_PER_DEG   (3200.0f / 360.0f)   /* ≈8.89 脉冲/度 */
#define MOTOR_PULSE_15DEG     (133)                /* 15° ≈ 133 脉冲 */
#define MOTOR_PULSE_30DEG     (267)                /* 30° ≈ 267 脉冲 */

/* 方向反转开关 (实测后改 0/1) */
#define MOTOR_X_INVERT   0   /* 1=电机1方向取反 */
#define MOTOR_Y_INVERT   0   /* 1=电机2方向取反 */

/* Y轴重载参数 */
#define MOTOR_Y_FOC_mA       3000   /* Y轴闭环电流(mA), 抬不动加到5000 */
#define MOTOR_X_SPEED_RPM   300    /* X轴位置模式速度 */
#define MOTOR_X_ACC         100    /* X轴加速度 */
#define MOTOR_Y_SPEED_RPM   120    /* Y轴位置模式速度 (重载低速) */
#define MOTOR_Y_ACC         60     /* Y轴加速度 (重载低加速) */

/* ===== 速度模式参数 (按住转松开停) =====
 * 快慢两档, 云台手动调位用. 嫌快改小, 嫌慢改大. */
#define MOTOR_X_VEL_SLOW    30     /* X轴慢档 RPM */
#define MOTOR_X_VEL_FAST    120    /* X轴快档 RPM */
#define MOTOR_Y_VEL_SLOW    15     /* Y轴慢档 RPM (重载更慢) */
#define MOTOR_Y_VEL_FAST    60     /* Y轴快档 RPM */
#define MOTOR_VEL_ACC       10     /* 速度模式加速度 (0-255, 0=无加减速直接起) */

/* 电机初始化: 使能双轴 + Y轴切闭环FOC + 设QPos位置模式参数
 * 返回前阻塞 ~1.5s (含电机上电等待) */
void Motor_Init(void);

/* 电机1(X轴)走指定脉冲 (相对当前位置, raF=2)
 * pulse: 正=CW(默认左转), 负=CCW(默认右转) */
void Motor_MoveX(int32_t pulse);

/* 电机2(Y轴)走指定脉冲 (相对当前位置, raF=2)
 * pulse: 正=CW(默认上转), 负=CCW(默认下转) */
void Motor_MoveY(int32_t pulse);

/* 电机走指定角度(度)的便捷接口, 正负同 Motor_MoveX/Y */
void Motor_MoveX_Deg(int16_t deg);
void Motor_MoveY_Deg(int16_t deg);

/* ===== 速度模式 (按住转松开停) =====
 * Motor_VelX/Y: 以指定 RPM 持续转 (vel=0 即停). dir: 0=正(左/上), 1=反(右/下).
 * Motor_StopX/Y: 立即停该轴. */
void Motor_VelX(uint8_t dir, uint16_t vel_rpm);
void Motor_VelY(uint8_t dir, uint16_t vel_rpm);
void Motor_StopX(void);
void Motor_StopY(void);

#endif /* __MOTOR_H */
