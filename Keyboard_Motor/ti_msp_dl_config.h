/*
 *  ============ ti_msp_dl_config.h ============
 *  Configured MSPM0 DriverLib module declarations
 *
 *  Keyboard_Motor 键盘控制电机项目 - 天猛星 MSPM0G3507
 *    UART_3 (PB12/PB13) -> Emm_V5 步进电机
 *    KEY (4x4矩阵)      行R1-R4=PA28/PA31/PB4/PB5, 列C1-C4=PA9/PA8/PA1/PA0
 *    LED_user (PB22)    状态指示灯 (高有效)
 *    时钟 80MHz
 *
 *  此文件由 SysConfig 生成(基于 empty.syscfg)。可手工编辑, 但用 SysConfig
 *  重新打开 empty.syscfg 生成会覆盖。功能等价。
 */
#ifndef ti_msp_dl_config_h
#define ti_msp_dl_config_h

#define CONFIG_MSPM0G350X

#if defined(__ti_version__) || defined(__TI_COMPILER_VERSION__)
#define SYSCONFIG_WEAK __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define SYSCONFIG_WEAK __weak
#elif defined(__GNUC__)
#define SYSCONFIG_WEAK __attribute__((weak))
#endif

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */

#define POWER_STARTUP_DELAY                                                (16)

/* CPU 时钟 (供 SysTick/delay_cycles 使用)
 * 默认 SYSOSC = 32MHz (不用 PLL, 与天猛星官方 02_2UART 例程一致). */
#define CPUCLK_FREQ                                                     32000000


/* Defines for UART_3 (电机, PB12=TX/PB13=RX, 115200-8N1, RX中断) */
#define UART_3_INST                                                        UART3
#define UART_3_INST_IRQHandler                                  UART3_IRQHandler
#define UART_3_INST_INT_IRQN                                      UART3_INT_IRQn
#define GPIO_UART_3_RX_PORT                                                GPIOB
#define GPIO_UART_3_TX_PORT                                                GPIOB
#define GPIO_UART_3_RX_PIN                                        DL_GPIO_PIN_13
#define GPIO_UART_3_TX_PIN                                        DL_GPIO_PIN_12
#define GPIO_UART_3_IOMUX_RX                                     (IOMUX_PINCM30)
#define GPIO_UART_3_IOMUX_TX                                     (IOMUX_PINCM29)
#define GPIO_UART_3_IOMUX_RX_FUNC                      IOMUX_PINCM30_PF_UART3_RX
#define GPIO_UART_3_IOMUX_TX_FUNC                      IOMUX_PINCM29_PF_UART3_TX
#define UART_3_BAUD_RATE                                                (115200)
/* UART 波特率分频 (BUSCLK=32MHz, 默认 SYSOSC 不用 PLL, 与官方例程一致)
 * 115200 @ 32MHz: 32M/(16*115200)=17.361 -> IBRD=17, FBRD=23 (实际115211, 误差0.006%) */
#define UART_3_IBRD_32_MHZ_115200_BAUD                                      (17)
#define UART_3_FBRD_32_MHZ_115200_BAUD                                      (23)


/* Defines for LED: PB22 用户灯 (PINCM50), 高电平点亮
 * 25diansai1 工程已确认天猛星板载用户灯 PB22 为高电平点亮.
 * (PA7 不再用作指示灯, 不在此配置) */
#define LED_user_PORT                                                   (GPIOB)
#define LED_user_PIN                                            (DL_GPIO_PIN_22)
#define LED_user_IOMUX                                          (IOMUX_PINCM50)

/* Defines for 4x4 矩阵键盘 KEY
 *   行 R1-R4 (输出扫描): R1=PA28, R2=PA31, R3=PB4, R4=PB5
 *   列 C1-C4 (输入上拉): C1=PA9, C2=PA8, C3=PA1, C4=PA0
 * 注意: KEY 引脚跨 GPIOA/GPIOB 两组, 调用时区分 _PORT. */
/* R1 = PA28 (PINCM3) */
#define KEY_R1_PORT                                                     (GPIOA)
#define KEY_R1_PIN                                              (DL_GPIO_PIN_28)
#define KEY_R1_IOMUX                                            (IOMUX_PINCM3)
/* R2 = PA31 (PINCM6) */
#define KEY_R2_PORT                                                     (GPIOA)
#define KEY_R2_PIN                                              (DL_GPIO_PIN_31)
#define KEY_R2_IOMUX                                            (IOMUX_PINCM6)
/* R3 = PB4 (PINCM17) */
#define KEY_R3_PORT                                                     (GPIOB)
#define KEY_R3_PIN                                              (DL_GPIO_PIN_4)
#define KEY_R3_IOMUX                                            (IOMUX_PINCM17)
/* R4 = PB5 (PINCM18) */
#define KEY_R4_PORT                                                     (GPIOB)
#define KEY_R4_PIN                                              (DL_GPIO_PIN_5)
#define KEY_R4_IOMUX                                            (IOMUX_PINCM18)
/* C1 = PA9 (PINCM20) */
#define KEY_C1_PORT                                                     (GPIOA)
#define KEY_C1_PIN                                              (DL_GPIO_PIN_9)
#define KEY_C1_IOMUX                                            (IOMUX_PINCM20)
/* C2 = PA8 (PINCM19) */
#define KEY_C2_PORT                                                     (GPIOA)
#define KEY_C2_PIN                                              (DL_GPIO_PIN_8)
#define KEY_C2_IOMUX                                            (IOMUX_PINCM19)
/* C3 = PA1 (PINCM2)
 * 注意: PA1 是模拟专用引脚, SysConfig 禁止配数字 INPUT (syscfg 里 pin 分配已注释).
 * 此处手写 DL_GPIO_initDigitalInputFeatures 绕过限制, 实测 S7 按键可用.
 * 若改用 SysConfig 接管, 需把 C3 线重接到正规数字脚 (如 PA2/PA3). */
#define KEY_C3_PORT                                                     (GPIOA)
#define KEY_C3_PIN                                              (DL_GPIO_PIN_1)
#define KEY_C3_IOMUX                                            (IOMUX_PINCM2)
/* C4 = PA0 (PINCM1) — 同 C3, PA0 也是模拟引脚, 手写绕过, syscfg 注释. S8 实测可用. */
#define KEY_C4_PORT                                                     (GPIOA)
#define KEY_C4_PIN                                              (DL_GPIO_PIN_0)
#define KEY_C4_IOMUX                                            (IOMUX_PINCM1)

/* clang-format on */

void SYSCFG_DL_init(void);
void SYSCFG_DL_initPower(void);
void SYSCFG_DL_GPIO_init(void);
void SYSCFG_DL_SYSCTL_init(void);
void SYSCFG_DL_UART_3_init(void);

#ifdef __cplusplus
}
#endif
#endif /* ti_msp_dl_config_h */
