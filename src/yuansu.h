  #ifndef __YUANSU_H__
  #define __YUANSU_H__

  #include <zf_common_headfile.h>
	
  
  typedef enum {
      RING_NONE = 0,
      RING_IN,
      RING_ARC,
      RING_PREPARE,
      RING_ENTERING,
      RING_INSIDE,
      RING_EXITING
  } ring_step_t;

  extern ring_step_t ring_step;
  extern int corner_row;
  extern int lower_corner_row;

  extern int debug_arc_fail;
  extern int debug_best_len;
  extern int debug_max_val;
  extern int debug_inc_cnt;
  extern int debug_dec_cnt;

  extern int debug_r_bot_lost;
  extern int debug_r_left;

  extern int debug_valley_ok;
  extern int debug_valley_dec;
  extern int debug_valley_inc;
  extern int debug_valley_lost;

  extern int debug_steady_ok;
  extern int debug_steady_total;
  extern int debug_steady_jump;
  extern int debug_exit_phase;
  extern int debug_l_bot;
  extern int debug_r_bot;

 extern int debug_r_lost_cnt;
  extern int debug_r_range;
  extern int debug_r_has_94;

  extern int debug_valid_cnt;
  extern int debug_top_len;
  extern int debug_phase0_cnt;
  extern int debug_r_valid;
  extern int debug_r_jump;
  extern int debug_r_has94_ex;

extern int find_arc_ok;
  extern int confirm_cnt;
	
	extern int exit_confirm_cnt ;
  uint8 detect_ring(const uint8 *image, uint8 *ring_center_col, uint8
  *ring_center_row);
  uint8 detect_crossroad(const uint8 *left_line, const uint8 *right_line, uint8
   *cross_start_row);
  void process_ring(void);
  void ips200_show_ring_debug(void);
void draw_arc_top_line(uint8 *image);
  #endif
