#ifndef __LED_H
#define __LED_H

#include "main.h"
#include "stm32f1xx_hal.h" 
#include "gpio.h"

// 定义 LED 编号
typedef enum
{
	LED1=2,
	LED2,
	LED3,
	LED4
}LED_num;

// 流水灯的流动方向
typedef enum
{
	LED_DIR_FORWARD = 0,   // 顺序：LED1 -> LED2 -> LED3 -> LED4 -> LED1 ...
	LED_DIR_REVERSE = 1    // 逆序：LED4 -> LED3 -> LED2 -> LED1 -> LED4 ...
}LED_Direction;

// 流水灯走一步（换一颗灯）的时间，单位是毫秒。500 表示每 0.5 秒移动一次
#define LED_FLOW_INTERVAL_MS 500

// 闪烁的节奏，单位是毫秒。200 表示亮 200 毫秒、灭 200 毫秒，交替进行
#define LED_BLINK_INTERVAL_MS 200

// LED 的工作模式
typedef enum
{
	LED_MODE_FLOW = 0,     // 流水灯：一颗灯按方向一颗一颗地移动
	LED_MODE_BLINK = 1     // 闪烁：4 颗灯一起亮、一起灭
}LED_Mode;

void LED_On(unsigned char led_num);
void LED_Off(unsigned char led_num);
void LED_Toggle(unsigned char led_num);

/* 下面是"方向可以切换的流水灯"，一共 4 个函数 */
void LED_FlowInit(void);                       // 上电时调用一次：方向复位成顺序，并点亮 LED1
void LED_FlowStep(void);                       // 主循环里反复调用：每 500ms 按当前方向移动一颗灯
void LED_SetFlowDirection(LED_Direction dir);  // 改变流动方向（K2 按下时调用）
LED_Direction LED_GetFlowDirection(void);      // 读取当前方向（调试或测试时用）

/* 模式控制：在"流水灯"和"闪烁"之间切换 */
void LED_SetMode(LED_Mode mode);               // 切换模式（K1 短按=FLOW，长按=BLINK）
LED_Mode LED_GetMode(void);                    // 读取当前模式（调试或测试时用）
void LED_Update(void);                         // 主循环里反复调用：按当前模式决定做什么

/* LED 测试模式：每个函数执行一个完整周期 */
void LED_FS1(void);  // 两灯交替
void LED_FS2(void);  // 跑马灯
void LED_FS3(void);  // 全亮全灭
void LED_FS4(void);  // LED1~LED4 逐个累加点亮后全灭
#endif
