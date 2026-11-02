#include "pid.h"
#include "zf_common_headfile.h"
#include <math.h>




//PID servo_pid=PID_CREATE(0,0,0,0.8);
//PID servo_pid = PID_CREATE(0, 0, 0, 0.2);

// 定义这些变量，或者从其他地方引入
float PID_W = 94.0f;          // 根据你的应用设置初始值
//float final_mid_line = 0.0f; // 根据你的应用设置初始值

// 动态pid增益参数
float Kp_base = 0.34;
float Kd_base = 3.0;

//舵机位置pid初始参数
//PID servo_pid = PID_CREATE(0.7,0,0.8,0.05);//p:0.7,d:0.8不晃
//闭环舵机闭环电机
PID servo_pid = PID_CREATE(1,0,0,0.05);
////电机增量式pid参数
//PID motor_pid_l = PID_CREATE(0.7,0.05,0,1);//_kp,_ki,_kd,_low_pass 原0，0.06，0，1
//PID motor_pid_r = PID_CREATE(0.7,0.05,0,1);
//闭环舵机闭环电机
PID motor_pid_l = PID_CREATE(5,0.5,0,1);
PID motor_pid_r = PID_CREATE(5,0.5,0,1);
// 绝对值函数
float my_abs_float(float x)
{
    return (x > 0) ? x : -x;
}

/**************************PID函数定义**************************/

/**
 * 动态设置伺服PID参数
 */
//void dynamic_pid_value_set(void)
//{
//    // 二次KP（原注释：最大幅值 0.05）
//     //float error = PID_W - final_mid_line;
//	
//     
////    servo_pid.Kp = Kp_base + error *  error * 0.0015f;
////    if(servo_pid.Kp > 0.76f) servo_pid.Kp = 0.76f;
////    servo_pid.Ki = 0.0f;
////    servo_pid.Kd = 0;//Kd_base+ error * 0.04f;
//	   float error = my_abs_float(MID_W - final_mid_line);
//	    if(error < 10) {
//        servo_pid.Kp = 0.4f;  // 降低直道 Kp
//        //servo_pid.Kd = 0.5f;   // 降低直道 Kd，减少噪声震动
//    } 
//    else if(error < 20){
//        // 随着误差增大，线性或二次方增加 Kp
//        servo_pid.Kp = 0.4f + (error-10) * 0.03f; 
//        //servo_pid.Kd = 3.5f + error * 0.05f; // 弯道需要更大的 Kd 来抑制冲出去的惯性
//    }
//    else{
//    // 限制最大值，防止舵机打死
//    if(servo_pid.Kp > 2f) servo_pid.Kp = 2f;
//		}
//}
// ===================================================================
//                 修正和改进后的动态PID函数
// ===================================================================
void dynamic_pid_value_set(void)
{
    // 1. 计算当前的中线绝对误差
    float error = my_abs_float(MID_W - final_mid_line);
    
   
    if(error < 8) // 直道或微小调整区间 (阈值可调，比如 8)
    {
        // 使用较小的 Kp 保证直道稳定性，防止过度修正
        servo_pid.Kp = 0.4f; 
        
        // 使用一个基础的 Kd 来抑制直行时的轻微抖动
        //servo_pid.Kd = 0.1f;
    } 
    else if(error < 20) // 缓弯区间 (阈值可调，比如 25)
    {
        // Kp 从基础值开始，随误差线性增长
        servo_pid.Kp = 0.6f + (error - 8) * 0.0025f; //原0.4 0.025
        
        // Kd 也随误差增长，为快速转向提供更强的“刹车”
        //servo_pid.Kd = 0.1f + (error - 8) * 0.04f;
    }
    else // 急弯区间 (error >= 25)
    {
        // 在急弯中，使用更大的增长率，让转向更猛烈
        // 这个值是在 error=25 的基础上继续增加
        servo_pid.Kp =1.0f;//1.9
        
        // Kd 同理，需要非常大的值来抑制在弯道顶点的超调
       // servo_pid.Kd = 0.1f + (25 - 8) * 0.04f + (error - 25) * 0.08f;
    }

    // 3. 统一进行PID参数限幅（非常重要！）
    // 这一步确保在任何情况下，参数都不会超出安全范围
    if(servo_pid.Kp > 2.0f)  servo_pid.Kp = 2.0f;
    if(servo_pid.Kp < 0.4f)  servo_pid.Kp = 0.4f; // 防止负值或过小

//    if(servo_pid.Kd > 2.5f)  servo_pid.Kd = 2.5f;
//    if(servo_pid.Kd < 0.1f)  servo_pid.Kd = 0.1f; // 防止负值或过小

    // Ki 在舵机控制中通常不用，保持为0
    servo_pid.Ki = 0.0f;
}

/**
 * 初始化PID结构体
 */
void PID_Init(PID *pid, float Kp, float Ki, float Kd, float LowPass)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->LowPass = LowPass;
    
    pid->Out_P = 0.0f;
    pid->Out_I = 0.0f;
    pid->Out_D = 0.0f;
    
    pid->PrevError = 0.0f;
    pid->LastError = 0.0f;
    pid->Error = 0.0f;
    pid->LastData = 0.0f;
}

/**
 * 位置式PID控制算法
 * @param pid PID结构体指针
 * @param NowData 当前测量值
 * @param Point 目标设定值
 * @return 控制输出
 */
float PID_Normal(PID *pid, float NowData, float Point)
{
    pid->Error = Point - NowData;
    pid->Out_P = pid->Error;
    pid->Out_D = (pid->Error - pid->LastError) * pid->LowPass + pid->Out_D * (1.0f - pid->LowPass);
    
    pid->LastError = pid->Error;
    
    return (pid->Kp * pid->Out_P + pid->Kd * pid->Out_D);

}

/**
 * 增量式PID控制算法，含积分限幅
 * @param pid PID结构体指针
 * @param NowData 当前测量值
 * @param Point 目标设定值
 * @return 控制输出
 */
float PID_Increase(PID *pid, float NowData, float Point)
{
    float pid_out_I_limit = 2000.0f; // 积分限幅值，可根据需要调整
    
    pid->Error = Point - NowData;
    pid->Out_P = (pid->Error - pid->LastError);
    
    // 积分项计算
    pid->Out_I = pid->Ki * pid->Error;
    
    // 积分限幅处理
    if (pid->Out_I > pid_out_I_limit)
        pid->Out_I = pid_out_I_limit;
    else if (pid->Out_I < -pid_out_I_limit)
        pid->Out_I = -pid_out_I_limit;
    
    pid->Out_D = (pid->Error - 2.0f * pid->LastError + pid->PrevError);
    
    // 更新误差历史（带滤波）
//    pid->PrevError = 0.9f * pid->LastError + 0.1f * pid->PrevError;
//    pid->LastError = 0.9f * pid->Error + 0.1f * pid->LastError;
//    pid->LastData = NowData;
    
		pid->PrevError = pid->LastError;
    pid->LastError = pid->Error;
    pid->LastData = NowData;
		
    // 计算输出
    //return (pid->Kp * pid->Out_P + pid->Ki*pid->Out_I + pid->Kd * pid->Out_D);
    //return (pid->Kp * pid->Out_P + pid->Ki * pid->Out_I + pid->Kd * pid->Out_D);
		return (pid->Kp * pid->Out_P + pid->Out_I + pid->Kd * pid->Out_D);
}

/**
 * 重置PID控制器
 */
void PID_Reset(PID *pid)
{
    pid->Out_P = 0.0f;
    pid->Out_I = 0.0f;
    pid->Out_D = 0.0f;
    pid->PrevError = 0.0f;
    pid->LastError = 0.0f;
    pid->Error = 0.0f;
    pid->LastData = 0.0f;
}