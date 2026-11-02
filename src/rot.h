#ifndef __ROT_H_
#define __ROT_H_
#include <zf_common_headfile.h>
extern const short mapx_s[30][47];
extern const short mapy_s[30][47];




float Xrot_point(uint8 x,uint8 y);
float Yrot_point(uint8 x, uint8 y);
float inv_rot_x(float x, float y);
float inv_rot_y(float x, float y);
float XShowRotimg(uint8 x,uint8 y);
float YShowRotimg(uint8 x,uint8 y);

#endif