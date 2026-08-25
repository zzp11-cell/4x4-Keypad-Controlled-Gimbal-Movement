#include "keypad.h"
#include "delay.h"

/**********************************************************
 *** 4x4 矩阵键盘扫描实现 (行出列入) - 速度模式
 ***
 *** 扫描原理:
 ***   行(R1-R4)默认输出高(不选中). 扫描时逐行拉低, 读列(C1-C4).
 ***   列上拉, 按下读到0. 据此定位按键.
 ***
 *** 速度模式语义 (按住持续返回, 松开返回0):
 ***   主循环每 ~1ms 调一次, 返回当前按住的键号 (1-8), 无键返回 0.
 ***   主循环据此发速度命令(有键->该轴转)或停止命令(无键->该轴停).
 ***   消抖: 连续两次扫描值一致才更新稳定输出, 过滤抖动.
 **********************************************************/

/* 选中指定行: 拉低该行, 其余行拉高(不选中)
 * row: 0=R1, 1=R2, 2=R3, 3=R4, 其他值=全部不选中 */
static void select_row(uint8_t row)
{
    /* 先全部行拉高 */
    DL_GPIO_setPins(KEY_R1_PORT, KEY_R1_PIN);
    DL_GPIO_setPins(KEY_R2_PORT, KEY_R2_PIN);
    DL_GPIO_setPins(KEY_R3_PORT, KEY_R3_PIN);
    DL_GPIO_setPins(KEY_R4_PORT, KEY_R4_PIN);

    /* 选中指定行: 拉低 */
    switch (row) {
    case 0: DL_GPIO_clearPins(KEY_R1_PORT, KEY_R1_PIN); break;
    case 1: DL_GPIO_clearPins(KEY_R2_PORT, KEY_R2_PIN); break;
    case 2: DL_GPIO_clearPins(KEY_R3_PORT, KEY_R3_PIN); break;
    case 3: DL_GPIO_clearPins(KEY_R4_PORT, KEY_R4_PIN); break;
    default: break;
    }
    delay_us(5);   /* 电平稳定延时 */
}

/* 读列, 返回 0~3 表示哪列按下(C1~C4), 0xFF=无键按下 */
static uint8_t read_col(void)
{
    if ((DL_GPIO_readPins(KEY_C1_PORT, KEY_C1_PIN) & KEY_C1_PIN) == 0) return 0;
    if ((DL_GPIO_readPins(KEY_C2_PORT, KEY_C2_PIN) & KEY_C2_PIN) == 0) return 1;
    if ((DL_GPIO_readPins(KEY_C3_PORT, KEY_C3_PIN) & KEY_C3_PIN) == 0) return 2;
    if ((DL_GPIO_readPins(KEY_C4_PORT, KEY_C4_PIN) & KEY_C4_PIN) == 0) return 3;
    return 0xFF;
}

/* 消抖用: 上次原始扫描值 + 当前稳定输出键号
 * 速度模式语义: 按住期间持续返回当前键号, 松开返回 0.
 * 消抖: 连续两次扫描一致才更新稳定值, 过滤抖动. */
static uint8_t last_raw = 0;     /* 上次原始扫描值 */
static uint8_t stable_key = 0;   /* 当前稳定输出的键号 (0=无键) */

/**********************************************************
 *** 扫描键盘 (速度模式: 按住持续返回, 松开返回0)
 *** 返回: 1-8 = 当前按住的键 S1-S8; 0 = 无键按下
 *** 主循环据此发速度命令(有键)或停止命令(无键)
 **********************************************************/
uint8_t Key_Scan(void)
{
    uint8_t col;
    uint8_t key = 0;

    /* 只扫 R1/R2 两行 (S1-S8), R3/R4 不用 */
    for (uint8_t row = 0; row < 2; row++) {
        select_row(row);
        col = read_col();
        if (col != 0xFF) {
            /* 行(row=0~1) × 列(col=0~3) -> 键号 1~8 */
            key = row * 4 + col + 1;
            break;   /* 一次只处理一个键 */
        }
    }

    /* 恢复: 全部行拉高 (降低功耗, 避免误触) */
    select_row(0xFF);

    /* 消抖: 连续两次扫描一致才更新稳定值
     * 本次与上次原始值相同 -> 确认为稳定状态
     * 本次与上次不同 -> 抖动中, 保持原稳定值不变 (给一个扫描周期稳定) */
    if (key == last_raw) {
        stable_key = key;   /* 稳定: 更新 (含 key==0 即松开) */
    }
    last_raw = key;

    return stable_key;
}
