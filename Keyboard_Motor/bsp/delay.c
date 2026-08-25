#include "delay.h"
#include "clock.h"   /* tick_ms (SysTick 1ms 中断累加) */

/**********************************************************
 *** delay_ms : 基于 tick_ms 的非阻塞 ms 延时 (等价 STM32 HAL_Delay)
 ***   依赖 SysTick_Init() 已运行 (tick_ms 在 SysTick_Handler 中累加).
 ***   用 tick_ms - start 比较, 天然处理 tick_ms 回绕 (uint32 溢出).
 **********************************************************/
void delay_ms(uint32_t ms)
{
    volatile unsigned long start = tick_ms;
    while ((tick_ms - start) < ms) { ; }
}

/**********************************************************
 *** delay_us : 忙等 us 延时 (基于 CPU 周期 delay_cycles, 不依赖 SysTick)
 ***   80MHz: 1us = 80 cycles. 精度有限, 仅供短延时 (如时序脉冲).
 ***   可在 SysTick_Init 之前调用.
 **********************************************************/
void delay_us(uint32_t us)
{
    delay_cycles(us * (CPUCLK_FREQ / 1000000));
}
