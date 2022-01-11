#include "bootpack.h"
#include <stdio.h>

void HariMain(void)
{
	char mcursor[256];

	RGB rgb = {0xb7,0xae,0x8f};	
	init_screen(&rgb);

	init_gdtidt();
	init_pic();
	io_sti();                           /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */

	RECT* screen = get_screen_src();
	int mx =  ( screen->width - PALETTE_INIT_SIZE ) / 2;       /* 计算画面的中心坐标*/
	int my =  ( screen->height - PALETTE_INIT_SIZE ) / 2;

	/* 显示鼠标 */
	init_mouse_cursor(mcursor);
	putblock8_8(mx, my,mcursor);

	io_out8(PIC0_IMR, 0xf9);            /* 开放PIC1和键盘中断(11111001) */
	io_out8(PIC1_IMR, 0xef);            /* 开放鼠标中断(11101111) */

	MOUSE_DEC mdec; 
	enable_mouse(&mdec);

	int i;
	char s[40];
	for (;;) {
		io_cli();
		if (get_keyboard_status() + get_mouse_status() == 0) {
			io_stihlt();
		} else {
			if (get_keyboard_status() != 0) {
				i = get_key();
				io_sti();
				sprintf(s, "%02X", i);
				fillrectangle(0, 16, 15, 31);
				putfonts8_asc(0, 16, s);
			} else if (get_mouse_status() != 0) {
				i = get_mouse();
				io_sti();
				if (mouse_decode(&mdec, i) != 0) {
					 /* 3字节都凑齐了，所以把它们显示出来*/
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					fillrectangle(32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(32, 16, s);
					/* 鼠标指针的移动 */
					fillrectangle(mx, my, mx + 15, my + 15); /* 隐藏鼠标 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > screen->width - 16) {
						mx = screen->width - 16;
					}
					if (my > screen->height - 16) {
						my = screen->height - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					fillrectangle( 0, 0, 79, 15); /* 隐藏坐标 */
					putfonts8_asc(0, 0, s); /* 显示坐标 */
					putblock8_8(mx, my, mcursor); /* 描画鼠标 */
				}
			}
		}
	}

}

