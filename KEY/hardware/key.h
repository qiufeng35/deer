#ifndef __KEY_H__
#define __KEY_H__

#include "stm32f1xx_hal.h"
#include "gpio.h"

/* K1 按住的时间超过这个值（单位：毫秒），就算"长按" */
#define KEY_LONG_PRESS_MS 500

/* 比这个时间（单位：毫秒）还短的按压，当成按键抖动，忽略掉 */
#define KEY_BOUNCE_MS 30

/* 扫描 K1、K2（上拉输入，按下时读到低电平），在主循环里反复调用：
 *   K1 短按（按住不到 500ms）-> 顺序流水灯
 *   K1 长按（按住达到 500ms）-> 4 颗灯一起闪烁
 *   K2 按下                  -> 逆序流水灯
 */
void KEY_Scan(void);

#endif /* __KEY_H__ */
