/*
 *  ============ ti_msp_dl_config.c ============
 *  Configured MSPM0 DriverLib module definitions
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
#include "ti_msp_dl_config.h"

/*
 *  ======== SYSCFG_DL_init ========
 *  Perform any initialization needed before using any board APIs
 */
SYSCONFIG_WEAK void SYSCFG_DL_init(void)
{
    SYSCFG_DL_initPower();
    SYSCFG_DL_GPIO_init();
    /* Module-Specific Initializations*/
    SYSCFG_DL_SYSCTL_init();
    SYSCFG_DL_UART_3_init();
}

SYSCONFIG_WEAK void SYSCFG_DL_initPower(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_UART_Main_reset(UART_3_INST);

    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);
    DL_UART_Main_enablePower(UART_3_INST);
    delay_cycles(POWER_STARTUP_DELAY);
}

SYSCONFIG_WEAK void SYSCFG_DL_GPIO_init(void)
{
    /* UART3 (电机) IOMUX: PB12=TX, PB13=RX */
    DL_GPIO_initPeripheralOutputFunction(
        GPIO_UART_3_IOMUX_TX, GPIO_UART_3_IOMUX_TX_FUNC);
    DL_GPIO_initPeripheralInputFunction(
        GPIO_UART_3_IOMUX_RX, GPIO_UART_3_IOMUX_RX_FUNC);

    /* LED2: PB22 用户灯, 推挽输出, 初始低电平(灭, 高有效)
     * (PA7 不再用作指示灯, 保持上电默认高阻态, 不初始化) */
    DL_GPIO_initDigitalOutput(LED_user_IOMUX);
    DL_GPIO_clearPins(LED_user_PORT, LED_user_PIN);
    DL_GPIO_enableOutput(LED_user_PORT, LED_user_PIN);

    /* ===== 4x4 矩阵键盘 =====
     * 行 R1-R4 推挽输出, 初始高(不选中), 扫描时拉低
     * 列 C1-C4 输入上拉, 按下读0 */
    /* R1=PA28, R2=PA31 (GPIOA) */
    DL_GPIO_initDigitalOutput(KEY_R1_IOMUX);
    DL_GPIO_initDigitalOutput(KEY_R2_IOMUX);
    /* R3=PB4, R4=PB5 (GPIOB) */
    DL_GPIO_initDigitalOutput(KEY_R3_IOMUX);
    DL_GPIO_initDigitalOutput(KEY_R4_IOMUX);
    /* C1=PA9, C2=PA8, C3=PA1, C4=PA0 (GPIOA) 输入上拉 */
    DL_GPIO_initDigitalInputFeatures(KEY_C1_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(KEY_C2_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(KEY_C3_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(KEY_C4_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    /* 行初始输出高 (扫描时逐个拉低) */
    DL_GPIO_setPins(KEY_R1_PORT, KEY_R1_PIN);
    DL_GPIO_setPins(KEY_R2_PORT, KEY_R2_PIN);
    DL_GPIO_setPins(KEY_R3_PORT, KEY_R3_PIN);
    DL_GPIO_setPins(KEY_R4_PORT, KEY_R4_PIN);
    DL_GPIO_enableOutput(KEY_R1_PORT, KEY_R1_PIN);
    DL_GPIO_enableOutput(KEY_R2_PORT, KEY_R2_PIN);
    DL_GPIO_enableOutput(KEY_R3_PORT, KEY_R3_PIN);
    DL_GPIO_enableOutput(KEY_R4_PORT, KEY_R4_PIN);
}


/* 默认时钟配置 (照搬天猛星官方 02_2UART 例程):
 *   SYSOSC = 32MHz (SYSOSC_FREQ_BASE), 不用 PLL, MCLK=BUSCLK=ULPCLK=32MHz.
 *   UART3 用 BUSCLK=32MHz, IBRD=17/FBRD=23 -> 115200 (实测帧正确, 电机响应).
 *   之前手写 PLL(qDiv=4)+按40MHz算波特率(IBRD=21/45)导致波特率错, 电机不动.
 *   现弃用 PLL, 改默认 32MHz, 与官方例程一致. */
SYSCONFIG_WEAK void SYSCFG_DL_SYSCTL_init(void)
{
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);
    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_disableHFXT();
    DL_SYSCTL_disableSYSPLL();
    DL_SYSCTL_setULPCLKDivider(DL_SYSCTL_ULPCLK_DIV_1);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);
}


/*
 * UART 时钟: BUSCLK = 32MHz (默认 SYSOSC, 不用 PLL, 与官方例程一致)
 * 115200 baud @ 32MHz: IBRD=17, FBRD=23 (实际 115211, 误差<0.01%)
 */
static const DL_UART_Main_ClockConfig gUART_3ClockConfig = {
    .clockSel    = DL_UART_MAIN_CLOCK_BUSCLK,
    .divideRatio = DL_UART_MAIN_CLOCK_DIVIDE_RATIO_1
};

static const DL_UART_Main_Config gUART_3Config = {
    .mode        = DL_UART_MAIN_MODE_NORMAL,
    .direction   = DL_UART_MAIN_DIRECTION_TX_RX,
    .flowControl = DL_UART_MAIN_FLOW_CONTROL_NONE,
    .parity      = DL_UART_MAIN_PARITY_NONE,
    .wordLength  = DL_UART_MAIN_WORD_LENGTH_8_BITS,
    .stopBits    = DL_UART_MAIN_STOP_BITS_ONE
};

SYSCONFIG_WEAK void SYSCFG_DL_UART_3_init(void)
{
    DL_UART_Main_setClockConfig(UART_3_INST, (DL_UART_Main_ClockConfig *) &gUART_3ClockConfig);

    DL_UART_Main_init(UART_3_INST, (DL_UART_Main_Config *) &gUART_3Config);
    /*
     * Configure baud rate by setting oversampling and baud rate divisors.
     *  Target baud rate: 115200
     *  Actual baud rate: 115190.78
     */
    DL_UART_Main_setOversampling(UART_3_INST, DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_Main_setBaudRateDivisor(UART_3_INST, UART_3_IBRD_32_MHZ_115200_BAUD, UART_3_FBRD_32_MHZ_115200_BAUD);

    /* Configure Interrupts: 仅 RX (不依赖 RTOUT 硬件超时) */
    DL_UART_Main_enableInterrupt(UART_3_INST,
                                 DL_UART_MAIN_INTERRUPT_RX);

    DL_UART_Main_enable(UART_3_INST);
}
