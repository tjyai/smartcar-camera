/* ============================================================================
 * 摄像头组智能车 —— 主循环
 *
 * 每帧流程：
 *   取图 → 自适应阈值 → 基准列定位 → 搜线（左右边线）→ 加权中线
 *        → 环岛状态机 → 圆目标检测 / 激光选通 → 舵机输出
 *
 * 图像与边界通过 WiFi-SPI 实时回传到上位机（逐飞助手），用于调参。
 * ========================================================================== */
#include <zf_common_headfile.h>

/* ------------------------------ WiFi 配置 ------------------------------ */
#define WIFI_SSID_TEST          "TY"
#define WIFI_PASSWORD_TEST      "2504564649"    /* 无密码时填 NULL */

volatile uint16 laser_timer = 0;                /* 激光保持计时（5ms 节拍） */

/* 上发给上位机的边界点数组大小：图像高度的 1.5 倍 */
#define BOUNDARY_NUM            (MT9V03X_H * 3 / 2)

/* 上发给上位机的三条边界（左 / 中 / 右） */
uint8 x1_boundary[MT9V03X_H], x2_boundary[MT9V03X_H], x3_boundary[MT9V03X_H];

/* 图像备份：先拷贝再发送，避免发送过程中被下一帧覆盖 */
uint8 far image_copy[MT9V03X_H][MT9V03X_W];

static int frame_count = 0;

void main(void)
{
    uint8  is_crossroad         = 0;
    uint8  cross_road_start_row = 0;
    uint8  ring_detected        = 0;
    uint8  ring_col = 0, ring_row = 0;
    uint16 ring_cooldown        = 0;    /* 激光冷却计时，避免同一目标重复触发 */
    uint16 speed;
    int    i, k, c;
    int    cs = 3;                      /* 圆心十字标记的半长 */

    /* ---------------------- 系统与外设基础初始化 ---------------------- */
    clock_init(SYSTEM_CLOCK_96M);       /* 时钟配置，系统初始化 */
    debug_init();                       /* 调试串口 */

    /* ---------------------- 连接 WiFi 图传模块 ---------------------- */
    while (wifi_spi_init(WIFI_SSID_TEST, WIFI_PASSWORD_TEST))
    {
        printf("\r\n connect wifi failed. \r\n");
        system_delay_ms(100);
    }

    printf("\r\n module version:%s", wifi_spi_version);
    printf("\r\n module mac    :%s", wifi_spi_mac_addr);
    printf("\r\n module ip     :%s", wifi_spi_ip_addr_port);

    /* 未开启自动连接时，手动建立到上位机的 TCP 连接 */
    if (1 != WIFI_SPI_AUTO_CONNECT)
    {
        while (wifi_spi_socket_connect("TCP", WIFI_SPI_TARGET_IP,
                                       WIFI_SPI_TARGET_PORT, WIFI_SPI_LOCAL_PORT))
        {
            printf("\r\n Connect TCP Servers error, try again.");
            system_delay_ms(100);
        }
    }

    /* 注意：使用 WiFi-SPI 模块时不能同时使用屏幕，二者引脚复用 */

    /* ---------------------- 摄像头初始化 ---------------------- */
    /* 必须先于上位机初始化，否则图像通道没有数据源 */
    while (mt9v03x_init())
    {
        system_delay_ms(100);
        printf("\r\n mt9v03x_init error, try again.");
    }

    /* 上位机接口初始化，数据走 WiFi-SPI */
    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_WIFI_SPI);

    /* 配置要下发的图像与边界信息 */
    for (i = 0; i < MT9V03X_H; i++)
    {
        x1_boundary[i] = left_edge_line[i];
        x2_boundary[i] = mid_line_list[i];
        x3_boundary[i] = right_edge_line[i];
    }
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X,
                                                 image_copy[0], MT9V03X_W, MT9V03X_H);
    seekfree_assistant_camera_boundary_config(X_BOUNDARY, MT9V03X_H,
                                              x1_boundary, x2_boundary, x3_boundary,
                                              NULL, NULL, NULL);

    /* ---------------------- 外设初始化 ---------------------- */
    EA = 0;                             /* 关总中断，防止初始化被打断 */
    init();
    EA = 1;                             /* 开总中断 */

    /* ---------------------- 主循环 ---------------------- */
    while (1)
    {
        if (mt9v03x_finish_flag)
        {
            mt9v03x_finish_flag = 0;

            /* 先备份图像再发送，避免发送过程中被下一帧覆盖造成撕裂 */
            memcpy(image_copy[0], mt9v03x_image[0], MT9V03X_IMAGE_SIZE);

            /* 1. 图像处理：自适应阈值 → 基准列 → 左右边线 */
            get_reference_point(image_copy[0]);
            search_reference_col(image_copy[0]);
            search_line(image_copy[0]);

            /* 2. 中线 + 环岛状态机 + 圆弧顶线绘制 */
            calculate_mid_line();
            process_ring();
            draw_arc_top_line(image_copy[0]);

            /* 3. 圆目标检测（用于激光打靶） */
            ring_detected = detect_ring(image_copy[0], &ring_col, &ring_row);

            /* 4. 十字：强制中线走图像中心，直行通过 */
            if (is_crossroad)
            {
                final_mid_line = 98;    /* 强制拉到图像中线 */

                /* 在图上画一条横线，标记十字起始行，便于上位机观察 */
                if (cross_road_start_row > 0 && cross_road_start_row < MT9V03X_H)
                {
                    for (c = 0; c < MT9V03X_W; c++)
                    {
                        image_copy[cross_road_start_row][c] = 0;
                    }
                }
            }

            /* 5. 把本帧边界填入上发数组 */
            for (i = 0; i < MT9V03X_H; i++)
            {
                x1_boundary[i] = left_edge_line[i];
                x2_boundary[i] = mid_line_list[i];
                x3_boundary[i] = right_edge_line[i];
            }

            /* 6. 激光打靶：按目标圆心所在列选择对应的激光口 */
            if (ring_cooldown > 0) ring_cooldown--;

            if (ring_detected && ring_row >= 50 && ring_row <= 90 && ring_cooldown == 0)
            {
                speed = (motor_l.encoder_speed + motor_r.encoder_speed) / 2;
                if (speed < 10) speed = 10;

                laser_timer = 10;

                if      (ring_col >= 16  && ring_col <= 50)  gpio_high(IO_P01);  /* 左1 */
                else if (ring_col >= 51  && ring_col <= 65)  gpio_high(IO_P02);  /* 左2 */
                else if (ring_col >= 66  && ring_col <= 81)  gpio_high(IO_P67);  /* 中  */
                else if (ring_col >= 82  && ring_col <= 96)  gpio_high(IO_P03);  /* 右2 */
                else if (ring_col >= 97  && ring_col <= 139) gpio_high(IO_P06);  /* 右1 */

                /* 在图上画十字标记圆心 */
                for (k = -cs; k <= cs; k++)
                {
                    if (ring_col + k >= 0 && ring_col + k < MT9V03X_W)
                        image_copy[ring_row][ring_col + k] = 0x00;
                    if (ring_row + k >= 0 && ring_row + k < MT9V03X_H)
                        image_copy[ring_row + k][ring_col] = 0x00;
                }

                ring_cooldown = 50;     /* 冷却 50 帧，约 1 秒 */
            }

            /* 7. 本帧图像与边界发给上位机 */
            seekfree_assistant_camera_send();

            /* 8. 加权中线 → 动态 PID → 舵机 */
            final_mid_line = find_mid_line_weight();
            dynamic_pid_value_set();
            servo(final_mid_line);
        }
    }
}
