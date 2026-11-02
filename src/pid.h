#ifndef __PID_H__
#define __PID_H__

#include "zf_common_headfile.h"

typedef struct PID
{
    float Kp;
    float Ki;
    float Kd;
    float LowPass;

    float Out_P;
    float Out_I;
    float Out_D;

    float PrevError;
    float LastError;
    float Error;
    float LastData;
} PID;

#define PID_CREATE(_kp, _ki, _kd, _low_pass) \
{                                           \
    _kp, _ki, _kd, _low_pass, 0.0f, 0.0f, 0.0f \
}

// 简化宏定义，只用于初始化常量值
#define PID_CREATE_DEFAULT {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

#define pid_out_I_Limit 30 //增量式pid积分项限幅，防止起步猛冲

// 定义外部可能用到的变量
extern float PID_W;
extern uint8 final_mid_line;

// 外部PID结构体声明
extern PID servo_pid;
extern PID motor_pid_l;
extern PID motor_pid_r;

// 函数声明
void PID_Init(PID *pid, float Kp, float Ki, float Kd, float LowPass);
void PID_Reset(PID *pid);
float PID_Normal(PID *pid, float NowData, float Point);
float PID_Increase(PID *pid, float NowData, float Point);
void dynamic_pid_value_set(void);
float my_abs_float(float x);

#endif