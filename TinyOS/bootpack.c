#include "bootpack.h"
#include <stdio.h>

void HariMain(void)
{
	char mcursor[256];

	RGB rgb = {0xb7,0xae,0x8f};	
	init_screen(&rgb);

	putfonts8_asc(8,  8, "Chinese");
	putfonts8_asc(31, 31, "Haribote OS.");
	putfonts8_asc(30, 30, "Haribote OS.");

	init_gdtidt();
	init_pic();
	io_sti();                           /* IDT/PIC的初始化已经完成，于是开放CPU的中断 */

	/* 显示鼠标 */
	init_mouse_cursor(mcursor);
	putblock8_8(mcursor);

	io_out8(PIC0_IMR, 0xf9);            /* 开放PIC1和键盘中断(11111001) */
	io_out8(PIC1_IMR, 0xef);            /* 开放鼠标中断(11101111) */

	for (;;) {
		io_hlt();
	}
}

