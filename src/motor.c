#include <zf_common_headfile.h>

void pit_handler (void);


struct motor1 motor_l = {0};   // 左电机
struct motor1 motor_r = {0};   // 右电机


static float current_target_speed=0.0f;


//按键控制电机启动和停止
uint8 start = 0; // 全局变量
void car_start(void)
{
    static uint8 last_key_state = 1; // 记录上一次按键的状态（默认1-没按下）
    uint8 current_key_state;

    // 获取 P3.2 当前的电平状态
    current_key_state = gpio_get_level(IO_PB2); 

    // 逻辑：如果上次是1（没按），现在是0（按下），说明触发了“按下”动作
    if (last_key_state == 1 && current_key_state == 0) 
    {
        start++;       // 改变启动状态
//        system_delay_ms(10); // 简单的软件去抖动
    }

    last_key_state = current_key_state; // 更新状态，留给下一次判断
}

/**********************************************
 函数功能: 电机差速控制
 函数名称: void Final_Motor_Control(int speed,float k,int limit)
 输入参数: 差速原始速度,差速比例系数,差速限幅
 输出参数: 无
 备 注: 无
 **********************************************/

void Final_Motor_Control(int speed, float k, int limit)
{
    Motor_Control(Limit_int(speed-limit, speed-k*(MID_W-final_mid_line), speed+limit),
                  Limit_int(speed-limit, speed+k*(MID_W-final_mid_line), speed+limit));
}

/*************************************************
* 函数功能: 编码器初始化
* 函数名称: void Encoder_Init(void)
* 输入参数: 无
* 输出参数: 无
* 备 注: 无
*************************************************/
void Encoder_Init(void)
{
    encoder_quad_init(PWMA_ENCODER, PWMA_ENCODER_CH2P_P62, PWMA_ENCODER_CH1P_P60);   // 初始化编码器模块与引脚 带方向增量编码器模式
    encoder_quad_init(PWMC_ENCODER,PWMC_ENCODER_CH2P_P42, PWMC_ENCODER_CH1P_P40);   // 初始化编码器模块与引脚 带方向增量编码器模式
}

/*************************************************
* 函数功能: 编码器数值获取
* 函数名称: void Encoder_Data_Get(void)
* 输入参数: 无
* 输出参数: 无
* 备 注: 无
*************************************************/
void Encoder_Data_Get(void)
{
    int16 raw_l, raw_r;
    
    // 1. 读取原始值
    raw_l = encoder_get_count(PWMA_ENCODER);
    raw_r = -encoder_get_count(PWMC_ENCODER);

    // 2. 【核心：防干扰滤波】
    // 如果瞬时增量超过 300（这个值根据你车速调，通常设为正常速度的1.5倍），就认为是干扰，丢弃它
    if(my_abs_float(raw_l - motor_l.encoder_raw) > 300) raw_l = motor_l.encoder_raw;
    if(my_abs_float(raw_r - motor_r.encoder_raw) > 300) raw_r = motor_r.encoder_raw;

    motor_l.encoder_raw = raw_l;
    motor_r.encoder_raw = raw_r;

    // 3. 滤波后的速度计算
    motor_l.encoder_speed = motor_l.encoder_speed * 0.4 + motor_l.encoder_raw * 0.6;
    motor_r.encoder_speed = motor_r.encoder_speed * 0.4 + motor_r.encoder_raw * 0.6;

    encoder_clear_count(PWMA_ENCODER);
    encoder_clear_count(PWMC_ENCODER);
}

//设置电机引脚，方向，速度
//作用pwm为负时候，反转
void Speed_Set(pwm_channel_enum pin1, gpio_pin_enum pin2, int speed, uint8 just, uint8 lose)
{
	if(speed>0)
	{
	pwm_set_duty(pin1,speed);
  gpio_set_level(pin2,just);
	}
  else
{
  pwm_set_duty (pin1,-speed);
	gpio_set_level (pin2,lose);
}
}


//电机闭环控制
void Motor_Control(int speed_L, int speed_R)
{
    int out_l, out_r;
    motor_l.target_speed = speed_L;
    motor_r.target_speed = speed_R;
 
    // 1. PID 计算增量
    motor_l.duty += PID_Increase(&motor_pid_l, (float)motor_l.encoder_speed, (float)motor_l.target_speed);
    motor_r.duty += PID_Increase(&motor_pid_r, (float)motor_r.encoder_speed, (float)motor_r.target_speed);

    // 2. 【绝对核心】：把下限锁死在 0！天王老子来了也不能变成负数！绝不允许倒车！
    motor_l.duty = Limit_int(0, motor_l.duty, pid_limit);
    motor_r.duty = Limit_int(0, motor_r.duty, pid_limit);

    // 3. 【绝对核心】：注释掉死区补偿！让 PID 自己用纯粹的占空比去克服摩擦力！
    // out_l = deadzone_compensate(motor_l.duty, 1000);  <-- 删掉或注释掉
    // out_r = deadzone_compensate(motor_r.duty, 1000);  <-- 删掉或注释掉
    out_l = motor_l.duty; 
    out_r = motor_r.duty; 

    // 4. 最终限幅（下限同样是0）
    out_l = Limit_int(0, out_l, pid_limit);
    out_r = Limit_int(0, out_r, pid_limit);
     
    // 5. 底层输出
    Speed_Set(PWM_L, GPIO_L, out_r, 1, 0);
    Speed_Set(PWM_R, GPIO_R, out_l, 1, 0);
}   
 
//void pit_handler(void)
//{   
////	int base_pwm;
////    float diff_k;
////    float error;
////    int pwm_l;
////    int pwm_r;
////	int current_base = base_pwm;
////	 //current_target_speed = 60.0f;
////	//final_mid_line = 93.0f;
////    // 1. 动态调整舵机 PID 参数（如果你想用的话）
////    
//////	if (current_target_speed < 40.0f)
//////    {
//////        current_target_speed += 0.5f; // 每次中断加0.5，慢慢涨上去
//////    }
//////   


////	 //Speed_Set (PWM_L, GPIO_L, 2000,0,1 );//这里的duty是pid算出来的
////   //Speed_Set (PWM_R, GPIO_R, 2000,0,1);
////    // 3. 电机差速与闭环控制
////    // 参数含义：基础速度(如80)，差速比例系数(如0.3)，差速限幅(如30)
////    // 速度值需要根据你的编码器实际量级进行修改测试
////	
////	  

////    // 2. 然后才能写执行逻辑和赋值语句
////    base_pwm = 1200;   // 基础占空比
////    diff_k = 15;    // 差速比例
////    
////    // 舵机闭环
////    

////    // 计算偏差
////    error = MID_W - final_mid_line; 
////    if(my_abs_float(error) < 5) diff_k = 0;
////    
////    if(error > 40 || error < -40) 
////    {
////        current_base = base_pwm + 1000; // 急弯时底盘动力额外加 1000
////    }

////    pwm_l = current_base - (int)(error * diff_k); 
////    pwm_r = current_base + (int)(error * diff_k); 

////    // 调用底层输出
////    Speed_Set(PWM_L, GPIO_L, pwm_l, 0, 1); 
////    Speed_Set(PWM_R, GPIO_R, pwm_r, 0, 1); 
////   
////    // 调基础速度 PID 时，建议先把差速系数设为 0，防止摄像头中线干扰两轮的独立阶跃响应
////    //Final_Motor_Control((int)current_target_speed, 0, 0);
// // 1. 变量定义并直接初始化
//    int base_pwm = 1200;   // 基础占空比
//    float diff_k = 15.0f;  // 差速比例 原15
//    float error = MID_W - final_mid_line; 
//    int current_base = base_pwm;
//    int pwm_l, pwm_r;

//    
//    

//    // 3. 差速逻辑
//    if(my_abs_float(error) < 5.0f) {
//        diff_k = 0; // 直道不差速
//    }
//    
//    // 急弯补偿
//    if(error > 40 || error < -40) {
//        current_base = base_pwm + 500; // 适当增加动力防止卡死 原500
//    }

//    // 计算左右轮 PWM
//    pwm_l = current_base - (int)(error * diff_k); 
//    pwm_r = current_base + (int)(error * diff_k); 

//    // 4. 限幅并输出
//    pwm_l = Limit_int(-5000, pwm_l, 5000); // 这里的 5000 根据你的电机功率调
//    pwm_r = Limit_int(-5000, pwm_r, 5000);

//    Speed_Set(PWM_L, GPIO_L, pwm_l, 1, 0); 
//    Speed_Set(PWM_R, GPIO_R, pwm_r, 1, 0); 
//}
void pit_handler(void)
{
    float error, abs_error;
    float diff_k; 
    int dynamic_base;
    int target_l, target_r;  
    
    Encoder_Data_Get();
    car_start();
    
    error = MID_W - final_mid_line; 
    abs_error = my_abs_float(error);

    if(start % 2 == 1) // 启动
    {
        // --- 第一步：动态速度规划 ---
        if(abs_error < 8) 
        {
            // 【直道】：保持稳定
            dynamic_base = 100; 
            diff_k = 0.4f; // 直道差速不用太大，防止细微干扰
        }
        else if(abs_error < 35) 
        {
            // 【标准弯道】：增加动力补偿摩擦力
            // 速度提到 300，让车有劲冲过弯道
            dynamic_base = 100; 
            diff_k = 1.0f; // 加大差速系数，帮助转向
        }
        else 
        {
            dynamic_base = 100; 
            diff_k = 1.2f; // 急弯需要极大的差速
        }

        // --- 第二步：软启动平滑处理 ---
        if(current_target_speed < dynamic_base) 
            current_target_speed += 20.0f; // 向上加速
        else if(current_target_speed > dynamic_base)
            current_target_speed -= 30.0f; // 向下减速（减速可以快一点）

        // --- 第三步：计算差速 ---
        target_l = (int)(current_target_speed - error * diff_k);
        target_r = (int)(current_target_speed + error * diff_k);

        Motor_Control(target_l, target_r);
    }
    else // 停止
    {
        Motor_Control(0, 0);
        current_target_speed = 0.0f;
        motor_l.duty = 0;
        motor_r.duty = 0;
        PID_Reset(&motor_pid_l);
        PID_Reset(&motor_pid_r);
    }
}
