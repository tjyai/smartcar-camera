#ifndef __CONTROL_H_
#define __CONTROL_H_
#include <zf_common_headfile.h>

#define turn_left_duoji_limit 11   //向左舵机限幅（应该可改
#define turn_right_duoji_limit 25   //向右舵机限幅
//#define duoji_mid 98            //舵机中值
//#define pid_limit 6000              //后轮电机pwm限幅 c2000
//闭环舵机闭环电机
#define duoji_mid 90            //舵机中值
#define pid_limit 5000           
//引脚号
#define PWM_L      PWMD_CH1_P50
#define PWM_R      PWMD_CH3_P52
#define GPIO_L     IO_P51
#define GPIO_R     IO_P53
#define duoji_jiao PWME_CH1P_PA0

//void turn (uint8 mid,uint8 k);
void servo(uint8 mid);
void imu660rb_stable_init(void);
#endif
