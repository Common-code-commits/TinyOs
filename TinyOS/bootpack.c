void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int  io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void fillrectangle(int xsize, int x0, int y0, int x1, int y1);

typedef struct{
	unsigned char Red;	       // 颜色的红色部分
	unsigned char Green;	   // 颜色的绿色部分
	unsigned char Blue;	       // 颜色的蓝色部分
} RGB;

void HariMain(void)
{
	int xsize, ysize;

	init_palette(); /* 设定调色板 */
	xsize = 320;
	ysize = 200;

	RGB rgb = { 0xff, 0x00, 0x00};
	setfillcolor(rgb);

	for (;;) {
		io_hlt();
	}
}

void init_palette(void)
{
	/* 设置调色盘默认颜色 */
	static unsigned char table_rgb[16 * 3] = {
		0xff, 0xff, 0xff,	/*  0:白 */
		0xff, 0x00, 0x00,	/*  1:亮红 */
		0x00, 0xff, 0x00,	/*  2:亮绿 */
		0xff, 0xff, 0x00,	/*  3:亮黄 */
		0x00, 0x00, 0xff,	/*  4:亮蓝 */
		0xff, 0x00, 0xff,	/*  5:亮紫 */
		0x00, 0xff, 0xff,	/*  6:浅亮蓝 */
		0x00, 0x00, 0x00,	/*  7:白 */
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

	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4); 
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	/* 复原中断许可标志 */
	return;
}

void setfillcolor(RGB rgb)
{
	/* 调色盘中的0号颜色就是填充图案中的颜色 默认为黑色*/
	unsigned char temp_color[3] = { rgb.Red, rgb.Green, rgb.Blue };
	unsigned char* temp_rgb = temp_color;
	set_palette(0,0,temp_rgb);
}

/* 画矩形 */
void fillrectangle(int xsize, int x0, int y0, int x1, int y1)
{
	char *vram = (char *) 0xa0000;    /* 地址变量赋值 */

	int x, y;

	/* 根据 0xa0000 + x + y * 320 计算坐标 8*/
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = 0;
	}

	return;
}
