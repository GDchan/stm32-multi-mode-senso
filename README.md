# STM32F103 多模式传感器终端

基于 STM32F103C8T6 + 标准外设库开发的多模式传感器采集系统，支持电位器调光、温度报警、光敏夜灯三种模式，带独立看门狗与串口状态日志。

## 功能

- **模式 1 电位器调光**：读取电位器 ADC 值，映射为 PWM 占空比控制 LED 亮度
- **模式 2 温度报警**：热敏电阻测温，超过阈值蜂鸣器报警
- **模式 3 光敏夜灯**：光照低于阈值时自动点亮 LED
- **独立看门狗**：程序卡死时自动复位，可通过串口确认复位来源
- **串口日志**：每 2 秒输出三路数值与当前模式

## 硬件

| 模块 | 引脚 | 说明 |
|---|---|---|
| 电位器 | PA4 | ADC 通道 4 |
| 热敏电阻 | PA5 | ADC 通道 5 |
| 光敏电阻 | PA6 | ADC 通道 6 |
| 调光 LED | PA1 | TIM2_CH2，PWM 输出 |
| 蜂鸣器 | PA2 | 推挽输出 |
| 夜灯 LED | PA3 | 推挽输出 |
| 模式切换键 | PA0 | 外部下拉，按下为高 |
| 看门狗测试键 | PC13 | 外部下拉，按下为高 |
| 串口 TX | PA9 | 115200-8-N-1 |

开发板：野火 STM32F103C8T6 核心板

![接线图][docs/wiring.jpg]

## 开发环境

- 芯片：STM32F103C8T6
- 固件库：STM32 标准外设库 SPL V3.5.0
- IDE：STM32CubeIDE + GCC

## 编译与烧录

1. 用 STM32CubeIDE 导入工程（`File → Open Projects from File System`）
2. `Project → Build All` 编译
3. 连接 ST-Link，`Run → Debug` 下载
4. 串口助手连接 PA9，波特率 115200，8 数据位 / 无校验 / 1 停止位

> 若编译报找不到头文件，检查工程属性里的 include 路径是否指向本机的
> `Inc/CMSIS`、`Inc/STM32F10x_StdPeriph_Driver` 与 `Src/BSP` 目录。

## 使用说明

- 按 PA0 键循环切换三种模式，日志会显示当前模式
- 模式 2 下温度超过 30℃ 蜂鸣器响，降到阈值以下自动停
- 模式 3 下光照低于 30% 时夜灯亮
- 长按 PC13 超过 1 秒，程序阻塞不喂狗，1 秒后看门狗复位，
  串口打印"系统卡死，正在重启"

## 目录结构

```
Inc/
├── CMSIS/                        ARM Cortex-M3 内核支持 + STM32F10x 设备头文件
└── STM32F10x_StdPeriph_Driver/   ST 标准外设库（ADC / GPIO / TIM / USART / IWDG 等）
Src/
├── main.c                        主循环调度 + TIM3 中断服务函数
├── syscalls.c                    CubeIDE 自动生成，newlib 系统调用桩
├── sysmem.c                      CubeIDE 自动生成，堆内存管理（_sbrk）
└── BSP/                          本项目外设驱动
    ├── ADC.c / ADC.h             三通道采集、滑动平均、数值换算
    ├── KEY.c / KEY.h             按键扫描与事件
    ├── LED.c / LED.h             PWM 输出与 GPIO
    ├── Timer.c / Timer.h         1ms 时基
    ├── UART.c / UART.h           串口状态日志
    ├── IWDG.c / IWDG.h           独立看门狗
    ├── NTC.h                     热敏电阻阻温表
    └── Delay.c / Delay.h         忙等延时
```

> 开发过程记录：[](链接)

## 已知问题
- 光敏部分输出的是相对明暗百分比，不是绝对照度值。GL5516 数据手册
  未提供完整阻值-照度曲线，无法做精确换算。
- 忙等延时系数在 Debug(-O0) 下标定，切换到 Release 后需要重新标定。
## 致谢
- ST 官方标准外设库例程
- 江协科技 STM32 教程（中断、看门狗部分参考）
- 南京时恒 MF52A103F3950 规格书（阻温表数据来源）

