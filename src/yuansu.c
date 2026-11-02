#include <zf_common_headfile.h>
#include <intrins.h>
#include <string.h>
#include "yuansu.h"
#define RING_SCAN_START  (SEARCH_IMAGE_H * 2 / 3)   // 环岛扫描开始行，图像下方2/3处
  #define RING_SCAN_END     10    
// ============================================================
 // 调试变量（Watch窗口用，或IPS200显示用）
  int debug_arc_fail = 0;           // 圆弧检测失败原因

  // 边界调试
  int debug_r_bot_lost = 0;         // 右边界底部连续丢失行数
  int debug_r_left     = 0;         // 右边界值<94的个数

  // 出口调试
  int debug_valley_ok   = 0;        // 出口检测: 0=未检测, 1=检测A(单侧),
 // 2=检测B(完全丢失)
  int debug_valley_dec  = 0;        // 下降边形成计数
  int debug_valley_inc  = 0;        // 上升边形成计数
  int debug_valley_lost = 0;        // 收口上方连续丢失行数

  // 右边界指标
  int debug_steady_ok    = 0;       // 右边界稳定检测: 1=通过, 0=未通过
  int debug_steady_total = 0;       // 右边界有效行数
  int debug_steady_jump  = 0;       // 右边界跳变次数
  int debug_exit_phase   = 0;       // 当前出口阶段(0/1)
  int debug_l_bot        = 0;       // 左边界底部连续丢失行数
  int debug_r_bot        = 0;       // 右边界底部连续丢失行数
	int debug_best_len = 0;
  int debug_max_val  = 0;
  int debug_inc_cnt  = 0;
  int debug_dec_cnt  = 0;
	
	int exit_confirm_cnt = 0;
	
	int debug_r_lost_cnt = 0;   // 扫描中右边界丢失行数
  int debug_r_range    = 0;   // 右边界尾部波动范围
  int debug_r_has_94   = 0;   // 是否存在<94: 0=无, 1=有
	
	
	int debug_valid_cnt = 0;
  int debug_top_len   = 0;
  int debug_phase0_cnt = 0;
  int debug_r_valid    = 0;
  int debug_r_jump     = 0;
  int debug_r_has94_ex = 0;
// ============================================================
// 标准赛道宽度
// ============================================================
const int STANDARD_HALF_WIDTH[120] = {
    15, 15, 15, 16, 16, 16, 17, 17, 18, 18,
    19, 19, 20, 20, 21, 21, 22, 23, 23, 24,
    24, 25, 25, 26, 26, 26, 27, 27, 28, 28,
    29, 29, 30, 30, 31, 31, 32, 32, 33, 33,
    34, 35, 35, 36, 36, 37, 37, 37, 38, 38,
    39, 39, 40, 40, 41, 41, 42, 42, 43, 44,
    44, 45, 45, 46, 46, 47, 47, 48, 48, 49,
    49, 49, 50, 50, 51, 51, 52, 53, 53, 54,
    54, 55, 55, 56, 56, 56, 57, 57, 58, 59,
    59, 59, 60, 60, 61, 61, 62, 62, 62, 63,
    63, 64, 64, 65, 65, 65, 66, 66, 67, 68,
    68, 69, 69, 70, 71, 71, 72, 73, 73, 74
};

// ============================================================
// 全局状态变量
// ============================================================

int         corner_row            = -1;
int         lower_corner_row      = -1;
int         ring_arc_found        = 0;
int         ring_fix_below_enable = 0;
int         ring_fix_above_enable = 0;
int         ring_arc_row          = -1;
 static int dynamic_hw = 65;   // 左边界丢失时的动态宽度
ring_step_t ring_step             = RING_NONE;
int  confirm_cnt    = 0;
static int  ring_fill_dist = 0;
int  ring_arc_max   = 0;
static int  last_arc_val   = 0;
static int  out_cool_cnt   = 0;
static int arc_lost_cnt = 0;   // 圆弧连续丢失帧数
static int arc_alive_cnt = 0; 
static int exit_phase    = 0;   
 int find_arc_ok   = 0;   // 当前帧是否找到圆弧
 static int phase0_cnt = 0;
  // ============================================================
  // 边界辅助函数
  // ============================================================
  uint8 left_lost(int row)
  {
      return (left_edge_line[row] <= 8);
  }

  uint8 right_lost(int row)
  {
      return (right_edge_line[row] >= SEARCH_IMAGE_W - 8);
  }

  int count_left_lost(int start_row, int end_row)
  {
      int row, cnt = 0;

      for (row = start_row; row >= end_row; row--)
      {
          if (left_lost(row)) cnt++;
      }

      return cnt;
  }

  int count_right_lost(int start_row, int end_row)
  {
      int row, cnt = 0;

      for (row = start_row; row >= end_row; row--)
      {
          if (right_lost(row)) cnt++;
      }
      return cnt;
  }

// 检测右边界是否是一条倾斜直线（整数运算版本）
// 返回1：是直线，返回0：不是直线
int right_boundary_is_line(int scan_start, int scan_end)
{
    int row, cnt = 0;
    int sum_x = 0, sum_y = 0;
    long sum_xy = 0, sum_x2 = 0;
    int x, y;
    long k_int, b_int, error, max_error;
    long n, denom;

    for (row = scan_start; row >= scan_end; row--)
    {
        if (right_lost(row)) continue;

        x = row;
        y = (int)right_edge_line[row];

        sum_x += x;
        sum_y += y;
        sum_xy += (long)x * y;
        sum_x2 += (long)x * x;
        cnt++;
    }

    if (cnt < 10) return 0;

    n = (long)cnt;
    denom = n * sum_x2 - (long)sum_x * sum_x;

    if (denom == 0) return 0;

    k_int = (n * sum_xy - (long)sum_x * sum_y) * 1000 / denom;
    b_int = ((long)sum_y * 1000 - k_int * sum_x) / n;

    max_error = 0;
    for (row = scan_start; row >= scan_end; row--)
    {
        if (right_lost(row)) continue;

        x = row;
        y = (int)right_edge_line[row];

        error = (long)y * 1000 - (k_int * x + b_int);
        if (error < 0) error = -error;
        error = error / 1000;

        if (error > max_error) max_error = error;
    }

    return (max_error <= 10) ? 1 : 0;
}

uint8 detect_ring(const uint8 *image, uint8 *ring_center_col, uint8 *ring_center_row)
{
    int row, col, chk, black_cnt;
    int row_max  = SEARCH_IMAGE_H - 1;     // 119
    int row_min  = 40;     // 40
    int ROW_STEP = 2;
    int COL_STEP = 2;
    int BLACK_NEED = 2;
    int ring_left  = -1, ring_right = -1;
    int found_row  = -1;
    int ring_width;
    int center_col;
    int top_row = -1, bottom_row = -1;
    // 第一步：扫描左右边界，从下往上扫描，找黑色区域的左右边界
    for (row = row_max; row >= row_min; row -= ROW_STEP)
    {
        const uint8 *p_row = &image[row * SEARCH_IMAGE_W];
        int lane_left  = left_edge_line[row];
        int lane_right = right_edge_line[row];
        int16 temp_left = -1, temp_right = -1;
        // 从左边开始找连续 BLACK_NEED 个黑点 → 黑色区域左边界
        for (col = lane_left; col <= lane_right - BLACK_NEED; col += COL_STEP)
        {
            black_cnt = 0;
            for (chk = 0; chk < BLACK_NEED; chk++)
            {
                if (*(p_row + col + chk) < white_min_point) black_cnt++;
                else break;
            }
            if (black_cnt >= BLACK_NEED) { temp_left = col; break; }
        }
        if (temp_left == -1) continue;
        // 从右边开始找连续 BLACK_NEED 个黑点 → 黑色区域右边界
        for (col = lane_right; col >= lane_left + BLACK_NEED; col -= COL_STEP)
        {
            black_cnt = 0;
            for (chk = 0; chk < BLACK_NEED; chk++)
            {
                if (*(p_row + col - chk) < white_min_point) black_cnt++;
                else break;
            }
            if (black_cnt >= BLACK_NEED) { temp_right = col; break; }
        }
        // 左右都找到了，说明这行有黑色区域
        if (temp_left != -1 && temp_right != -1 && temp_right > temp_left)
        {
            ring_left = temp_left;
            ring_right = temp_right;
            found_row = row;
            break;
        }
    }
    if (found_row == -1 || ring_left < 0) return 0;
    // 尺寸校验：圆环直径应在 8~60 之间
    ring_width = ring_right - ring_left;
    if (ring_width < 8 || ring_width > 60) return 0;
    // 边界白点校验：黑色区域的左边和右边应该是白色赛道
    if (ring_left - 1 < 0) return 0;
    if (*(image + found_row * SEARCH_IMAGE_W + ring_left - 1) < white_min_point) return 0;
    if (ring_right + 1 >= SEARCH_IMAGE_W) return 0;
    if (*(image + found_row * SEARCH_IMAGE_W + ring_right + 1) < white_min_point) return 0;
    // 第二步：沿中线扫描，找中心点（圆环的上下边缘）
    // 圆环截面：黑白黑的分布，从下往上是 黑(下边缘) 白(中间) 黑(上边缘) 白
    center_col = (ring_left + ring_right) / 2;
    // 找下边缘：从下往上，找黑→白跳变（当前行黑，下一行白）
    for (row = row_max - 1; row >= row_min + 1; row--)
    {
        if (image[row * SEARCH_IMAGE_W + center_col] < white_min_point &&
            image[(row + 1) * SEARCH_IMAGE_W + center_col] >= white_min_point)
        {
            bottom_row = row;
            break;
        }
    }
    if (bottom_row == -1) return 0;
    // 找上边缘：从下边缘往上，找白→黑跳变
    for (row = bottom_row - 1; row >= row_min + 1; row--)
    {
        if (image[row * SEARCH_IMAGE_W + center_col] < white_min_point &&
            image[(row + 1) * SEARCH_IMAGE_W + center_col] >= white_min_point)
        {
            top_row = row;
            break;
        }
    }
    if (top_row == -1) return 0;
    // 上下边缘距离校验（圆环直径不能太小）
    if (bottom_row - top_row < 4) return 0;
    *ring_center_col = (uint8)center_col;
    *ring_center_row = (uint8)((top_row + bottom_row) / 2);
    return 1;
}
  // ============================================================
 
static int find_arc_top(int scan_start, int scan_end,
                           int *arc_row, int *arc_val)
  {
      int row, cur_val;
      int lost_start, lost_end;
      int best_len, best_start, best_end;
      int in_lost;
      int max_val, max_row;
      int inc_cnt, dec_cnt;
      int last_val;
      int valid_cnt;
      int dec_end_row;
      int r_bottom, r_top;

      *arc_row = -1;
      *arc_val = 0;

      debug_best_len   = 0;
      debug_max_val    = 0;
      debug_inc_cnt    = 0;
      debug_dec_cnt    = 0;
      debug_arc_fail   = 0;
      debug_r_lost_cnt = 0;
      debug_r_range    = 0;
      debug_r_has_94   = 0;

      if (scan_start >= SEARCH_IMAGE_H) scan_start = SEARCH_IMAGE_H - 1;
      if (scan_start < 0)               scan_start = SEARCH_IMAGE_H - 1;
      if (scan_end   < 0)               scan_end   = 0;
      if (scan_end  >= scan_start)      scan_end   = scan_start - 1;

      // ============================================================
      // 阶段1：左边界连续丢失 >= 8 行
      // ============================================================
    best_len   = 0;
      best_start = -1;
      best_end   = -1;
      in_lost    = 0;
      lost_start = -1;
      lost_end   = -1;
      for (row = 119; row >= 5; row--)
      {
          cur_val = (int)left_edge_line[row];
          if (cur_val <= 5)
          {
              if (!in_lost) { in_lost = 1; lost_start = row; }
              lost_end = row;
          }
          else
          {
              if (in_lost)
              {
                  int len = lost_start - lost_end + 1;
                  if (len > best_len)
                  {
                      best_len  = len;
                      best_start = lost_start;
                      best_end   = lost_end;
                  }
                  in_lost = 0;
              }
          }
      }
      if (in_lost)
      {
          int len = lost_start - lost_end + 1;
          if (len > best_len)
          { best_len = len; best_start = lost_start; best_end = lost_end; }
      }

      debug_best_len = best_len;
      if (best_len < 20) { debug_arc_fail = 1; return 0; }

      // ============================================================
      // 阶段2：找丢失区上方的左边界值 >= 15，有效行 >= 5
      // ============================================================
      max_val   = 0;
      max_row   = -1;
      valid_cnt = 0;

      for (row = best_end - 1; row >= scan_end; row--)
      {
          cur_val = (int)left_edge_line[row];
          if (cur_val <= 2) break;
          if (cur_val > max_val) { max_val = cur_val; max_row = row; }
          valid_cnt++;
      }

      debug_max_val = max_val;
			 debug_valid_cnt = valid_cnt;
      if (max_row < 0)   { debug_arc_fail = 2; return 0; }
      if (max_val < 15)  { debug_arc_fail = 3; return 0; }
      if (valid_cnt < 5) { debug_arc_fail = 4; return 0; }

      // ============================================================
      // 阶段3：上升判断 - 从丢失区到最大值之间的最长连续上升 >= 5
      // ============================================================
      {
          int cur_streak = 0, max_streak = 0;
          last_val = -1;
          for (row = best_end - 1; row >= max_row; row--)
          {
              cur_val = (int)left_edge_line[row];
              if (cur_val <= 2) continue;
              if (last_val > 0 && cur_val > last_val)
              {
                  cur_streak++;
                  if (cur_streak > max_streak) max_streak = cur_streak;
              }
              else { cur_streak = 0; }
              last_val = cur_val;
          }
          inc_cnt = max_streak;
      }

      debug_inc_cnt = inc_cnt;
      if (inc_cnt < 2) { debug_arc_fail = 5; return 0; }

      // ============================================================
      // 阶段4：下降判断 - 从最大值往上，最长连续下降 >= 4
      // ============================================================
      dec_end_row = -1;
      {
          int cur_streak = 0, max_streak = 0;
          last_val = max_val;
          for (row = max_row - 1; row >= scan_end; row--)
          {
              cur_val = (int)left_edge_line[row];
              if (cur_val <= 5) { dec_end_row = row; break; }
              if (cur_val < last_val)
              {
                  cur_streak++;
                  if (cur_streak > max_streak) max_streak = cur_streak;
              }
              else { cur_streak = 0; }
              last_val = cur_val;
          }
          dec_cnt = max_streak;
      }

      debug_dec_cnt = dec_cnt;
      if (dec_cnt < 4) { debug_arc_fail = 6; return 0; }

      // ============================================================
      // 阶段5：最大值上方连续黑行 >= 5 行
      // ============================================================
      if (dec_end_row < 0) { debug_arc_fail = 9; return 0; }
      {
          int top_len = 0;
          for (row = dec_end_row; row >= scan_end; row--)
          {
              cur_val = (int)left_edge_line[row];
              if (cur_val <= 2) top_len++;
              else break;
          }
					debug_top_len = top_len; 
          if (top_len < 0) { debug_arc_fail = 9; return 0; }
      }

      // ============================================================
      // 阶段6：右边界验证
      //   - 最值处右边界不能丢失
      //   - 右边界最值 > 左边界最值
      //   - 扫描范围内右边界丢失 < 3
      //   - 右边界最小值有效 > 94
      //   - 右边界尾部波动 <= 65
      // ============================================================
      if (right_lost(max_row))                 { debug_arc_fail = 7; return 0;
  }
      if (right_edge_line[max_row] <= max_val) { debug_arc_fail = 8; return 0;
  }

      r_bottom          = -1;
      r_top             = -1;
      debug_r_lost_cnt  = 0;
      debug_r_has_94    = 0;
      debug_r_range     = 0;

      for (row = scan_start; row >= scan_end; row--)
      {
          if (right_lost(row)) { debug_r_lost_cnt++; continue; }

          cur_val = (int)right_edge_line[row];
          if (cur_val < 94) { debug_r_has_94 = 1; }

          if (r_bottom < 0) r_bottom = cur_val;
          r_top = cur_val;
      }

      if (r_bottom >= 0)
      {
          debug_r_range = r_bottom - r_top;
          if (debug_r_range < 0) debug_r_range = -debug_r_range;
      }

      if (debug_r_has_94)        { debug_arc_fail = 11; return 0; }
      if (debug_r_range > 65)    { debug_arc_fail = 12; return 0; }
      if (debug_r_lost_cnt >= 3) { debug_arc_fail = 10; return 0; }

      // 右边界必须是倾斜直线
      if (!right_boundary_is_line(scan_start, scan_end))
      {
          debug_arc_fail = 13;
          return 0;
      }

      debug_arc_fail = 0;
      *arc_row = max_row;
      *arc_val = max_val;
      return 1;
  }
	//============================================================
  // 第一阶段：全图右边界补线，同时记录动态宽度
  // ============================================================
  static void ring_fix_below_by_right(void)
  {
      int row;

//      // 以当前右边界与圆弧顶线的距离 / 2 作为动态半宽
//      dynamic_hw = ((int)right_edge_line[ring_arc_row] - ring_arc_max) / 2;
//      if (dynamic_hw < 10) dynamic_hw = STANDARD_HALF_WIDTH[ring_arc_row];

      for (row = 10; row < SEARCH_IMAGE_H; row++)
      {
          int mid = (int)right_edge_line[row] - STANDARD_HALF_WIDTH[row]-8;
          if (mid < 1) mid = 1;
          if (mid > SEARCH_IMAGE_W - 1) mid = SEARCH_IMAGE_W - 1;
          mid_line_list[row] = (uint8)mid;
      }
  }

  // ============================================================
  // 左边界丢失时，左边界 + 动态宽度补线
  // ============================================================
    static void ring_fix_by_left(void)
  {
      int row;
      int last_mid = 60;

      for (row = SEARCH_IMAGE_H - 5; row >= 10; row--)
      {
          int mid;

          if (!left_lost(row) && left_edge_line[row] > 10)
              mid = (int)left_edge_line[row] + dynamic_hw;
          else
              mid = last_mid;          // 丢失时保持上一行的值

          if (mid < 1) mid = 1;
          if (mid > SEARCH_IMAGE_W - 1) mid = SEARCH_IMAGE_W - 1;
          if (mid > last_mid + 4)       mid = last_mid + 4;
          if (mid < last_mid - 4)       mid = last_mid - 4;

          mid_line_list[row] = (uint8)mid;
          last_mid = mid;
      }
  }

 static void ring_fix_by_both(void)
  {
      int row;
      int last_mid = SEARCH_IMAGE_W / 2;
      for (row = SEARCH_IMAGE_H - 5; row >= 10; row--)
      {
          int mid;
          mid = ((int)left_edge_line[row] + (int)right_edge_line[row]) / 2;
          if (mid < 1) mid = 1;
          if (mid > SEARCH_IMAGE_W - 1) mid = SEARCH_IMAGE_W - 1;
          if (mid > last_mid + 4)       mid = last_mid + 4;
          if (mid < last_mid - 4)       mid = last_mid - 4;
          mid_line_list[row] = (uint8)mid;
          last_mid = mid;
      }
  }

 
 static int left_enter_ring(void)
  {
      int row;
      int r_bot_lost = 0;
      int r_left_cnt = 0;

      for (row = SEARCH_IMAGE_H - 1; row >= 10; row--)
      {
          if (right_lost(row)) r_bot_lost++;
          else break;
      }

      for (row = SEARCH_IMAGE_H - 1; row >= 10; row--)
      {
          if (!right_lost(row) && right_edge_line[row] < 85)   //  < 80
              r_left_cnt++;
      }

      debug_r_bot_lost = r_bot_lost;
      debug_r_left     = r_left_cnt;

      return (r_left_cnt > 3 && r_bot_lost > 50&& r_bot_lost < 80);
  }
	
	
  // ============================================================
  // 出口阶段 phase 0：左边界补线，同时向右偏追右边界
  // ============================================================
  static void ring_fix_exit_left(void)
  {
      int row;
      int last_mid = SEARCH_IMAGE_W / 2;

      for (row = SEARCH_IMAGE_H - 5; row >= 10; row--)
      {
          int mid;

          if (!left_lost(row) && left_edge_line[row] > 10)
              mid = (int)left_edge_line[row] + STANDARD_HALF_WIDTH[row];
          else
              mid = last_mid;

          if (mid < 1) mid = 1;
          if (mid > SEARCH_IMAGE_W - 1) mid = SEARCH_IMAGE_W - 1;
          if (mid > last_mid + 4)       mid = last_mid + 4;
          if (mid < last_mid - 4)       mid = last_mid - 4;

          mid_line_list[row] = (uint8)mid;
          last_mid = mid;
      }
  }

	 static int detect_exit_right_valley(void)
  {
      int row;
      int l_lost_all = 1, r_lost_all = 1;

      debug_valley_ok   = 0;
      debug_valley_dec  = 0;
      debug_valley_inc  = 0;
      debug_valley_lost = 0;

      for (row = SEARCH_IMAGE_H - 1; row >= 60; row--)
      {
          if (!left_lost(row))  l_lost_all = 0;
          if (!right_lost(row)) r_lost_all = 0;
      }

      if (l_lost_all && r_lost_all)
      {
          debug_valley_ok = 2;
          return 1;
      }

      return 0;
  }

	
	 static int right_bottom_steady(void)
  {
      int row;
      int total_cnt  = 0;
      int jump_cnt   = 0;
      int prev_val   = -1;
      int scan_start = SEARCH_IMAGE_H - 1;
      int scan_end   = 10;
      int scan_range = scan_start - scan_end + 1;
      int min_need   = scan_range / 2;
      int cur_val;
      debug_steady_total = 0;
      debug_steady_jump  = 0;
      debug_steady_ok    = 0;

      for (row = scan_start; row >= scan_end; row--)
      {
          if (right_lost(row)) continue;

          cur_val = (int)right_edge_line[row];
          total_cnt++;

          if (prev_val >= 0)
          {
              int diff = cur_val - prev_val;
              if (diff < 0) diff = -diff;
              if (diff > 5) jump_cnt++;
          }

          prev_val = cur_val;
      }

      debug_steady_total = total_cnt;
      debug_steady_jump  = jump_cnt;

      if (total_cnt >= min_need && jump_cnt <= 2)
      {
          debug_steady_ok = 1;
          return 1;
      }
      return 0;
  }
	
	 static int right_boundary_straight(void)
  {
      int row;
      int cur_val;
      int diff;
      int total_cnt  = 0;
      int jump_cnt   = 0;
      int prev_val   = -1;
      int first_val  = -1;
      int last_val   = -1;
      int scan_start = SEARCH_IMAGE_H / 2;
      int scan_end   = 10;

      debug_steady_total = 0;
      debug_steady_jump  = 0;
      debug_steady_ok    = 0;

      for (row = scan_start; row >= scan_end; row--)
      {
          if (right_lost(row)) continue;

          cur_val = (int)right_edge_line[row];
          total_cnt++;

          if (first_val < 0) first_val = cur_val;
          last_val = cur_val;

          if (prev_val >= 0)
          {
              diff = cur_val - prev_val;
              if (diff < 0) diff = -diff;
              if (diff > 5) jump_cnt++;
          }

          prev_val = cur_val;
      }

      debug_steady_total = total_cnt;
      debug_steady_jump  = jump_cnt;

      if (total_cnt >= 50)
      {
          int range = first_val - last_val;
          if (range < 0) range = -range;
          if (range <= 30)
          {
              if (jump_cnt <= total_cnt / 4)
              {
                  debug_steady_ok = 1;
                  return 1;
              }
          }
      }
      return 0;
  }

	
	 // ============================================================
  // 状态机
  //   RING_NONE    未 检测到圆弧(5帧确认) → RING_IN
  //   RING_IN      弧 圆弧丢失(arc_alive达3或10帧) → RING_ARC
  //   RING_ARC     弧 左边界进入环岛 → RING_INSIDE
  //   RING_INSIDE  弧 检测到出口 → RING_EXITING
  //   RING_EXITING 弧 phase0完成 / phase1右边界恢复 → RING_NONE
  // ============================================================
 void process_ring(void)
  {
      int arc_row = -1;
      int arc_val = 0;
      int row, r_bot, l_bot;
      int enter;

      find_arc_ok = find_arc_top(RING_SCAN_START, RING_SCAN_END,
                                 &arc_row, &arc_val);

      switch (ring_step)
      {
          case RING_NONE:
          {
              if (find_arc_ok)
              {
                  confirm_cnt++;
                  if (confirm_cnt >= 2)
                  {
                      confirm_cnt   = 0;
                      arc_alive_cnt = 0;
                      arc_lost_cnt  = 0;
                      ring_arc_row  = arc_row;
                      ring_arc_max  = arc_val;
                      ring_step     = RING_IN;
                      ring_fix_below_by_right();
                  }
              }
              else
              { if (confirm_cnt > 0 && confirm_cnt < 3)
              ;                    // 计数在 0~3 之间时不做处理，等待下一帧确认
          else
                  confirm_cnt   = 0;
                  ring_arc_row  = -1;
                  arc_alive_cnt = 0;
              }
              break;
          }
  case RING_IN:
  {
      // 每帧刷新左右边界的底部丢失标志，用于实时显示
      {
          int row2, l_bl = 0;
          for (row2 = SEARCH_IMAGE_H - 1; row2 >= 0; row2--)
          {
              if (left_lost(row2)) l_bl++;
              else break;
          }
          debug_l_bot = l_bl;
      }

 if (find_arc_ok)
  {
      arc_alive_cnt++;
      ring_arc_row = arc_row;
      ring_arc_max = arc_val;
      ring_fix_below_by_right();
  }
  else
  {
      ring_arc_row = -1;
      ring_fix_below_by_right();

      arc_lost_cnt++;
      if (arc_lost_cnt >= 2)
      {
          arc_lost_cnt  = 0;
          arc_alive_cnt = 0;

          if (debug_l_bot < 30)
          {
              ring_step = RING_ARC;
              ring_fix_by_left();
          }
      }
  }
      break;
  }
         

          case RING_ARC:
  {
      static int enter_confirm = 0;

      ring_fix_by_left();

      enter = left_enter_ring();
      if (enter)
      {
          enter_confirm++;
          if (enter_confirm >= 3)
          {
              enter_confirm = 0;
              ring_step = RING_EXITING;  // 直接进入RING_EXITING
              exit_phase = 0;
          }
      }
      else
      {
          enter_confirm = 0;
      }
      break;
  }

//          case RING_INSIDE:
//          {
//              static int exit_confirm = 0;
//
//              ring_fix_by_both();
//
//              if (detect_exit_right_valley())
//              {
//                  exit_confirm++;
//                  if (exit_confirm >= 2)
//                  {
//                      exit_confirm = 0;
//                      exit_phase   = 0;
//                      ring_step    = RING_EXITING;
//                  }
//              }
//              else
//              {
//                  exit_confirm = 0;
//              }
//              break;
//          }

          case RING_EXITING:
          {
              static int phase0_cnt = 0;
              debug_exit_phase = exit_phase;

              if (exit_phase == 0)
              {
                  phase0_cnt++;

                  // 中线 = 右边界 + 50
                  for (row = SEARCH_IMAGE_H - 5; row >= 10; row--)
                  {
                      int mid = (int)right_edge_line[row] + 50;
                      if (mid < 1) mid = 1;
                      if (mid > SEARCH_IMAGE_W - 1) mid = SEARCH_IMAGE_W - 1;
                      mid_line_list[row] = (uint8)mid;
                  }

                   r_bot = 0;
                   for (row = RING_SCAN_START; row >= RING_SCAN_END; row--)
                   {
                       if (right_lost(row)) r_bot++;
                   }
                   debug_r_bot = r_bot;

                   {
                       int r_has94 = 0, r_jump = 0, r_valid = 0;
                       int r_bot_val = -1, r_top_val = -1, r_range = 0;
                       int prev = -1, cv;

                       for (row = RING_SCAN_START; row >= RING_SCAN_END; row--)
                       {
                           if (right_lost(row)) continue;

                           cv = (int)right_edge_line[row];
                           if (cv < 80) { r_has94 = 1; break; }

                           r_valid++;
                           if (r_bot_val < 0) r_bot_val = cv;
                           r_top_val = cv;
                           if (prev >= 0)
                           {
                               int d = cv - prev;
                               if (d < 0) d = -d;
                               if (d > 5) r_jump++;
                           }
                           prev = cv;
                       }
                       if (r_bot_val >= 0)
                       {
                           r_range = r_bot_val - r_top_val;
                           if (r_range < 0) r_range = -r_range;
                       }

                       if (phase0_cnt >= 5 && r_bot < 8 && r_valid >= 10 && r_jump <= 3 && r_has94 == 0 && r_range <= 65)
                      {
                          phase0_cnt = 0;
                          exit_phase = 1;
                      }
                       debug_phase0_cnt = phase0_cnt;
                       debug_r_valid    = r_valid;
                       debug_r_jump     = r_jump;
                       debug_r_has94_ex = r_has94;
                   }
              }
              else
              {
                  phase0_cnt = 0;
                  ring_fix_below_by_right();

                  l_bot = 0;
                  for (row = SEARCH_IMAGE_H - 1; row >= 10; row--)
                  {
                      if (left_lost(row)) l_bot++;
                      else break;
                  }
                  debug_l_bot = l_bot;

                  if (l_bot < 8)
                  {
                      exit_phase    = 0;
                      ring_step     = RING_NONE;
                      confirm_cnt   = 0;
                      arc_alive_cnt = 0;
                      arc_lost_cnt  = 0;
                      ring_arc_row  = -1;
                  }
              }
              break;
          }

          default:
          {
              ring_step = RING_NONE;
              break;
          }
      }

      if (ring_step >= RING_IN && ring_step <= RING_EXITING)
      {
          corner_row = ring_arc_row;
      }
  }
	
	
	
	void draw_arc_top_line(uint8 *image)
  {
      int col;
      if (ring_arc_row < 0 || ring_arc_row >= SEARCH_IMAGE_H) return;

      for (col = 0; col < SEARCH_IMAGE_W; col++)
      {
          image[ring_arc_row * SEARCH_IMAGE_W + col] = 0x00;
      }
      // 上下各加一行加粗显示
      if (ring_arc_row > 0)
      {
          for (col = 0; col < SEARCH_IMAGE_W; col++)
              image[(ring_arc_row - 1) * SEARCH_IMAGE_W + col] = 0x00;
      }
      if (ring_arc_row < SEARCH_IMAGE_H - 1)
      {
          for (col = 0; col < SEARCH_IMAGE_W; col++)
              image[(ring_arc_row + 1) * SEARCH_IMAGE_W + col] = 0x00;
      }
  }