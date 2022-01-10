/* 关于绘图部分的处理 */

#include "bootpack.h"
#define PALLETE_SIZE  256
#define PALLETE_INIT_SIZE 16

BOOTINFO *binfo = (BOOTINFO *) ADR_BOOTINFO;   /* 屏幕信息变量赋值 */
char *vram;                                    /* 创建地址变量    */

int palette_head       = PALLETE_INIT_SIZE;
int font_color_index   = FONT_COLOR;
int fill_color_index   = FILL_COLOR;
int border_color_index = BORDER_COLOR;
int screen_color_index = SCREEN_COLOR;

void init_palette(void)
{
	/* 设置调色盘默认颜色 */
	static unsigned char table_rgb[PALLETE_INIT_SIZE * 3] = {
		0x00, 0x00, 0x00,	/*  0:黑 */
		0xff, 0x00, 0x00,	/*  1:亮红 */
		0x00, 0xff, 0x00,	/*  2:亮绿 */
		0xff, 0xff, 0x00,	/*  3:亮黄 */
		0x00, 0x00, 0xff,	/*  4:亮蓝 */
		0xff, 0x00, 0xff,	/*  5:亮紫 */
		0x00, 0xff, 0xff,	/*  6:浅亮蓝 */
		0xff, 0xff, 0xff,	/*  7:白 */
		0xc6, 0xc6, 0xc6,	/*  8:亮灰 */
		0x84, 0x00, 0x00,	/*  9:暗红 */
		0x00, 0x84, 0x00,	/* 10:暗绿 */
		0x84, 0x84, 0x00,	/* 11:暗黄 */
		0x00, 0x00, 0x84,	/* 12:暗青 */
		0x84, 0x00, 0x84,	/* 13:暗紫 */
		0x00, 0x84, 0x84,	/* 14:浅暗蓝 */
		0x84, 0x84, 0x84	/* 15:暗灰 */
	};
	set_palette(0, 15, table_rgb);
	return;

	/* C语言中的static char语句只能用于数据，相当于汇编中的DB指令 */
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();	/* 记录中断许可标志的值 */
	io_cli(); 					/* 将中断许可标志置为0,禁止中断 */
	io_out8(0x03c8, start);

	/* 根据VGA显卡要求，R、G、B三种颜色的值时只能用低6位来指定 */
	/* 除四操作就是算数右移两位，由于颜色分量为unsigned char，算数右移和逻辑右移是一样的 */
    /* 将颜色存入调色板 */
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4); 
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}

	io_store_eflags(eflags);	/* 复原中断许可标志 */
	return;
}

/* 设置填充颜色 */
void set_fill_color(RGB *rgb)
{
	/* 调色盘中的0号颜色就是填充图案中的颜色，默认为白色*/
	unsigned char temp_color[3] = { rgb->Red, rgb->Green, rgb->Blue };
	unsigned char* temp_rgb = temp_color;

	if ( rgb )
	{
		if ( palette_head < PALLETE_SIZE - 1)
		{
			fill_color_index = ++palette_head;
		}
		else
		{
			palette_head = PALLETE_INIT_SIZE;
			fill_color_index = palette_head;
		}

	}
	else
	{
		fill_color_index = FILL_COLOR;
	}
	set_palette(fill_color_index,fill_color_index,temp_rgb);

}

/* 设置边框颜色 */
void set_border_color(RGB *rgb)
{
	/* 调色盘中的1号颜色就是边框颜色，默认为亮红色*/
	unsigned char temp_color[3] = { rgb->Red, rgb->Green, rgb->Blue };
	unsigned char* temp_rgb = temp_color;
	if ( rgb )
	{
		if ( palette_head < PALLETE_SIZE - 1)
		{
			border_color_index = ++palette_head;
		}
		else
		{
			palette_head = PALLETE_INIT_SIZE;
			border_color_index = palette_head;
		}
	}
	else
	{
		border_color_index = BORDER_COLOR;
	}
	set_palette(border_color_index,border_color_index,temp_rgb);
}

/* 设置字体颜色 */
void set_font_color(RGB *rgb)
{
	/* 调色盘中的2号颜色就是字体颜色， 默认为白色*/
	unsigned char temp_color[3] = { rgb->Red, rgb->Green, rgb->Blue };
	unsigned char* temp_rgb = temp_color;

	
	if ( rgb )
	{
		if ( palette_head < PALLETE_SIZE - 1)
		{
			font_color_index = ++palette_head;
		}
		else
		{
			palette_head = PALLETE_INIT_SIZE;
			font_color_index = palette_head;
		}

	}
	else
	{
		font_color_index = FONT_COLOR;
	}
	set_palette(font_color_index,font_color_index,temp_rgb);
}

/* 设置屏幕背景颜色 */
void set_screen_color(RGB *rgb)
{
	/* 调色盘中的4号颜色就是屏幕背景颜色, 默认为亮蓝色*/
	unsigned char temp_color[3] = { rgb->Red, rgb->Green, rgb->Blue };
	unsigned char* temp_rgb = temp_color;
	if ( rgb )
	{
		if ( palette_head < PALLETE_SIZE - 1)
		{
			screen_color_index = ++palette_head;
		}
		else
		{
			palette_head = PALLETE_INIT_SIZE;
			screen_color_index = palette_head;
		}
	}
	else
	{
		screen_color_index = SCREEN_COLOR;
	}
	set_palette(screen_color_index,screen_color_index,temp_rgb);
}

/* 画填充矩形 */
void fillrectangle(int x0, int y0, int x1, int y1)
{

	int x, y;
	int xsize = binfo->scrnx;
	/* 根据 0xa0000 + x + y * 320 计算坐标 8*/
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = fill_color_index;
	}

	return;
}

void init_screen(RGB *rgb)
{
	int x = 0;
	int y = binfo->scrny;
	int xsize = binfo->scrnx;
	vram = binfo->vram; 

	init_palette(); /* 设定调色板 */
	
	set_screen_color(rgb);

	for (; y >= 0; y--) {
		for (x = xsize; x >= 0; x--)
			vram[y * xsize + x] = screen_color_index;
	}

	return;
}

void putfont( int x, int y,char *font)
{
	int xsize = binfo->scrnx;
	char *p, d ;
	int i;
	for (i = 0; i < PALLETE_INIT_SIZE; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = font_color_index; }
		if ((d & 0x40) != 0) { p[1] = font_color_index; }
		if ((d & 0x20) != 0) { p[2] = font_color_index; }
		if ((d & 0x10) != 0) { p[3] = font_color_index; }
		if ((d & 0x08) != 0) { p[4] = font_color_index; }
		if ((d & 0x04) != 0) { p[5] = font_color_index; }
		if ((d & 0x02) != 0) { p[6] = font_color_index; }
		if ((d & 0x01) != 0) { p[7] = font_color_index; }
	}
	return;
}

void putfonts8_asc(int x, int y, unsigned char *s)
{
	extern char hankaku[4096];
	/* C语言中，字符串都是以0x00结尾 */
	for (; *s != 0x00; s++) {
		putfont(x, y,hankaku + *s * PALLETE_INIT_SIZE);
		x += 8;
	}
	return;
}

void init_mouse_cursor(char *mouse)
/* 准备鼠标光标（16x16） */
{
	static char cursor[PALLETE_INIT_SIZE][PALLETE_INIT_SIZE] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};
	int x, y;

	for (y = 0; y < PALLETE_INIT_SIZE; y++) {
		for (x = 0; x < PALLETE_INIT_SIZE; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * PALLETE_INIT_SIZE + x] = MOUSE_BORDER_COLOR;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * PALLETE_INIT_SIZE + x] = MOUSE_FILL_COLOR;
			}
			if (cursor[y][x] == '.') {
				mouse[y * PALLETE_INIT_SIZE + x] = screen_color_index;
			}
		}
	}
	return;
}

void putblock8_8( char *buf )
{
	int vxsize = binfo->scrnx;
	int pxsize = MOUSE_SIZE, pysize = MOUSE_SIZE;
	int bxsize = MOUSE_SIZE;

	int px0 = (binfo->scrnx - PALLETE_INIT_SIZE) / 2;       /* 计算画面的中心坐标*/
	int py0 =  (binfo->scrny - PALLETE_INIT_SIZE) / 2;

	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}
