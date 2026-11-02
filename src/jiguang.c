
#include <zf_common_headfile.h>

#define JIGUANG_PIT (TIM6_PIT)
extern volatile uint16 laser_timer;
static vuint8  pit_flag  = 0;
static uint16  pit_count = 0;

uint16 jiguang_distance = 0;

void jiguang_init(void)
{
    while (dl1b_init())
    {
        system_delay_ms(100);
    }
    pit_ms_init(JIGUANG_PIT, 5, jiguang_pit_handler);
}

void jiguang_pit_handler(void)
{
    if (0 == pit_count % 10)
    {
        dl1b_get_distance();
    }

    if (dl1b_finsh_flag)
    {
        jiguang_distance = dl1b_distance_mm;
        dl1b_finsh_flag  = 0;
    }

    pit_count = (pit_count == 995) ? 0 : (pit_count + 5);
}

void laser_pit_handler(void)
{
    if (laser_timer > 0) {
        laser_timer--;
    } else {
        // 关闭所有激光口
        gpio_low(IO_P67);  // 中间
        gpio_low(IO_P01);  // 左1
        gpio_low(IO_P02);  // 左2
        gpio_low(IO_P03);  // 右2
        gpio_low(IO_P06);  // 右1
    }
}


