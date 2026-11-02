#include <zf_common_headfile.h>
#include "line.h"

uint8 reference_point;                // 动态参考点
uint8 reference_col;                   // 动态参考列
uint8 white_max_point;                 // 动态白点最大值
uint8 white_min_point;                 // 动态白点最小值
uint8 reference_contrast_ratio = 30;   // 参考对比度（可自己调）
uint8 reference_col_line[SEARCH_IMAGE_H];  // 参考列绘制
uint8 longest_white_col[SEARCH_IMAGE_W]; // 最长白列结果
uint8 remote_distance[SEARCH_IMAGE_W];     // 白点远端距离
uint8 left_edge_line[SEARCH_IMAGE_H];      // 左边界
uint8 right_edge_line[SEARCH_IMAGE_H];     // 右边界
uint32 if_count;
uint8 mid_line_list[MT9V03X_H];       // 中线数组
uint8 smooth_left_line[SEARCH_IMAGE_H];    // 平滑后的左边界
uint8 smooth_right_line[SEARCH_IMAGE_H];   // 平滑后的右边界

#define ROW_CONTRAST_OFFSET      3       // 行扫描步长（改小，精度更高）
#define CONSECUTIVE_BLACK        5       // 需要的连续黑点数量
#define BLACK_RING_MAX  8

#define RING_WIDTH_THRESHOLD 6  // 黑色连续宽度小于此值视为圆环干扰

// 函数简介    获取参考点位
// 参数说明    image          图像数组指针
// 返回类型    void
// 使用示例    get_reference_point(mt9v03x_image[0]);
// 备注信息
//--------------------------------------------------------------------------------------------------
void get_reference_point(const uint8 *image)
{
    //uint8 *p = (uint8 *)&image[(SEARCH_IMAGE_H-REFRENCEROW) * SEARCH_IMAGE_W];  // 图像数组指针指向待统计区域
	  const uint8 *p = image + (SEARCH_IMAGE_H - REFRENCEROW) * SEARCH_IMAGE_W;
    uint16 temp  = 0;                                                          // 保存统计点总数量
    uint32 temp1 = 0;                                                          // 保存所有统计点加起来的和
    int i;
    temp = REFRENCEROW * SEARCH_IMAGE_W;                                       // 计算待统计点总数量
    for( i= 0; i < temp; i ++)
    {
        temp1 += *(p + i);                                                     // 统计点求和
    }
    reference_point = (uint8)(temp1 / temp);                                   // 计算点平均值，作为本幅图像的参考点
    white_max_point = (uint8)func_limit_ab((int32)reference_point * WHITEMAXMUL / 10, BLACKPOINT, 255);  // 根据参考点计算白点最大值
    white_min_point = (uint8)func_limit_ab((int32)reference_point * WHITEMINMUL / 10, BLACKPOINT, 255);  // 根据参考点计算白点最小值
}

void search_reference_col(const uint8 *image)  // 找最长白列
{
    int col, row;
    uint16 temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    int i;
    uint8 pix;
    uint16 last_edge_row = SEARCH_IMAGE_H - 1;
    uint16 last_edge_col = 0;
    uint8 search_failed = 0;
    uint16 search_start_row;
    uint16 search_end_row;
	  uint8 found;
    for (col = 0; col < SEARCH_IMAGE_W; col++)
    {
        remote_distance[col] = SEARCH_IMAGE_H - 1;
    }

    for (col = 0; col < SEARCH_IMAGE_W; col += CONTRASTOFFSET)
    {
         
         search_end_row = STOPROW;

        if (col > 0 && search_failed == 0)
        {
            search_start_row = last_edge_row + ROW_CONTRAST_OFFSET;
            if (search_start_row >= SEARCH_IMAGE_H)
                search_start_row = SEARCH_IMAGE_H - 1;
        }
        else
        {
            search_start_row = SEARCH_IMAGE_H - 1;
            search_failed = 0;
        }

        found = 0;

        for (row = search_start_row; row > search_end_row; row -= CONTRASTOFFSET)
        {
            temp1 = *(image + row * SEARCH_IMAGE_W + col);
            temp2 = *(image + (row - STOPROW) * SEARCH_IMAGE_W + col);

            if (temp2 > white_max_point)
                continue;

            if (temp1 < white_min_point)
            {
                if (row >= 5)
                    temp4 = *(image + (row - 5) * SEARCH_IMAGE_W + col);
                else
                    temp4 = 0;

                if (temp4 < white_min_point)
                {
                    remote_distance[col] = (uint8)row;
                    last_edge_row = row;
                    found = 1;
                    break;
                }

                search_failed = 1;
                break;
            }

            temp3 = (temp1 - temp2) * 200 / (temp1 + temp2);

            if (temp3 > reference_contrast_ratio || row >= search_end_row)
            {
                remote_distance[col] = (uint8)row;
                last_edge_row = row;
                found = 1;
                break;
            }
        }

        // 后备搜索：从底部重新搜索，加入圆环过滤
        if (!found)
        {
            for (row = SEARCH_IMAGE_H - 1; row > STOPROW; row -= CONTRASTOFFSET)
            {
                if (row < 1) break;

                temp1 = *(image + row * SEARCH_IMAGE_W + col);
                temp2 = *(image + (row - STOPROW) * SEARCH_IMAGE_W + col);

                if (temp2 > white_max_point)
                    continue;

                if (temp1 < white_min_point)
                {
                    // 统计向上连续黑色像素数，过滤圆环
                    int black_count = 0;
                    int check_row;
                    for (check_row = row; check_row > STOPROW && black_count < BLACK_RING_MAX; check_row--)
                    {
                        pix = *(image + check_row * SEARCH_IMAGE_W + col);
                        if (pix < white_min_point)
                            black_count++;
                        else
                            break;
                    }

                    if (black_count >= BLACK_RING_MAX)
                    {
                        // 连续黑色足够，是真实边界
                        remote_distance[col] = (uint8)row;
                        last_edge_row = row;
                        last_edge_col = col;
                        found = 1;
                        search_failed = 0;
                        break;
                    }
                    else
                    {
                        // 是圆环，跳过这段黑色区域继续向上
                        row = check_row;
                        continue;
                    }
                }

                temp3 = (temp1 - temp2) * 200 / (temp1 + temp2);

                if (temp3 > reference_contrast_ratio || row <= STOPROW)
                {
                    remote_distance[col] = (uint8)row;
                    last_edge_row = row;
                    last_edge_col = col;
                    found = 1;
                    search_failed = 0;
                    break;
                }
            }
        }
    }

    reference_col = find_extreme_index(remote_distance, 10, SEARCH_IMAGE_W - 10, 1);
    reference_col = (uint8)func_limit_ab(reference_col, 1, SEARCH_IMAGE_W - 2);

    for (i = 0; i < SEARCH_IMAGE_H; i++)
    {
        reference_col_line[i] = reference_col;
    }
}





//void search_reference_col(const uint8 *image)//找最长白列
//{
//    int col, row;
//    int16 temp1 = 0, temp2 = 0, temp3 = 0;
//    int i;
//    // 初始化所有列的距离为图像底部
//    for(col = 0; col < SEARCH_IMAGE_W; col ++)
//    {
//        remote_distance[col] = SEARCH_IMAGE_H - 1;
//    }

//    // 按步长扫描每一列
//    for(col = 0; col < SEARCH_IMAGE_W; col += CONTRASTOFFSET)
//    {
//        // 从图像底部向上扫描
//        for(row = SEARCH_IMAGE_H - 1; row > STOPROW; row -= CONTRASTOFFSET)
//        {
//            // 获取当前点和对比点的灰度值
//            temp1 = *(image + row * SEARCH_IMAGE_W + col);                // 当前点
//            temp2 = *(image + (row - STOPROW) * SEARCH_IMAGE_W + col);     // 对比点（上方STOPROW行）

//            // 对比点过亮，跳过
//            if(temp2 > white_max_point)
//            {
//                continue;
//            }

//            // 当前点过暗，直接判定为边缘
//            if(temp1 < white_min_point)
//            {
//                remote_distance[col] = (uint8)row;
//                break;
//            }

//            // 计算局部对比度
//            temp3 = (temp1 - temp2) * 200 / (temp1 + temp2);

//            // 对比度达标或扫描到顶，判定为边缘
//            if(temp3 > reference_contrast_ratio || row == STOPROW)
//            {
//                remote_distance[col] = (uint8)row;
//                break;
//            }
//        }
//    }

//    // 计算参考列
//    reference_col = find_extreme_value(remote_distance, 10, SEARCH_IMAGE_W - 10, 0) + CONTRASTOFFSET;
//    reference_col = (uint8)func_limit_ab(reference_col, 1, SEARCH_IMAGE_W - 2);

//    // 生成参考线
//    for(i = 0; i < SEARCH_IMAGE_H; i++)
//    {
//        reference_col_line[i] = reference_col;
//    }
//}





void search_line(const uint8 *image)
{
    uint8 *p = (uint8 *)image;
    uint8 row_max = SEARCH_IMAGE_H - 1;
    uint8 row_min = STOPROW;
    uint8 col_max = SEARCH_IMAGE_W - CONTRASTOFFSET;
    uint8 col_min = CONTRASTOFFSET;

    int16 leftstartcol  = reference_col;
    int16 rightstartcol = reference_col;
    int16 leftendcol    = col_min;
    int16 rightendcol   = col_max;

    uint8 search_time = 0;
    uint8 temp1 = 0, temp2 = 0;
    int   temp3 = 0;
    int   leftstop = 0, rightstop = 0, stoppoint = 0;

    int col, row;

    uint8 left_found  = 1;
    uint8 right_found = 1;

    for(row = row_max; row >= row_min; row--)
    {
       left_edge_line[row]  = col_min;
        right_edge_line[row] = col_max;
    }

    for(row = row_max; row >= row_min; row--)
    {
        p = (uint8 *)&image[row * SEARCH_IMAGE_W];

        /* ==================== 左侧边界搜索 ==================== */
        if(!leftstop)
        {
            uint8 row_left_found = 0;
            uint8 filter_ring    = 1;

            if(left_found&& row < row_max)
            {
                leftstartcol = (int16)func_limit_ab(left_edge_line[row + 1] + 5, col_min, col_max);
               
            }
            else
            {
                leftstartcol = reference_col;
               
                filter_ring  = 1;
            }

            search_time = 2;
            do
            {
                if(search_time == 1)
                {
                    leftstartcol = reference_col;
									while(*(p + leftstartcol) < white_min_point && leftstartcol < col_max)
    {
        leftstartcol++;
    }
                    leftendcol   = col_min;
                    filter_ring  = 0;
                }
                search_time--;
                for(col = leftstartcol; col > leftendcol; col--)
                {
                    temp1 = *(p + col);
                    temp2 = *(p + col - 1);
                    
                    if(temp1 >= white_min_point && temp2 < white_min_point)
                    {
                        if(filter_ring) 
                        {
                            // 修正点：确保不再这里定义变量，或者直接写逻辑
                            // 我们改为直接检查深度点 (col-6)
                            if(col - 6 >= col_min)
                            {
                                if(*(p + col - 6) >= white_min_point)
                                {
                                    continue; 
                                }
                            }
                        }

                        left_edge_line[row] = (uint8)(col - 1);
                        row_left_found = 1;
                        search_time = 0;
                        break;
                    }
                } 

            } while(search_time > 0);

            left_found = row_left_found;
        }

        /* ==================== 右侧边界搜索 ==================== */
        if(!rightstop)
        {
            uint8 row_right_found = 0;
            uint8 filter_ring_r   = 1;

            if(right_found&& row < row_max)
            {
                rightstartcol = (int16)func_limit_ab(right_edge_line[row + 1] - 5, col_min, col_max);
                rightendcol   = col_max;
							filter_ring_r = 1;
            }
            else
            {
                rightstartcol = (int16)reference_col;
                rightendcol   = col_max;
                filter_ring_r = 0;
            }

            search_time = 2;
            do
            {
                if(search_time == 1)
                {
                    rightstartcol = (int16)reference_col;
                    rightendcol   = col_max;
                    filter_ring_r = 0;
                }
                search_time--;

             for(col = rightstartcol; col < rightendcol; col++)
                {
                    temp1 = *(p + col);     // 当前点（左侧，应该是亮）
                    temp2 = *(p + col + 1); // 右侧邻居（应该是黑）

                    if(temp1 >= white_min_point && temp2 < white_min_point)
                    {
                        if(filter_ring_r)
                        {
                            // --- 检测跳变点右边第5个像素 (col+1+5 = col+6) ---
                            if(col + 6 > col_max || *(p + col + 6) >= white_min_point)
                            {
                                continue; 
                            }
                        }

                        right_edge_line[row] = (uint8)(col + 1);
                        row_right_found = 1;
                        search_time = 0;
                        break;
                    }
                }

                   
            } while(search_time > 0);

            right_found = row_right_found;
        }
    }
}


//void search_line(const uint8 *image)
//{
//    uint8 *p = (uint8 *)image;                              // 图像数组指针
//    uint8 row_max = SEARCH_IMAGE_H - 1;                        // 行最大值
//    uint8 row_min = STOPROW;                                  // 行最小值
//    uint8 col_max = SEARCH_IMAGE_W - CONTRASTOFFSET;           // 列最大值
//    uint8 col_min = CONTRASTOFFSET;                            // 列最小值

//    int16 leftstartcol  = reference_col;                       // 搜线左起始列
//    int16 rightstartcol = reference_col;                       // 搜线右起始列
//    int16 leftendcol    = 0;                                    // 搜线左终止列
//    int16 rightendcol   = SEARCH_IMAGE_W - 1;                   // 搜线右终止列

//    uint8 search_time   = 0;                                    // 单点搜线次数
//    uint8 temp1 = 0, temp2 = 0;                                // 临时变量 用于存储图像数据
//    int   temp3 = 0;                                           // 临时变量  用于存储对比度
//    int   leftstop = 0, rightstop = 0, stoppoint = 0;          // 搜线自锁变量

//    int col, row;

//    /* ---- 初始化边界线数组 ---- */
//    for(row = row_max; row >= row_min; row--)
//    {
//        left_edge_line[row]  = col_min - CONTRASTOFFSET;
//        right_edge_line[row] = col_max + CONTRASTOFFSET;
//    }

//    /* ==================== 左侧边界搜索 ==================== */
//    for(row = row_max; row >= row_min; row--)
//    {
//        p = (uint8 *)&image[row * SEARCH_IMAGE_W];              // 获取本行起点位置指针

//        if(!leftstop)
//        {
//            search_time = 2;
//            do
//            {
//                if(search_time == 1)
//                {
//                    leftstartcol = reference_col;
//                    leftendcol   = col_min;
//                }
//                search_time--;

//                for(col = leftstartcol; col >= leftendcol; col--)
//                {
//                    temp1 = *(p + col);                         // 获取当前点灰度值
//                    temp2 = *(p + col - CONTRASTOFFSET);        // 获取对比点灰度值

//                  
//                    if(temp1 < white_min_point && col == leftstartcol && leftstartcol == reference_col)
//                    {
//                        leftstop = 1;                           // 线搜索自锁，不再进行左边线搜索
//                        for(stoppoint = row; stoppoint >= 0; stoppoint--)
//                        {
//                            left_edge_line[stoppoint] = col_min;
//                        }
//                        search_time = 0;
//                        break;
//                    }

//                    if(temp1 < white_min_point)//黑
//                    {
//                        left_edge_line[row] = (uint8)col;
//                        break;
//                    }

//                    if(temp2 > white_max_point)                 // 对比点为白点，跳过
//                    {
//                        continue;
//                    }

//                 
//                    temp3 = (temp1 - temp2) * 200 / (temp1 + temp2);  // 计算对比度

//                    if(temp3 > reference_contrast_ratio || col == col_min)
//                    {
//                        left_edge_line[row] = (uint8)(col - CONTRASTOFFSET);  // 保存左边界

//                        leftstartcol = (uint16)func_limit_ab(col + SEARCHRANGE, col, col_max);
//                        leftendcol   = (uint16)func_limit_ab(col - SEARCHRANGE, col_min, col);
//                        search_time = 0;
//                        break;
//                    }
//                }
//            } while(search_time > 0);
//        }

//    if(!rightstop)
//    {
//        search_time = 2;
//        do
//        {
//            if(search_time == 1)
//            {
//                rightstartcol = (int16)reference_col;       // 转为 int16 避免溢出
//                rightendcol   = col_max;
//            }
//            search_time--;

//            for(col = rightstartcol; col <= rightendcol; col++)
//            {
//                temp1 = *(p + col);                         // 获取当前点灰度值
//                temp2 = *(p + col + CONTRASTOFFSET);        // 获取对比点灰度值（右侧偏移）

//                /* 自锁：起始点为黑点且已到达参考列 */
//                if(temp1 < white_min_point && col == rightstartcol && rightstartcol == (int16)reference_col)
//                {
//                    rightstop = 1;                           // 线搜索自锁
//                    for(stoppoint = row; stoppoint >= 0; stoppoint--)
//                    {
//                        right_edge_line[stoppoint] = col_max;
//                    }
//                    search_time = 0;
//                    break;
//                }

//                if(temp1 < white_min_point)
//                {
//                    right_edge_line[row] = (uint8)col;
//                    break;
//                }

//                if(temp2 > white_max_point)                 // 对比点为白点，跳过
//                {
//                    continue;
//                }

//                /* 修正对比度公式：右侧用 (temp2 - temp1)，与左侧 (temp1 - temp2) 镜像对称 */
//                /* 保证左侧/右侧都得到正值对比度 */
//                temp3 = (temp1 - temp2) * 200 / (temp1 + temp2);

//                if(temp3 > reference_contrast_ratio || col == col_max)
//                {
//                    right_edge_line[row] = (uint8)(col + CONTRASTOFFSET);  // 保存右边界

//                    rightstartcol = (int16)func_limit_ab(col - SEARCHRANGE, col_min, col);
//                    rightendcol   = (int16)func_limit_ab(col + SEARCHRANGE, col, col_max);
//                    search_time = 0;
//                    break;
//                }
//            }
//        } while(search_time > 0);
//    }
//}

//}



void cross_analysis(void)//十字交叉路口检测与补线
{    int cross_flag=0;
    uint32 track_width = 0;
    uint8 start_point;
    uint8 end_point;
    int i;
    for(i = (MT9V03X_H * 2 / 3); i > (MT9V03X_H / 3); i --)
    {
        track_width += (right_edge_line[i] - left_edge_line[i]);
    }

    if(track_width > (MT9V03X_W * (MT9V03X_H * 4 / 15)))
    {
        cross_flag = 1;
    }

    if(cross_flag == 1)
    {
//        start_point   = find_jump_point(left_edge_line, MT9V03X_H - 5, 5, 30, 1) + 2;
//        end_point     = find_jump_point(left_edge_line, MT9V03X_H - 5, 5, 30, 0) - 2;
			start_point = find_jump_point(left_edge_line, MT9V03X_H - 5, 5, 30, 1) + 2;  // 反向，不用改
      end_point   = find_jump_point(left_edge_line, 5, MT9V03X_H - 5, 30, 0) - 2;  // 正向，交换！

        connect_point(left_edge_line, start_point, end_point);

        start_point   = find_jump_point(right_edge_line, MT9V03X_H - 5, 5, 30, 1) + 2;
         end_point = find_jump_point(right_edge_line, 5, MT9V03X_H - 5, 30, 0) - 2;
        connect_point(right_edge_line, start_point, end_point);

        if(track_width < (MT9V03X_W * (MT9V03X_H * 1 / 5)))
        {
            cross_flag = 0;
        }
    }
}
/**
 * @brief  查找数组指定范围内的跳变点（灰度值发生显著变化的点）
 * @param  array_value : 指向目标数组的指针
 * @param  num0        : 查找起始索引
 * @param  num1        : 查找结束索引
 * @param  jump_num    : 跳变阈值，当前点与前一点的灰度差大于此值时认为是跳变点
 * @param  model       : 搜索方向  0-正向遍历(索引从小到大)  1-反向遍历(索引从大到小)
 * @return uint8       : 返回跳变点的索引，未找到则返回 0xFF
 * @example find_jump_point(arr, 0, 9, 30, 0)  --> 正向查找 arr[0]~arr[9] 中首个跳变点
 * @example find_jump_point(arr, 9, 0, 30, 1)  --> 反向查找 arr[9]~arr[0] 中首个跳变点
 */
uint8 find_jump_point(uint8 *array_value, uint8 num0, uint8 num1, uint8 jump_num, uint8 model)
{
    uint8 i       = 0;
    uint8 start   = 0;      // 搜索起始索引
    uint8 end     = 0;      // 搜索终止索引
    uint8 diff    = 0;      // 灰度差值
    int8  step    = 0;      // 遍历步长（+1 或 -1）
    uint8 point   = 0xFF;   // 返回值，默认 0xFF 表示未找到

    /* ---- 根据 model 确定搜索方向 ---- */
    if(model == 0)
    {
        /* 正向遍历：索引从小到大 */
        if(num0 > num1) return point;             // 参数非法，直接返回
        start = num0;
        end   = num1;
        step  = 1;
    }
    else
    {
        /* 反向遍历：索引从大到小 */
        if(num0 < num1) return point;             // 参数非法，直接返回
        start = num0;
        end   = num1;
        step  = -1;
    }

    /* ---- 遍历数组，查找跳变点 ---- */
    for(i = start; i != end; i += step)
    {
        /* 计算当前点与前一点的灰度差（取绝对值） */
        if(i == start)
        {
            /* 第一个点：与下一个点比较，作为起始基准 */
            diff = (array_value[i] > array_value[i + step])
                   ? (array_value[i] - array_value[i + step])
                   : (array_value[i + step] - array_value[i]);

            if(diff >= jump_num)
            {
                point = i;                          // 记录跳变点索引
                break;
            }
        }
        else
        {
            /* 后续点：与前一个点比较 */
            diff = (array_value[i] > array_value[i - step])
                   ? (array_value[i] - array_value[i - step])
                   : (array_value[i - step] - array_value[i]);

            if(diff >= jump_num)
            {
                point = i;                          // 记录跳变点索引
                break;
            }
        }
    }

    return point;                                  // 返回跳变点索引，未找到为 0xFF
}


uint8 mid_weight_list[130] = {
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
    3,3,3,3,3,3,3,3,3,3,
    7,8,9,10,11,12,13,14,15,16,
    17,18,19,20,20,20,20,19,18,17,
    16,15,14,13,12,11,10,9,8,7,
    6,6,6,6,6,6,6,6,6,6,
    1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,
};

uint8 final_mid_line = MID_W;    // 最终输出中线值
uint8 last_mid_line = MID_W;     // 上次中线值

/****************************************************************************************************
函数功能：根据权重求中线值（整数优化版）
函数名称：uint8 find_mid_line_weight(void)
输入参数：无
输出参数：中线值
备  注：权重可通过图像权重组调节，无浮点运算
****************************************************************************************************/
uint8 find_mid_line_weight(void)
{
    uint8 i;
    uint32 weight_midline_sum = 0;   // 加权中线累加值
    uint32 weight_sum = 0;           // 权重累加值
    uint8 mid_line; 
	  uint8 mid_line_value;
    for(i = MT9V03X_H - 1; i > search_end_line; i--)
    {
        weight_midline_sum += (uint32)mid_line_list[i] * mid_weight_list[i];
        weight_sum += mid_weight_list[i];
    }

    // 加权平均，整数除法
    mid_line = (uint8)(weight_midline_sum / weight_sum);

    // 互补滤波：last_mid_line*0.5 + mid_line*0.5 等价于 (last_mid_line + mid_line)/2
    mid_line_value = (uint8)(((uint16)last_mid_line + mid_line) >> 1);

    // 限幅
    mid_line_value = Limit_uint8(MID_W - mid_line_limit, mid_line_value, MID_W + mid_line_limit);

    last_mid_line = mid_line_value;
    return mid_line_value;
}

/****************************************************************************************************
函数功能：计算每行中线（整数优化版）
函数名称：void calculate_mid_line(void)
输入参数：无
输出参数：无
备  注：使用整数除法/移位，避免浮点
****************************************************************************************************/
//void calculate_mid_line(void)
//{
//    int i;
//    const int NEAR_BOUNDARY = SEARCH_IMAGE_H * 2 / 3; // 80行，近端/远端分界
//    for (i = 0; i < SEARCH_IMAGE_H; i++)
//    {
//        // ====================================================
//        // 环岛阶段 且 远端行（行号小 = 图像上方）
//        // ====================================================
//        if ((ring_step == RING_FOUND      ||
//             ring_step == RING_LEFT_LOST  ||
//             ring_step == RING_ARC_SEARCH ||
//             ring_step == RING_EXITING)
//            && i < NEAR_BOUNDARY)
//        {
//            // 远端只用右边界推算中线
//            if (!right_lost(i))
//            {
//                mid_line_list[i] = (uint8)((int)right_edge_line[i]
//                                 - STANDARD_HALF_WIDTH[i]);
//            }
//            else
//            {
//                if (i > 0)
//                    mid_line_list[i] = mid_line_list[i - 1];
//                else
//                    mid_line_list[i] = MID_W;
//            }
//        }
//        // ====================================================
//        // 正常情况 或 近端行：左右平均
//        // ====================================================
//        else
//        {
//            if (right_edge_line[i] > left_edge_line[i])
//            {
//                mid_line_list[i] = (left_edge_line[i]
//                                  + right_edge_line[i]) >> 1;
//            }
//            else
//            {
//                if (i > 0)
//                    mid_line_list[i] = mid_line_list[i - 1];
//                else
//                    mid_line_list[i] = MID_W;
//            }
//        }
//        // 限幅
//        mid_line_list[i] = Limit_uint8(1, mid_line_list[i],
//                                       SEARCH_IMAGE_W - 2);
//    }
//}
// calculate_mid_line 只负责算原始中线，不管环岛
void calculate_mid_line(void)
{
    int i;
    for(i = 0; i < SEARCH_IMAGE_H; i++)
    {
        if(right_edge_line[i] > left_edge_line[i])
        {
            mid_line_list[i] = (left_edge_line[i] + right_edge_line[i]) >> 1;
        }
        else
        {
            if(i > 0)
                mid_line_list[i] = mid_line_list[i - 1];
            else
                mid_line_list[i] = MID_W;
        }
        mid_line_list[i] = Limit_uint8(1, mid_line_list[i], SEARCH_IMAGE_W - 2);
    }
}


//void calculate_mid_line(void)
//{
//    int i;
//    for(i = 0; i < SEARCH_IMAGE_H; i++)
//    {
//        if(right_edge_line[i] > left_edge_line[i])
//        {
//            // 中点 = (左+右)/2，使用整数除法（右移1位更快）
//            mid_line_list[i] = (left_edge_line[i] + right_edge_line[i]) >> 1;
//        }
//        else
//        {
//            if(i > 0)
//            {
//                mid_line_list[i] = mid_line_list[i - 1];
//            }
//            else
//            {
//                mid_line_list[i] = MID_W;
//            }
//        }
//        mid_line_list[i] = Limit_uint8(1, mid_line_list[i], SEARCH_IMAGE_W - 2);
//    }
//}
