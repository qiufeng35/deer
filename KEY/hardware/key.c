#include "key.h"
#include "led.h"

/* ================= K1、K2 按键扫描 =================
 *
 * 硬件说明（见 gpio.c 里的配置）：
 *   K1、K2 都是"上拉输入"：按键的一端接单片机引脚，另一端接 GND。
 *   没有按下时：引脚被内部上拉电阻拉到高电平，读回来是 GPIO_PIN_SET；
 *   按下时    ：引脚被按键连到 GND，读回来是 GPIO_PIN_RESET（低电平）。
 *
 * 要实现的功能：
 *   K1 短按（按住不到 500ms）-> 顺序流水灯；
 *   K1 长按（按住达到 500ms）-> 4 颗灯一起闪烁（按住到 0.5 秒就立刻开始闪，不用等松手）；
 *   K2 按下                  -> 逆序流水灯；
 *   长按之后松手，不会又被当成一次短按。
 *
 * 两个时间常量（在 key.h 里定义）：
 *   KEY_LONG_PRESS_MS = 500  ：按住超过 500ms 就算长按
 *   KEY_BOUNCE_MS     = 30   ：按住不到 30ms 的"按压"当成抖动，忽略掉
 *
 * 本函数要在主循环里反复调用。主循环转一圈只要几微秒，
 * 所以按键按下和松手的时刻都能被及时捕捉到。
 */
void KEY_Scan(void)
{
    /* ---- 第 1 部分：用 static 变量记住上一次扫描的结果 ---- */
    /* 加了 static，函数返回后变量的值不会丢失，下次调用时还能看到。
       0 表示"没有按下"，1 表示"按下了"。
       上面两个初值故意写成 1（意思是"上一次是按下状态"），
       这样开机瞬间手正好按在键上时，不会被误当成一次"新的按下"。 */
    static unsigned char key1_last_state = 1;   /* 上一次 K1 的状态：0=没按下，1=按下 */
    static unsigned char key2_last_state = 1;   /* 上一次 K2 的状态：0=没按下，1=按下 */

    static unsigned int  key1_press_time = 0;   /* K1 这一次是按什么时候按下的（毫秒） */
    static unsigned char key1_long_done  = 0;   /* K1 这一次按住是否已经按"长按"处理过 */

    unsigned char key1_now_state;   /* 这一次 K1 的状态：0=没按下，1=按下 */
    unsigned char key2_now_state;   /* 这一次 K2 的状态：0=没按下，1=按下 */

    /* ---- 第 2 部分：读取 K1、K2 现在的状态 ---- */
    if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
    {
        key1_now_state = 1;         /* 读到低电平，说明 K1 正被按下 */
    }
    else
    {
        key1_now_state = 0;         /* 读到高电平，说明 K1 没有被按下 */
    }

    if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
    {
        key2_now_state = 1;         /* 读到低电平，说明 K2 正被按下 */
    }
    else
    {
        key2_now_state = 0;         /* 读到高电平，说明 K2 没有被按下 */
    }

    /* ---- 第 3 部分：K1 用"按住多久"来区分短按和长按 ---- */
    if (key1_now_state == 1)
    {
        /* K1 现在按着。如果上一次是"没按下"，说明刚刚按下，记下按下时刻 */
        if (key1_last_state == 0)
        {
            key1_press_time = HAL_GetTick();    /* 记下按下的时刻 */
            key1_long_done = 0;                 /* 这一次还没有按长按处理过 */
        }

        /* 按住的时间达到 500ms，而且这一次还没处理过长按 -> 按长按处理 */
        if (key1_long_done == 0 && (HAL_GetTick() - key1_press_time) >= KEY_LONG_PRESS_MS)
        {
            LED_SetMode(LED_MODE_BLINK);        /* 长按：4 颗灯一起闪烁 */
            key1_long_done = 1;                 /* 做个记号，一直按着也不会重复触发 */
        }
    }
    else
    {
        /* K1 现在松开了。若这次按住没走长按那条路，而且按住时间超过了抖动时间，
           就算一次"短按"。（太短的按压是抖动，忽略掉） */
        if (key1_last_state == 1 && key1_long_done == 0 &&
            (HAL_GetTick() - key1_press_time) >= KEY_BOUNCE_MS)
        {
            LED_SetMode(LED_MODE_FLOW);              /* 短按：回到流水灯 */
            LED_SetFlowDirection(LED_DIR_FORWARD);   /* 方向 = 顺序 */
        }
    }

    /* ---- 第 4 部分：K2 按下就切换成"逆序"（和原来的行为一样） ---- */
    /* 这一次按下了(1)、上一次没按下(0)，说明是刚刚按下 */
    if (key2_now_state == 1 && key2_last_state == 0)
    {
        LED_SetFlowDirection(LED_DIR_REVERSE);       /* K2：逆序 */
    }

    /* ---- 第 5 部分：把"这一次"记成"上一次"，留给下一次调用比较 ---- */
    key1_last_state = key1_now_state;
    key2_last_state = key2_now_state;
}
