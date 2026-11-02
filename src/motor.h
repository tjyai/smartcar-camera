#ifndef __MOTOR_H__
#define __MOTOR_H__

typedef struct motor1
{
  int target_speed;
	int duty;
	int encoder_speed;
	int encoder_raw;
	int total_encoder;

}motor1;

extern struct motor1 motor_l;
extern struct motor1 motor_r;

extern void pit_handler(void);

  void car_start(void);
	void Final_Motor_Control(int speed,float k,int Limit);
	void Encoder_Init(void);
  void Encoder_Data_Get(void);
  void Speed_Set(pwm_channel_enum pin1, gpio_pin_enum pin2, int speed, uint8 just, uint8 lose);
	void Motor_Control(int Speed_L, int Speed_R);

#endif
