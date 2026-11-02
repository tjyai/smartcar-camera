#include <zf_common_headfile.h>
 
 
 
 
 void init (void)
 {//锟斤拷锟斤拷头锟斤拷始锟斤拷锟斤拷一位
	 mt9v03x_init();
	 //锟斤拷锟斤拷锟绞硷拷锟�
   pwm_init(PWME_CH1P_PA0, 50, duoji_mid);
//gpio锟斤拷始锟斤拷
  gpio_init(GPIO_L, GPO, 1, GPO_PUSH_PULL);
  gpio_init(GPIO_R, GPO, 1, GPO_PUSH_PULL);
	gpio_init(IO_PB2, GPI, 0, GPI_PULL_UP);
	gpio_init(IO_P67, GPO, GPIO_LOW, GPO_PUSH_PULL);//涓�闂存縺鍏�
	gpio_init(IO_P01, GPO, GPIO_LOW, GPO_PUSH_PULL);//宸�1婵�鍏�
	gpio_init(IO_P02, GPO, GPIO_LOW, GPO_PUSH_PULL);//宸�2婵�鍏�
	gpio_init(IO_P03, GPO, GPIO_LOW, GPO_PUSH_PULL);//鍙�2婵�鍏�
	gpio_init(IO_P06, GPO, GPIO_LOW, GPO_PUSH_PULL);//鍙�1婵�鍏�
	 //gpio_init(IO_PB2, GPO, 1, GPO_PUSH_PULL);
//pwm锟斤拷始锟斤拷
  pwm_init(PWM_L, 10000, 0);
  pwm_init(PWM_R, 10000, 0);
//锟斤拷示锟斤拷锟斤拷始锟斤拷
   ips200_init();
	 
	 jiguang_init();
	 //锟斤拷锟斤拷锟斤拷锟斤拷始锟斤拷
	 Encoder_Init();
	 //锟斤拷锟斤拷锟角筹拷始锟斤拷
	imu660rb_init();
	 
//锟斤拷时锟斤拷锟叫断筹拷始锟斤拷锟秸伙拷锟斤拷锟斤拷栈锟斤拷锟斤拷
   pit_ms_init(TIM1_PIT, 20,pit_handler);
   pit_ms_init(TIM2_PIT, 5, laser_pit_handler); 
	 //pit_ms_init(TIM2_PIT, 2,Encoder_Data_Get); 

 
 }
 