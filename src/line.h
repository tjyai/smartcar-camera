#ifndef __LINE_H__
#define __LINE_H__

#define SEARCH_IMAGE_W    MT9V03X_W        // 搜线宽度
#define SEARCH_IMAGE_H    MT9V03X_H        // 搜线高度
#define BLACKPOINT        50               // 黑点值
#define WHITEMAXMUL       13               // 白点最大值基于参考点的放大倍数 10为不放大
#define WHITEMINMUL       7                // 白点最小值基于参考点的放大倍数 10为不放大
#define MID_W 94                  // 赛道中值
#define mid_line_limit 30         //中线值限幅


#define REFRENCEROW    5       // 参考点统计行数
#define SEARCHRANGE   5      // 搜线半径
#define STOPROW       3       // 搜线停止行
#define CONTRASTOFFSET 3      // 搜线对比偏移
#define search_end_line 30        // 搜线停止行


extern uint8 reference_point;                // 动态参考点
extern uint8 reference_col  ;                // 动态参考列
extern uint8 white_max_point;                // 动态白点最大值
extern uint8 white_min_point;                // 动态白点最小值
extern uint8 reference_contrast_ratio;       // 参考对比度
extern uint8 reference_col_line[SEARCH_IMAGE_H];  // 参考列绘制
extern uint8 remote_distance[SEARCH_IMAGE_W];     // 白点远端距离
extern uint8 left_edge_line[SEARCH_IMAGE_H];      // 左右边界
extern uint8 right_edge_line[SEARCH_IMAGE_H];     // 左右边
extern uint8 mid_line_list[MT9V03X_H];
extern uint32 if_count;
extern uint8 final_mid_line;
extern uint8 longest_white_col[SEARCH_IMAGE_W]; // 最长白列结果
extern uint8 smooth_left_line[SEARCH_IMAGE_H];    // 平滑后的左边界
extern uint8 smooth_right_line[SEARCH_IMAGE_H];   // 平滑后的右边界

//void Get_image(uint8 (*mt9v03x_image)[MT9V03X_W]);
void get_reference_point(const uint8 *image);
void search_reference_col(const uint8 *image);
void search_line(const uint8 *image);
void cross_analysis(void);
uint8 find_jump_point(uint8 *array_value, uint8 num0, uint8 num1, uint8 jump_num, uint8 model);
void calculate_mid_line(void);
uint8 find_mid_line_weight(void);
void smooth_line(const uint8 *line_in, uint8 *line_out, int start_row, int end_row, int window_size);
#endif