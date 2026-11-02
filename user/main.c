#include <zf_common_headfile.h>

#define INCLUDE_BOUNDARY_TYPE   1

#define WIFI_SSID_TEST          "TY"
#define WIFI_PASSWORD_TEST      "2504564649" // 锟斤拷锟斤拷锟揭�锟斤拷锟接碉拷WIFI 没锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷要锟斤拷 锟斤拷锟斤拷 锟芥换为 NULL
volatile uint16 laser_timer = 0;

// 锟竭斤拷牡锟斤拷锟斤拷锟皆讹拷锟斤拷锟酵硷拷锟竭度ｏ拷锟斤拷锟节憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
#define BOUNDARY_NUM            (MT9V03X_H * 3 / 2)

// 锟斤拷糯锟斤拷锟斤拷锟斤拷锟斤拷透锟斤拷图

// 只锟斤拷X锟竭斤拷
uint8 xy_x1_boundary[BOUNDARY_NUM], xy_x2_boundary[BOUNDARY_NUM], xy_x3_boundary[BOUNDARY_NUM];

// 只锟斤拷Y锟竭斤拷
uint8 xy_y1_boundary[BOUNDARY_NUM], xy_y2_boundary[BOUNDARY_NUM], xy_y3_boundary[BOUNDARY_NUM];

// X Y锟竭界都锟角碉拷锟斤拷指锟斤拷锟斤拷
uint8 x1_boundary[MT9V03X_H], x2_boundary[MT9V03X_H], x3_boundary[MT9V03X_H];
uint8 y1_boundary[MT9V03X_W], y2_boundary[MT9V03X_W], y3_boundary[MT9V03X_W];

// 图锟今备凤拷锟斤拷锟介，锟节凤拷锟斤拷前锟斤拷图锟今备凤拷锟劫斤拷锟叫凤拷锟酵ｏ拷锟斤拷锟斤拷锟斤拷锟皆憋拷锟斤拷图锟斤拷锟斤拷锟剿猴拷训锟斤拷锟斤拷锟�
uint8 far image_copy[MT9V03X_H][MT9V03X_W];
static int frame_count = 0; 
//void print_half_width_array(void) {
//  int i;
//    printf("\n\n// ======= 锟诫复锟斤拷锟斤拷锟斤拷锟斤拷锟轿达拷锟斤拷锟芥换锟斤拷锟斤拷锟斤拷锟� STANDARD_HALF_WIDTH 锟斤拷锟斤拷 =======\n");
//    printf("const int STANDARD_HALF_WIDTH[%d] = {\n    ", SEARCH_IMAGE_H);
//    
//    for (i = 0; i < SEARCH_IMAGE_H; i++) {
//        // 锟斤拷锟斤拷锟斤拷实锟斤拷锟� = 锟揭憋拷锟斤拷 - 锟斤拷锟斤拷
//        int half_width = right_edge_line[i] - mid_line_list[i];
//        
//        // 锟捷达拷锟斤拷锟斤拷锟斤拷锟斤拷锟矫簧�锟斤拷锟竭ｏ拷锟斤拷锟斤拷一锟叫碉拷值锟斤拷锟斤拷
//        if (half_width <= 0 || half_width > 150) { 
//            if (i > 0) half_width = right_edge_line[i-1] - mid_line_list[i-1];
//            else half_width = 15; 
//        }
//        
//        printf("%d", half_width);
//        if (i != SEARCH_IMAGE_H - 1) printf(", ");
//        if ((i + 1) % 10 == 0 && i != SEARCH_IMAGE_H - 1) printf("\n    ");
//    }
//    printf("\n};\n// ==================================================================\n\n");
//}


void main (void)
{   uint8 is_crossroad = 0;
    uint8 cross_road_start_row = 0;
	  int k;
	  uint8 ring_detected = 0;
    uint8 ring_col = 0, ring_row = 0;
	  int c;
	  int print_cnt=0;
	  uint16 last_dist = 0;
	  int i;
	  uint16 speed;
	  int cs = 3;
	  uint16 ring_cooldown = 0;  // 鍐峰嵈璁℃椂鍣�
	
	
#if(1 == INCLUDE_BOUNDARY_TYPE || 2 == INCLUDE_BOUNDARY_TYPE || 4 == INCLUDE_BOUNDARY_TYPE)
    i = 0;
#elif(3 == INCLUDE_BOUNDARY_TYPE)
    int32 j = 0;
#endif
    clock_init(SYSTEM_CLOCK_96M); 				// 时锟斤拷锟斤拷锟矫硷拷系统锟斤拷始锟斤拷<锟斤拷乇锟斤拷锟�>
    debug_init();                       		// 锟斤拷锟皆达拷锟斤拷锟斤拷息锟斤拷始锟斤拷

    while(wifi_spi_init(WIFI_SSID_TEST, WIFI_PASSWORD_TEST))
    {
        printf("\r\n connect wifi failed. \r\n");
        system_delay_ms(100);                                                   // 锟斤拷始锟斤拷失锟斤拷 锟饺达拷 100ms
    }

    printf("\r\n module version:%s",wifi_spi_version);                          // 模锟斤拷碳锟斤拷姹�
    printf("\r\n module mac    :%s",wifi_spi_mac_addr);                         // 模锟斤拷 MAC 锟斤拷息
    printf("\r\n module ip     :%s",wifi_spi_ip_addr_port);                     // 模锟斤拷 IP 锟斤拷址

    // zf_device_wifi_spi.h 锟侥硷拷锟节的宏定锟斤拷锟斤拷愿锟斤拷锟侥ｏ拷锟斤拷锟斤拷锟�(锟斤拷锟斤拷) WIFI 之锟斤拷锟角凤拷锟皆讹拷锟斤拷锟斤拷 TCP 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 UDP 锟斤拷锟接★拷锟斤拷锟斤拷 TCP 锟斤拷锟斤拷锟斤拷锟饺诧拷锟斤拷
    if(1 != WIFI_SPI_AUTO_CONNECT)                                              // 锟斤拷锟矫伙拷锌锟斤拷锟斤拷远锟斤拷锟斤拷锟� 锟斤拷锟斤拷要锟街讹拷锟斤拷锟斤拷目锟斤拷 IP
    {
        while(wifi_spi_socket_connect(                                          // 锟斤拷指锟斤拷目锟斤拷 IP 锟侥端口斤拷锟斤拷 TCP 锟斤拷锟斤拷
            "TCP",                                                              // 指锟斤拷使锟斤拷TCP锟斤拷式通讯
            WIFI_SPI_TARGET_IP,                                                 // 指锟斤拷远锟剿碉拷IP锟斤拷址锟斤拷锟斤拷写锟斤拷位锟斤拷锟斤拷IP锟斤拷址
            WIFI_SPI_TARGET_PORT,                                               // 指锟斤拷远锟剿的端口号ｏ拷锟斤拷写锟斤拷位锟斤拷锟侥端口号ｏ拷通锟斤拷锟斤拷位锟斤拷默锟斤拷锟斤拷8080
            WIFI_SPI_LOCAL_PORT))                                               // 指锟斤拷锟斤拷锟斤拷锟侥端口猴拷
        {
            // 锟斤拷锟揭恢憋拷锟斤拷锟绞э拷锟� 锟斤拷锟斤拷一锟斤拷锟角诧拷锟斤拷没锟叫斤拷硬锟斤拷锟斤拷位
            printf("\r\n Connect TCP Servers error, try again.");
            system_delay_ms(100);                                               // 锟斤拷锟斤拷锟斤拷锟斤拷失锟斤拷 锟饺达拷 100ms
        }
    }

    // 使锟矫革拷锟斤拷 WIFI SPI模锟斤拷时锟睫凤拷使锟斤拷锟斤拷幕锟斤拷锟斤拷为锟斤拷锟斤拷锟叫癸拷锟矫ｏ拷
    // 使锟矫革拷锟斤拷 WIFI SPI模锟斤拷时锟睫凤拷使锟斤拷锟斤拷幕锟斤拷锟斤拷为锟斤拷锟斤拷锟叫癸拷锟矫ｏ拷
    // 使锟矫革拷锟斤拷 WIFI SPI模锟斤拷时锟睫凤拷使锟斤拷锟斤拷幕锟斤拷锟斤拷为锟斤拷锟斤拷锟叫癸拷锟矫ｏ拷


    // 锟狡硷拷锟饺筹拷始锟斤拷锟斤拷锟斤拷头锟斤拷锟斤拷锟绞硷拷锟斤拷锟斤拷锟斤拷锟斤拷
    while(mt9v03x_init())
    {
        system_delay_ms(100); 
		printf("\r\n mt9v03x_init error, try again.");
    }

    // 使锟矫革拷锟斤拷 WIFI SPI模锟斤拷时锟睫凤拷使锟斤拷锟斤拷幕锟斤拷锟斤拷为锟斤拷锟斤拷锟叫癸拷锟矫ｏ拷
    // 使锟矫革拷锟斤拷 WIFI SPI模锟斤拷时锟睫凤拷使锟斤拷锟斤拷幕锟斤拷锟斤拷为锟斤拷锟斤拷锟叫癸拷锟矫ｏ拷
    // 使锟矫革拷锟斤拷 WIFI SPI模锟斤拷时锟睫凤拷使锟斤拷锟斤拷幕锟斤拷锟斤拷为锟斤拷锟斤拷锟叫癸拷锟矫ｏ拷

    // 锟斤拷锟斤拷锟斤拷殖锟绞硷拷锟� 锟斤拷锟捷达拷锟斤拷使锟矫革拷锟斤拷WIFI SPI
    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_WIFI_SPI);

    // 锟斤拷锟揭�锟斤拷锟斤拷图锟斤拷锟斤拷息锟斤拷锟斤拷锟斤拷氐锟斤拷锟絪eekfree_assistant_camera_information_config锟斤拷锟斤拷锟斤拷锟叫憋拷要锟侥诧拷锟斤拷锟斤拷锟斤拷
    // 锟斤拷锟斤拷锟揭�锟斤拷锟酵憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟絪eekfree_assistant_camera_boundary_config锟斤拷锟斤拷锟斤拷锟矫憋拷锟竭碉拷锟斤拷息

#if(0 == INCLUDE_BOUNDARY_TYPE)
    // 锟斤拷锟斤拷锟斤拷锟斤拷锟酵硷拷锟斤拷锟较�(锟斤拷锟斤拷锟斤拷原始图锟斤拷锟斤拷息)
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);


#elif(1 == INCLUDE_BOUNDARY_TYPE)
    // 锟斤拷锟斤拷锟斤拷锟斤拷锟酵硷拷锟斤拷锟较�(锟斤拷锟揭帮拷锟斤拷锟斤拷锟斤拷锟竭斤拷锟斤拷息锟斤拷锟竭斤拷锟斤拷息只锟斤拷锟叫猴拷锟斤拷锟斤拷锟疥，锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷图锟斤拷叨鹊玫锟斤拷锟斤拷锟轿讹拷锟矫匡拷锟斤拷呓锟斤拷锟揭伙拷锟斤拷锟街伙拷锟斤拷锟揭伙拷锟斤拷锟�)
    // 锟皆边斤拷锟斤拷锟斤拷写锟斤拷锟斤拷锟斤拷
    for(i = 0; i < MT9V03X_H; i++)
    {
        x1_boundary[i] =left_edge_line[i];
        x2_boundary[i] = mid_line_list[i];
        x3_boundary[i] = right_edge_line[i];
    }
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);
   seekfree_assistant_camera_boundary_config(X_BOUNDARY, MT9V03X_H, x1_boundary, x2_boundary, x3_boundary, NULL, NULL ,NULL);


#elif(2 == INCLUDE_BOUNDARY_TYPE)
    // 锟斤拷锟斤拷锟斤拷锟斤拷锟酵硷拷锟斤拷锟较�(锟斤拷锟揭帮拷锟斤拷锟斤拷锟斤拷锟竭斤拷锟斤拷息锟斤拷锟竭斤拷锟斤拷息只锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟疥，锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷图锟斤拷锟斤拷鹊玫锟斤拷锟斤拷锟轿讹拷锟矫匡拷锟斤拷呓锟斤拷锟揭伙拷锟斤拷锟街伙拷锟斤拷锟揭伙拷锟斤拷锟�)
    // 通锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷使锟斤拷锟斤拷锟斤拷
    // 锟皆边斤拷锟斤拷锟斤拷写锟斤拷锟斤拷锟斤拷
    for(i = 0; i < MT9V03X_W; i++)
    {
        y1_boundary[i] = i * MT9V03X_H / MT9V03X_W;
        y2_boundary[i] = MT9V03X_H / 2;
        y3_boundary[i] = (MT9V03X_W - i) * MT9V03X_H / MT9V03X_W;
    }
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);
    seekfree_assistant_camera_boundary_config(Y_BOUNDARY, MT9V03X_W, NULL, NULL ,NULL, y1_boundary, y2_boundary, y3_boundary);

   


#elif(3 == INCLUDE_BOUNDARY_TYPE)
    // 锟斤拷锟斤拷锟斤拷锟斤拷锟酵硷拷锟斤拷锟较�(锟斤拷锟揭帮拷锟斤拷锟斤拷锟斤拷锟竭斤拷锟斤拷息锟斤拷锟竭斤拷锟斤拷息锟斤拷锟叫猴拷锟斤拷锟斤拷锟斤拷锟斤拷)
    // 锟斤拷锟斤拷锟侥凤拷式锟斤拷锟斤拷实锟街讹拷锟斤拷锟叫伙拷锟斤拷谋呓锟斤拷锟绞�
    j = 0;
    for(i = MT9V03X_H - 1; i >= MT9V03X_H / 2; i--)
    {
        // 直锟竭诧拷锟斤拷
        xy_x1_boundary[j] = 34;
        xy_y1_boundary[j] = i;

        xy_x2_boundary[j] = 47;
        xy_y2_boundary[j] = i;

        xy_x3_boundary[j] = 60;
        xy_y3_boundary[j] = i;
        j++;
    }

    for(i = MT9V03X_H / 2 - 1; i >= 0; i--)
    {
        // 直锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
        xy_x1_boundary[j] = 34 + (MT9V03X_H / 2 - i) * (MT9V03X_W / 2 - 34) / (MT9V03X_H / 2);
        xy_y1_boundary[j] = i;

        xy_x2_boundary[j] = 47 + (MT9V03X_H / 2 - i) * (MT9V03X_W / 2 - 47) / (MT9V03X_H / 2);
        xy_y2_boundary[j] = 15 + i * 3 / 4;

        xy_x3_boundary[j] = 60 + (MT9V03X_H / 2 - i) * (MT9V03X_W / 2 - 60) / (MT9V03X_H / 2);
        xy_y3_boundary[j] = 30 + i / 2;
        j++;
    }

    for(i = 0; i < MT9V03X_H / 2; i++)
    {
        // 锟斤拷锟戒部锟斤拷
        xy_x1_boundary[j] = MT9V03X_W / 2 + i * (138 - MT9V03X_W / 2) / (MT9V03X_H / 2);
        xy_y1_boundary[j] = i;

        xy_x2_boundary[j] = MT9V03X_W / 2 + i * (133 - MT9V03X_W / 2) / (MT9V03X_H / 2);
        xy_y2_boundary[j] = 15 + i * 3 / 4;

        xy_x3_boundary[j] = MT9V03X_W / 2 + i * (128 - MT9V03X_W / 2) / (MT9V03X_H / 2);
        xy_y3_boundary[j] = 30 + i / 2;
        j++;
    }
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, image_copy[0], MT9V03X_W, MT9V03X_H);
    seekfree_assistant_camera_boundary_config(XY_BOUNDARY, BOUNDARY_NUM, xy_x1_boundary, xy_x2_boundary, xy_x3_boundary, xy_y1_boundary, xy_y2_boundary, xy_y3_boundary);


#elif(4 == INCLUDE_BOUNDARY_TYPE)
    // 锟斤拷锟斤拷锟斤拷锟斤拷锟酵硷拷锟斤拷锟较�(锟斤拷锟揭帮拷锟斤拷锟斤拷锟斤拷锟竭斤拷锟斤拷息锟斤拷锟竭斤拷锟斤拷息只锟斤拷锟叫猴拷锟斤拷锟斤拷锟疥，锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷图锟斤拷叨鹊玫锟斤拷锟斤拷锟轿讹拷锟矫匡拷锟斤拷呓锟斤拷锟揭伙拷锟斤拷锟街伙拷锟斤拷锟揭伙拷锟斤拷锟�)
    // 锟皆边斤拷锟斤拷锟斤拷写锟斤拷锟斤拷锟斤拷
    for(i = 0; i < MT9V03X_H; i++)
    {
        x1_boundary[i] = 70 - (70 - 20) * i / MT9V03X_H;
        x2_boundary[i] = MT9V03X_W / 2;
        x3_boundary[i] = 118 + (168 - 118) * i / MT9V03X_H;
    }
    seekfree_assistant_camera_information_config(SEEKFREE_ASSISTANT_MT9V03X, NULL, MT9V03X_W, MT9V03X_H);
    seekfree_assistant_camera_boundary_config(X_BOUNDARY, MT9V03X_H, x1_boundary, x2_boundary, x3_boundary, NULL, NULL ,NULL);


   #endif
    EA = 0;             // 锟截憋拷锟斤拷锟叫讹拷
    init();             // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟接诧拷锟斤拷锟绞硷拷锟斤拷锟斤拷锟�
    EA = 1;


    // 锟剿达拷锟斤拷写锟矫伙拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷锟绞硷拷锟斤拷锟斤拷锟斤拷
    while(1)
    {	
        if(mt9v03x_finish_flag)
        {
           mt9v03x_finish_flag = 0;
            // 锟节凤拷锟斤拷前锟斤拷图锟今备凤拷锟劫斤拷锟叫凤拷锟酵ｏ拷锟斤拷锟斤拷锟斤拷锟皆憋拷锟斤拷图锟斤拷锟斤拷锟剿猴拷训锟斤拷锟斤拷锟�
           memcpy(image_copy[0], mt9v03x_image[0], MT9V03X_IMAGE_SIZE);
					
           // 1. 锟斤拷锟斤拷图锟斤拷识锟斤拷
           get_reference_point(image_copy[0]); 
				   search_reference_col(image_copy[0]);
           search_line(image_copy[0]);
					
           //cross_analysis();
					
					
					 // 2. 锟斤拷锟竭硷拷锟斤拷锟诫环锟斤拷锟斤拷锟斤拷

        calculate_mid_line();  // 3. 锟斤拷锟斤拷锟斤拷
        process_ring();        // 2. 锟斤拷锟斤拷状态锟斤拷锟斤拷锟斤拷锟斤拷 ring_arc_row锟斤拷
        draw_arc_top_line(image_copy[0]);
      

			
         ring_detected = detect_ring(image_copy[0], &ring_col, &ring_row);//锟斤拷锟皆�
		
					if (is_crossroad)
           {   
               // 锟斤拷锟界，锟斤拷小锟斤拷锟斤拷锟斤拷直锟斤拷通锟斤拷十锟斤拷路锟斤拷
               // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷一锟斤拷锟斤拷志位锟斤拷锟矫讹拷锟斤拷锟斤拷锟斤拷锟绞憋拷锟斤拷锟斤拷锟斤拷呒锟斤拷悖�锟斤拷锟街达拷锟轿�0
               final_mid_line = 98; // 强锟斤拷锟斤拷锟斤拷锟斤拷图锟斤拷锟斤拷锟斤拷
               
               // 锟斤拷锟竭ｏ拷为锟剿匡拷锟接伙拷锟斤拷锟斤拷锟角匡拷锟斤拷锟斤拷锟斤拷位锟斤拷锟较憋拷浅锟绞�锟斤拷路锟节碉拷锟斤拷始锟斤拷
               // 锟斤拷锟界画一锟斤拷锟斤拷锟斤拷
            
               if(cross_road_start_row > 0 && cross_road_start_row < MT9V03X_H)
               {
                   for(c = 0; c < MT9V03X_W; c++)
                   {
                       image_copy[cross_road_start_row][c] = 0; // 锟斤拷一锟斤拷锟斤拷锟斤拷
                   }
               }
           }
					 
					 
					 
					 	for(i = 0; i < MT9V03X_H; i++)

        {
            x1_boundary[i] = left_edge_line[i];    // 使锟斤拷 left_edge_line
            x2_boundary[i] =mid_line_list[i];     // 使锟斤拷 calculate_mid_line() 锟侥斤拷锟�
            x3_boundary[i] = right_edge_line[i];   // 使锟斤拷 right_edge_line
				}
				
				
				
// 鍐峰嵈璁℃椂鍣ㄩ�掑噺
if (ring_cooldown > 0) ring_cooldown--;

if (ring_detected && ring_row >= 50 && ring_row <= 90 && ring_cooldown == 0)
{
    speed = (motor_l.encoder_speed + motor_r.encoder_speed) / 2;
    if (speed < 10) speed = 10;

    laser_timer = 10;

    // 鏍规嵁鍦嗗績鍒楀潗鏍囬�夋嫨婵�鍏夊彛
    if (ring_col >= 16 && ring_col <= 50)
        gpio_high(IO_P01);      // 宸�1婵�鍏夛紙鍦嗗績43锛�
    else if (ring_col >= 51 && ring_col <= 65)
        gpio_high(IO_P02);      // 宸�2婵�鍏夛紙鍦嗗績57锛�
    else if (ring_col >= 66 && ring_col <= 81)
        gpio_high(IO_P67);      // 涓�闂存縺鍏夛紙鍦嗗績73锛�
    else if (ring_col >= 82 && ring_col <= 96)
        gpio_high(IO_P03);      // 鍙�2婵�鍏夛紙鍦嗗績90锛�
    else if (ring_col >= 97 && ring_col <= 139)
        gpio_high(IO_P06);      // 鍙�1婵�鍏夛紙鍦嗗績102锛�

    // 鐢诲崄瀛楁爣璁板渾蹇�
    for (k = -cs; k <= cs; k++)
    {
        if (ring_col + k >= 0 && ring_col + k < MT9V03X_W)
            image_copy[ring_row][ring_col + k] = 0x00;
        if (ring_row + k >= 0 && ring_row + k < MT9V03X_H)
            image_copy[ring_row + k][ring_col] = 0x00;
    }

    ring_cooldown = 50;  // 鍐峰嵈50甯э紙绾�1绉掞級
}

    seekfree_assistant_camera_send();
				
				
        // 6. 锟斤拷锟狡讹拷锟�
				final_mid_line=find_mid_line_weight();
				dynamic_pid_value_set();
				servo(final_mid_line);//80
				
   
		
        }
				
		
		
    }
}

