#include <zf_common_headfile.h>




/****************************************************************************************************
函数功能：边线和中线显示
函数名称：void draw_line(void)
输入参数：无
输出参数：无
备  注：调用此函数后在屏幕上显示左右边线以及中线
****************************************************************************************************/
void draw_line(void)
{uint8 i;
    for(i=MT9V03X_H - 1;i > search_end_line;i--)
    {
       ips200_draw_point (Limit_int(1,left_edge_line[i],MT9V03X_W-2),i,RGB565_BLUE);
       ips200_draw_point (Limit_int(1,right_edge_line[i],MT9V03X_W-2),i,RGB565_GREEN);
       ips200_draw_point (Limit_int(1,mid_line_list[i],MT9V03X_W-2),i,RGB565_RED);
    }
}
