#include <zf_common_headfile.h>



//限制b的大小范围在a到c，b为输出
uint8 Limit_uint8( uint8 a, uint8 b, uint8 c )
{
    if( ( b >= a ) && ( b <= c ) )
    {
        return b;
    }
    else if( b < a )
    {
        return a;
    }
    else if( b > c )
    {
        return c;
    }
    return 0;
}
int Limit_int( int a, int b, int c )
{
    if( ( b >= a ) && ( b <= c ) )
    {
        return b;
    }
    else if( b < a )
    {
        return a;
    }
    else if( b > c )
    {
        return c;
    }
    return 0;
    }

    /**
 * @brief  查找数组指定范围内的极值（最大值或最小值）
 * @param  array_value : 指向目标数组的指针
 * @param  num0        : 查找起始索引
 * @param  num1        : 查找结束索引
 * @param  model       : 模式选择  0-查找最大值  1-查找最小值
 * @return uint8       : 返回查找到的极值
 * @example find_extreme_value(arr, 0, 9, 0)  --> 查找 arr[0]~arr[9] 中的最大值
 * @example find_extreme_value(arr, 0, 9, 1)  --> 查找 arr[0]~arr[9] 中的最小值
 */
uint8 find_extreme_value(uint8 *array_value, uint8 num0, uint8 num1, uint8 model)
{
    uint8 i      = 0;
    uint8 temp   = 0;   /* 用于交换索引的临时变量     */
    uint8 temp1  = 0;   /* 用于记录最大值             */
    uint8 temp2  = 0;   /* 用于记录最小值             */
    uint8 value  = 0;   /* 最终返回的极值             */

    /* ---- 确保起始索引 <= 结束索引，若不满足则交换 ---- */
    if(num0 > num1)
    {
        temp = num0;
        num0 = num1;
        num1 = temp;
    }

    /* ---- 初始化：以第一个元素作为比较基准 ---- */
    temp1 = array_value[num0];   /* 初始最大值 */
    temp2 = array_value[num0];   /* 初始最小值 */

    /* ---- 第一个循环：遍历数组，查找范围内最大值 ---- */
    for(i = num0; i <= num1; i++)
    {
        if(array_value[i] > temp1)
        {
            temp1 = array_value[i];   /* 更新最大值 */
        }
    }

    /* ---- 第二个循环：遍历数组，查找范围内最小值 ---- */
    for(i = num0; i <= num1; i++)
    {
        if(array_value[i] < temp2)
        {
            temp2 = array_value[i];   /* 更新最小值 */
        }
    }

    /* ---- 根据 model 返回对应极值 ---- */
    if(model == 0)
    {
        value = temp1;   /* model = 0，返回最大值 */
    }
    else
    {
        value = temp2;   /* model = 1，返回最小值 */
    }

    return value;
}

//
// 函数简介     连接两点 用于边界补线
// 参数说明    array_value       数组指针
// 参数说明    num0            起始范围
// 参数说明    num1            终止范围
// 返回类型    void
// 使用示例    connect_point(left_edge_line, 73, 25);
// 备注信息
//
void connect_point(uint8 *array_value, uint8 num0, uint8 num1)
{   int i;
    float point_1 = (float)array_value[num0];
    float point_2 = (float)array_value[num1];
    float temp_slope = (point_2 - point_1) / (num0 - num1);

    for( i= 0; i < (num0 - num1); i ++)
    {
        array_value[num0 - i] = (int8)(temp_slope * i) + array_value[num0];
    }
}


/**
 * @brief  查找数组指定范围内的极值（最大值或最小值）所在的索引
 * @param  array_value : 指向目标数组的指针
 * @param  num0        : 查找起始索引
 * @param  num1        : 查找结束索引
 * @param  model       : 模式选择  0-查找最大值的索引  1-查找最小值的索引
 * @return uint8       : 返回查找到的极值所在的索引
 */
uint8 find_extreme_index(uint8 *array_value, uint8 num0, uint8 num1, uint8 model)
{
    // 所有变量统一在函数最开头定义（C89 强制要求）
    uint8 i;
    uint8 temp;
    uint8 extreme_value;
    uint8 extreme_index;

    // 交换起始/结束索引，保证 num0 <= num1
    if(num0 > num1) {
        temp = num0;
        num0 = num1;
        num1 = temp;
    }

    // 初始化极值和索引
    extreme_value = array_value[num0];
    extreme_index = num0;

    // 遍历查找
    for(i = num0 + 1; i <= num1; i++)
    {
        if(model == 0) // 找最大值
        {
            if(array_value[i] > extreme_value)
            {
                extreme_value = array_value[i];
                extreme_index = i;
            }
        }
        else // 找最小值
        {
            if(array_value[i] < extreme_value)
            {
                extreme_value = array_value[i];
                extreme_index = i;
            }
        }
    }
    return extreme_index;
}
