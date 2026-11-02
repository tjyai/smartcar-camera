 #ifndef __JIGUANG_H__
  #define __JIGUANG_H__

  #include <zf_common_headfile.h>

  extern uint16 jiguang_distance;    // 最新测距值(mm)，主循环直接读

  void jiguang_init(void);
  void jiguang_pit_handler(void);
	void laser_pit_handler(void);
	
	 #endif