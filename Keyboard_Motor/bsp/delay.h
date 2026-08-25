#ifndef __DELAY_H
#define __DELAY_H

#include "ti_msp_dl_config.h"

/**********************************************************
 *** 延时接口 (移植自张大头例程, 改为基于 SysTick tick_ms)
 ***   delay_ms : 非阻塞 ms 延时 (等价 STM32 HAL_Delay), 依赖 tick_ms
 ***   delay_us : 忙等 us 延时 (基于 CPU 周期, 不依赖 SysTick)
 ***
 *** 注意: delay_ms 必须在 SysTick_Init() 之后调用 (tick_ms 由 SysTick
 ***       中断累加). main.c 顺序: SYSCFG_DL_init() -> SysTick_Init() -> delay_ms
 ***
 *** Emm_V5 库内部调用 delay_ms, 故 Motor_Init 等需在 SysTick_Init 之后.
 **********************************************************/

void delay_ms(uint32_t ms);
void delay_us(uint32_t us);

#endif /* __DELAY_H */
