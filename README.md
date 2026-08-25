# Keyboard_Motor - 4×4 矩阵键盘控制双轴云台电机 (天猛星 MSPM0G3507)

独立工程，用 4×4 矩阵键盘的 8 个按键控制 Emm_V5 双轴闭环步进电机转固定角度。
电机通信已用方案B（USB转TTL）验证通过，本工程直接接天猛星实测。

## 硬件接线

### 电机（UART3）
| 天猛星 | 外设 | 连接 | 说明 |
|---|---|---|---|
| **PB12** | UART3 TX | -> Emm_V5 电机 RAH(RX) | 电机命令发送 |
| **PB13** | UART3 RX | <- Emm_V5 电机 TBL(TX) | 电机响应接收（可选） |
| GND | - | 共地 | 电机/K230/天猛星共地 |

- 电机：Emm_V5 闭环步进（张大头 X42S/Y42），双轴并联 UART3 总线，ID 1=X轴, 2=Y轴, 115200-8N1
- 电机独立供电 12V/24V -> V+/GND

### 4×4 矩阵键盘
| 键盘 | 天猛星 | 方向 |
|---|---|---|
| R1（行） | PA28 | 输出扫描 |
| R2（行） | PA31 | 输出扫描 |
| R3（行） | PB4 | 输出扫描（预留不用） |
| R4（行） | PB5 | 输出扫描（预留不用） |
| C1（列） | PA9 | 输入上拉 |
| C2（列） | PA8 | 输入上拉 |
| C3（列） | PA1 | 输入上拉 |
| C4（列） | PA0 | 输入上拉 |

### LED
| 天猛星 | 外设 | 说明 |
|---|---|---|
| PA7 | GPIO 输出 | 状态指示 |

## 按键功能

只用 S1-S8（R1/R2 两行 × C1-C4 四列），R3/R4 行预留。

| 键 | 行×列 | 电机 | 动作 | 脉冲 |
|---|---|---|---|---|
| S1 | R1×C1 | 电机1 (X轴) | 左转 15° | +133 |
| S2 | R1×C2 | 电机1 (X轴) | 右转 15° | -133 |
| S3 | R1×C3 | 电机1 (X轴) | 左转 30° | +267 |
| S4 | R1×C4 | 电机1 (X轴) | 右转 30° | -267 |
| S5 | R2×C1 | 电机2 (Y轴) | 上转 15° | +133 |
| S6 | R2×C2 | 电机2 (Y轴) | 下转 15° | -133 |
| S7 | R2×C3 | 电机2 (Y轴) | 上转 30° | +267 |
| S8 | R2×C4 | 电机2 (Y轴) | 下转 30° | -267 |

- 16 细分，3200 脉冲/圈 = 360°
- 电机1 = X轴（左右，底座水平旋转）
- 电机2 = Y轴（上下，俯仰），物理上叠在电机1上，随电机1在X轴移动

## 工程结构

```
Keyboard_Motor\
├── empty.syscfg              SysConfig (UART3 + KEY键盘 + LED + 80MHz)
├── ti_msp_dl_config.c/.h     手写版 (基于 empty.syscfg)
├── main.c                    主程序 (键盘扫描 + 按键映射 + 电机命令)
├── bsp\
│   ├── Emm_V5.c/.h           Emm_V5 步进电机驱动库 (官方MSPM0例程)
│   ├── usart.c/.h            UART3 电机命令发送/接收 (阻塞式)
│   ├── fifo.c/.h             接收 FIFO
│   ├── delay.c/.h            delay_ms (基于 tick_ms) / delay_us
│   ├── clock.c/.h            SysTick 1ms + tick_ms
│   ├── keypad.c/.h           4x4 矩阵键盘扫描 (行出列入, 返回键号1-8)
│   └── motor.c/.h            电机控制封装 (使能+QPos位置模式+按脉冲/角度移动)
├── ti\                       driverlib dl_*.c
├── source\                   MSPM0 SDK source 副本 (79MB, driverlib头文件)
└── keil\
    ├── empty_LP_MSPM0G3507_nortos_keil.uvprojx   Keil 工程
    ├── startup_mspm0g350x_uvision.s
    └── mspm0g3507.sct
```

## 编译

1. Keil 打开 `keil/empty_LP_MSPM0G3507_nortos_keil.uvprojx`
2. **直接 Build（F7）** -- BeforeMake 已禁用，用手写 ti_msp_dl_config.c/.h
3. 烧录（J-Link / XDS110 / BSL）

> 工程自带 `source/`（79MB SDK 副本），IncludePath 用相对路径，**不依赖 SDK 安装路径**。

## 上电流程

1. LED 闪 3 次 → 初始化
2. 电机使能锁定（手转不动，有保持力）→ Motor_Init 完成
3. LED 快闪 2 次 → 进入键盘扫描
4. 按键 → 电机转指定角度，LED 亮一下反馈

## 调参（均在 `bsp/motor.h`）

| 宏 | 值 | 含义 |
|---|---|---|
| `MOTOR_X_INVERT` | 0 | 电机1方向反转（方向不对改 1） |
| `MOTOR_Y_INVERT` | 0 | 电机2方向反转（方向不对改 1） |
| `MOTOR_PULSE_15DEG` | 133 | 15° 脉冲数（细分不对改） |
| `MOTOR_PULSE_30DEG` | 267 | 30° 脉冲数 |
| `MOTOR_Y_FOC_mA` | 3000 | Y轴闭环电流（抬不动加到 5000，发烫再降） |
| `MOTOR_X_SPEED_RPM` | 300 | X轴位置模式速度 |
| `MOTOR_Y_SPEED_RPM` | 120 | Y轴位置模式速度（重载低速） |
| `MOTOR_Y_ACC` | 60 | Y轴加速度（重载低加速抗重力矩） |

## 关键设计

- **"按一下走一步"语义**：`keypad.c` 用 `last_key` 记忆，按住不重复发命令（位置模式电机走完脉冲自动停，按住重复发会让电机重复走）。松开再按才发新命令。
- **消抖**：检测到键后延时 20ms 再扫描确认，过滤抖动。
- **Y轴重载补偿**：电机2 切闭环FOC + 低速(120RPM) + 低加速(60) + 电流3000mA，抗重力矩。
- **方向反转**：`motor.h` 的 `MOTOR_X_INVERT`/`MOTOR_Y_INVERT`，实测方向不对改 1 即可。

## 故障排查

| 现象 | 可能原因 | 排查 |
|---|---|---|
| 电机不动 | 接线/供电/ID | PB12->RAH, GND共地, 电机ID=1/2, 电机独立供电 |
| 电机方向反 | 脉冲极性 | 改 `MOTOR_X_INVERT`/`MOTOR_Y_INVERT` = 1 |
| LED 不亮/常亮 | PA7 高低有效 | 交换 main.c 的 `LED_ON`/`LED_OFF` 宏 |
| 角度不准 | 细分不对 | 改 `MOTOR_PULSE_15DEG`/`30DEG` 或确认电机细分=16 |
| Y轴抬不动 | 扭矩不足 | `MOTOR_Y_FOC_mA` 加到 5000 |
| 按键无反应 | 接线/扫描方向 | 行R1-R4接PA28/31/PB4/5, 列C1-C4接PA9/8/1/0 |
