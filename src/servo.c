//#include <zf_common_headfile.h>
//#include "init.h"

//#define SERVO_DUTY(x)         ((float)PWM_DUTY_MAX / (1000.0 / (float)SERVO_FREQ) * (0.5 + (float)(x) / 90.0))
//#define SERVO_FREQ    50
////舵机开环转向 
////void turn (uint8 mid,uint8 k)
////{
////   int temp;

////   temp = Limit_int(-turn_left_duoji_limit, k * (MID_W - mid), turn_right_duoji_limit);
////   pwm_set_duty(duoji_jiao, SERVO_DUTY(duoji_mid ));//+ temp

////}
///*************************************************
//函数功能: 舵机闭环转向
//函数名称: void servo(uint8 mid)
//输入参数: 中线值
//输出参数: 无
//备  注: 将函数放入中断内即可控制舵机闭环转向
//*************************************************/
//void servo(uint8 mid)
//{   float error = mid - MID_W;
//    int servo_value;//舵机打搅
//    int temp1; 
////    servo_value=PID_Normal(&servo_pid,-MID_W+mid,0);
////    //temp1=duoji_mid-Limit_int(-turn_right_duoji_limit, servo_value, turn_left_duoji_limit);
////	  servo_value = Limit_int( -turn_left_duoji_limit, servo_value, turn_right_duoji_limit );
////	  temp1 = SERVO_DUTY(duoji_mid+servo_value);
////    pwm_set_duty(duoji_jiao, temp1);
//	
//	
//	  if(my_abs_float(error) < 4.0f) // 如果偏差小于 6 个像素，认为是在直线上，不动作
//    {
//        error = 0;
//    }
//    
//    servo_value = PID_Normal(&servo_pid, error, 0); // 注意这里的输入逻辑
//    servo_value = Limit_int(-turn_left_duoji_limit, servo_value, turn_right_duoji_limit);
//    temp1 = SERVO_DUTY(duoji_mid + servo_value);
//    pwm_set_duty(duoji_jiao, temp1);
//}
//#include <zf_common_headfile.h>
//#include "init.h"

//#define SERVO_DUTY(x)         ((float)PWM_DUTY_MAX / (1000.0 / (float)SERVO_FREQ) * (0.5 + (float)(x) / 90.0))
//#define SERVO_FREQ    50

//// ========== 参数 ==========
//#define GYRO_DEADZONE_INT    80     // 适当加大死区
//#define GYRO_STABLE_GAIN     0.0f  // 增益先给很小的值 原0.2

//// 低通滤波系数（0~1，越小越平滑，先用0.1）
//#define GYRO_LPF_ALPHA       0.1f

//static float gyro_filtered = 0.0f;  // 滤波后的值（静态变量）

//void servo(uint8 mid)
//{
//    int16 error = (int16)mid - (int16)MID_W;
//    float servo_value_float;
//    int16 servo_value_int;
//    int16 gyro_correction = 0;
//    int temp1;

//    // 低通滤波
//    imu660rb_get_gyro();
//    gyro_filtered = GYRO_LPF_ALPHA * (float)imu660rb_gyro_x
//                  + (1.0f - GYRO_LPF_ALPHA) * gyro_filtered;

//    // 只在直线段（图像偏差小）才启用陀螺仪修正
//    if(error > -8 && error < 8)  // 偏差小 = 直线，阈值可调
//    {
//        if(gyro_filtered > GYRO_DEADZONE_INT || gyro_filtered < -GYRO_DEADZONE_INT)
//        {
//            gyro_correction = (int16)(gyro_filtered * GYRO_STABLE_GAIN);
//        }
//    }
//    // 弯道时 gyro_correction 保持 0，不干扰转向

//    if(error > -6 && error < 6) error = 0;//原4

//    servo_value_float = PID_Normal(&servo_pid, (float)error, 0);
//    servo_value_int = (int16)servo_value_float + gyro_correction;

//    if(servo_value_int >  turn_right_duoji_limit) servo_value_int =  turn_right_duoji_limit;
//    if(servo_value_int < -turn_left_duoji_limit)  servo_value_int = -turn_left_duoji_limit;

//    temp1 = SERVO_DUTY(duoji_mid + servo_value_int);
//    pwm_set_duty(duoji_jiao, temp1);
//}

//闭环舵机，闭环电机
#include <zf_common_headfile.h>
#include "init.h"

#define SERVO_DUTY(x)         ((float)PWM_DUTY_MAX / (1000.0 / (float)SERVO_FREQ) * (0.5 + (float)(x) / 90.0))
#define SERVO_FREQ    50

void servo(uint8 mid)
{
    int16 error = (int16)mid - (int16)MID_W;
    float servo_value_float;
    int16 servo_value_int;
    int temp1;
    int16 gyro_val ;
    imu660rb_get_gyro();

    // 1. 给陀螺仪加一个死区，比如小于 30 的杂波直接忽略，不让舵机跟着抖
    gyro_val = imu660rb_gyro_z;
    if(gyro_val > -30 && gyro_val < 30) gyro_val = 0; 

    servo_value_float = PID_Normal(&servo_pid, error, 0); 

    // 2. 适当微调系数，如果你觉得反应慢，把 0.02 降到 0.015 或 0.01 
    // 这样阻尼就没那么强，视觉 PID 的话语权就更大，反应就快了
    servo_value_int = (int16)servo_value_float + (int16)(gyro_val * 0.005f); 

    if(servo_value_int >  turn_right_duoji_limit) servo_value_int =  turn_right_duoji_limit;
    if(servo_value_int < -turn_left_duoji_limit)  servo_value_int = -turn_left_duoji_limit;

    temp1 = SERVO_DUTY(duoji_mid + servo_value_int);
    pwm_set_duty(duoji_jiao, temp1);
}
