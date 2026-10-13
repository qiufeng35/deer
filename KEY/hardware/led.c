#include "led.h"
#include "main.h"

void LED_On(unsigned char led_num)
{
    switch(led_num)
    {
        case LED1: HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET); break;
        case LED2: HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET); break;
        case LED3: HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_RESET); break;
        case LED4: HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_RESET); break;
    }
}

void LED_Off(unsigned char led_num)
{
    switch(led_num)
    {
        case LED1: HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET); break;
        case LED2: HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET); break;
        case LED3: HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, GPIO_PIN_SET); break;
        case LED4: HAL_GPIO_WritePin(LED4_GPIO_Port, LED4_Pin, GPIO_PIN_SET); break;
    }
}

void LED_Toggle(unsigned char led_num)
{
    switch(led_num)
    {
        case LED1: HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); break;
        case LED2: HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin); break;
        case LED3: HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin); break;
        case LED4: HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin); break;
    }
}

/* 关闭全部 LED */
static void LED_All_Off(void)
{
    LED_Off(LED1);
    LED_Off(LED2);
    LED_Off(LED3);
    LED_Off(LED4);
}
/* ==================== 可切换方向的流水灯 ====================
 * 思路（很简单，就是"排队走"）：
 *   1. 把 4 颗灯的编号放进一个数组 led_table 里；
 *   2. 用变量 led_index 记住"现在亮的是数组里的第几个"，取值 0、1、2、3；
 *   3. 主循环不断调用 LED_FlowStep()，每过 500ms 就让 led_index 加 1（顺序）
 *      或者减 1（逆序），然后只点亮 led_index 对应的那一颗灯、其余全灭，
 *      看上去就是一颗灯在流动；
 *   4. K1/K2 只改变方向，不改变当前位置，所以切换方向时灯不会灭、也不会跳。
 */

/* 4 颗灯的编号。led_table[0] 是 LED1，led_table[3] 是 LED4 */
static const unsigned char led_table[4] = {LED1, LED2, LED3, LED4};

static unsigned char led_index = 0;                /* 当前亮的是 led_table 里的第几个：0 ~ 3 */
static LED_Direction flow_direction = LED_DIR_FORWARD;  /* 当前方向。上电时默认"顺序" */
static unsigned int last_move_time = 0;            /* 上一次移动的时刻，单位：毫秒 */

static LED_Mode led_mode = LED_MODE_FLOW;          /* 当前模式。上电时默认"流水灯" */
static unsigned char blink_is_on = 0;              /* 闪烁现在处于"亮"(1)还是"灭"(0) */
static unsigned int last_blink_time = 0;           /* 上一次闪烁切换的时刻，单位：毫秒 */

/* 上电时调用一次：模式回到"流水灯"、方向回到"顺序"，并马上点亮第一颗灯 */
void LED_FlowInit(void)
{
    led_mode = LED_MODE_FLOW;           /* 模式 = 流水灯（上电后默认就是它） */
    flow_direction = LED_DIR_FORWARD;   /* 方向 = 顺序（满足"上电后默认顺序"） */
    led_index = 0;                      /* 从第一颗灯开始 */
    blink_is_on = 0;                    /* 闪烁状态清零 */
    last_move_time = HAL_GetTick();     /* 记下当前时刻 */
    LED_All_Off();                      /* 先把 4 颗灯全部熄灭 */
    LED_On(led_table[0]);               /* 点亮第一颗灯 */
}

/* 改变流动方向。按键扫描函数在 K2 按下时会调用它 */
void LED_SetFlowDirection(LED_Direction dir)
{
    /* 只接受两个合法的方向值，别的值一律忽略，防止写错 */
    if (dir == LED_DIR_FORWARD || dir == LED_DIR_REVERSE)
    {
        flow_direction = dir;   /* 只改方向，不动当前灯的位置 */
    }
}

/* 读取当前方向 */
LED_Direction LED_GetFlowDirection(void)
{
    return flow_direction;
}

/* 主循环里反复调用本函数，它自己计时：每满 500ms 才让灯走一步。
   因为不用 HAL_Delay() 死等，主循环不会卡住，按键随时都能被扫描到 */
void LED_FlowStep(void)
{
    unsigned int now_time = HAL_GetTick();   /* 现在的时刻（毫秒） */

    /* 距离上一次移动还不到 500ms，什么也不做，直接返回 */
    if ((now_time - last_move_time) < LED_FLOW_INTERVAL_MS)
    {
        return;
    }

    last_move_time = now_time;   /* 记下这次移动的时刻 */

    if (flow_direction == LED_DIR_FORWARD)
    {
        /* 顺序：位置往后挪一个 */
        led_index = led_index + 1;      /* 0->1->2->3 */
        if (led_index >= 4)
        {
            led_index = 0;              /* 走过最后一颗，回到第一颗 */
        }
    }
    else
    {
        /* 逆序：位置往前退一个 */
        if (led_index == 0)
        {
            led_index = 3;              /* 已经在第一颗，退到最后一颗 */
        }
        else
        {
            led_index = led_index - 1;  /* 3->2->1->0 */
        }
    }

    LED_All_Off();                  /* 先把 4 颗灯全部熄灭 */
    LED_On(led_table[led_index]);   /* 再点亮当前这一颗 */
}

/* ==================== 两种模式：流水灯 / 闪烁 ====================
 * 模式由按键决定：
 *   K1 短按 -> LED_MODE_FLOW ，即上面那套流水灯；
 *   K1 长按 -> LED_MODE_BLINK，即 4 颗灯一起亮、一起灭地闪烁。
 * 主循环只要调用 LED_Update()，它会看当前是哪种模式，然后去做对应的事。
 */

/* 切换模式。模式没有变化时什么都不做，避免把流水灯的节奏打乱 */
void LED_SetMode(LED_Mode mode)
{
    /* 只接受两个合法的模式值，别的值一律忽略 */
    if (mode != LED_MODE_FLOW && mode != LED_MODE_BLINK)
    {
        return;
    }

    /* 本来就是这个模式，不用做任何事 */
    if (mode == led_mode)
    {
        return;
    }

    led_mode = mode;

    if (led_mode == LED_MODE_BLINK)
    {
        /* 进入闪烁：先让 4 颗灯一起亮，给出立刻就能看到的反馈 */
        blink_is_on = 1;
        last_blink_time = HAL_GetTick();
        for (unsigned char i = 0; i < 4; i++)
        {
            LED_On(led_table[i]);
        }
    }
    else
    {
        /* 回到流水灯：只点亮当前这一颗灯，并重新开始 500ms 计时 */
        LED_All_Off();
        LED_On(led_table[led_index]);
        last_move_time = HAL_GetTick();
    }
}

/* 读取当前模式 */
LED_Mode LED_GetMode(void)
{
    return led_mode;
}

/* 闪烁一步：每 LED_BLINK_INTERVAL_MS，让 4 颗灯一起翻转（亮<->灭） */
static void LED_BlinkStep(void)
{
    unsigned int now_time = HAL_GetTick();   /* 现在的时刻（毫秒） */

    /* 距离上一次切换还不到 200ms，什么也不做，直接返回 */
    if ((now_time - last_blink_time) < LED_BLINK_INTERVAL_MS)
    {
        return;
    }

    last_blink_time = now_time;   /* 记下这次切换的时刻 */

    if (blink_is_on == 0)
    {
        blink_is_on = 1;          /* 原来是灭的，这次改成亮 */
    }
    else
    {
        blink_is_on = 0;          /* 原来是亮的，这次改成灭 */
    }

    for (unsigned char i = 0; i < 4; i++)
    {
        if (blink_is_on == 1)
        {
            LED_On(led_table[i]);    /* 4 颗灯一起亮 */
        }
        else
        {
            LED_Off(led_table[i]);   /* 4 颗灯一起灭 */
        }
    }
}

/* 主循环里反复调用：按当前模式决定做什么 */
void LED_Update(void)
{
    if (led_mode == LED_MODE_FLOW)
    {
        LED_FlowStep();     /* 流水灯：每 500ms 移动一颗灯 */
    }
    else
    {
        LED_BlinkStep();    /* 闪烁：每 200ms 让 4 颗灯一起亮或一起灭 */
    }
}
