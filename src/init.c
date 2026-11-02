#include <zf_common_headfile.h>

/* ============================================================================
 * 外设初始化
 * 顺序：摄像头 → 舵机 → 电机/按键/激光 GPIO → 电机 PWM → 屏幕
 *       → 激光测距 → 编码器 → 陀螺仪 → 定时器中断
 * ========================================================================== */
void init(void)
{
    mt9v03x_init();                                  /* 摄像头 */

    /* 舵机：PWM 50Hz，中值 duoji_mid */
    pwm_init(PWME_CH1P_PA0, 50, duoji_mid);

    /* GPIO：电机方向、启动按键、5 路激光 */
    gpio_init(GPIO_L, GPO, 1, GPO_PUSH_PULL);        /* 左电机方向 */
    gpio_init(GPIO_R, GPO, 1, GPO_PUSH_PULL);        /* 右电机方向 */
    gpio_init(IO_PB2, GPI, 0, GPI_PULL_UP);          /* 启动按键，上拉输入 */
    gpio_init(IO_P67, GPO, GPIO_LOW, GPO_PUSH_PULL); /* 中间激光 */
    gpio_init(IO_P01, GPO, GPIO_LOW, GPO_PUSH_PULL); /* 左1 激光 */
    gpio_init(IO_P02, GPO, GPIO_LOW, GPO_PUSH_PULL); /* 左2 激光 */
    gpio_init(IO_P03, GPO, GPIO_LOW, GPO_PUSH_PULL); /* 右2 激光 */
    gpio_init(IO_P06, GPO, GPIO_LOW, GPO_PUSH_PULL); /* 右1 激光 */

    /* 电机 PWM：10kHz */
    pwm_init(PWM_L, 10000, 0);
    pwm_init(PWM_R, 10000, 0);

    ips200_init();                                   /* 屏幕，调参时显示状态量 */

    jiguang_init();                                  /* DL1B 激光测距 */
    Encoder_Init();                                  /* 正交编码器 */
    imu660rb_init();                                 /* 陀螺仪，Z 轴参与转向阻尼 */

    /* 定时器中断：20ms 控制周期、5ms 激光节拍 */
    pit_ms_init(TIM1_PIT, 20, pit_handler);
    pit_ms_init(TIM2_PIT, 5, laser_pit_handler);
}
