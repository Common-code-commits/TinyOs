
/* 
   如果存在结构体是被某个魔数地址带来的数据直接赋值，
   该结构体保存为操作系统信息
*/

/* 屏幕信息结构体 */
typedef struct {
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
} BOOTINFO;

typedef struct {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
} SEGMENT_DESCRIPTOR;

typedef struct{
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
} GATE_DESCRIPTOR;

typedef struct{
	unsigned char Red;	       // 颜色的红色部分
	unsigned char Green;	   // 颜色的绿色部分
	unsigned char Blue;	       // 颜色的蓝色部分
} RGB;

#define FILL_COLOR		     0      // 填充默认色号
#define BORDER_COLOR	     1      // 边框默认色号
#define FONT_COLOR		     2      // 字体默认色号
#define MOUSE_FILL_COLOR     3      // 鼠标填充默认色号
#define SCREEN_COLOR	     4      // 屏幕背景默认色号
#define MOUSE_BORDER_COLOR   5      // 鼠标边框默认色号

#define MOUSE_SIZE      16     // 鼠标大小

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int  io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void setfillcolor(RGB rgb);
void fillrectangle( int x0, int y0, int x1, int y1);
void putfont(int x, int y, char *font);
void putfonts8_asc(int x, int y,unsigned char *s);
void init_mouse_cursor(char *mouse);
void putblock8_8(int px0, int py0, char *buf);
void init_screen(RGB rgb);

void init_gdtidt(void);
void set_segmdesc(SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

static BOOTINFO *binfo = (BOOTINFO *) 0x0ff0;   /* 屏幕信息变量赋值 */
static char *vram;                              /* 地址变量赋值    */

void HariMain(void)
{
	char mcursor[256];
	int mx, my;

	RGB rgb = {0xb7,0xae,0x8f};	
	init_screen(rgb);

	/* 显示鼠标 */
	mx = (binfo->scrnx - 16) / 2; /* 计算画面的中心坐标*/
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor(mcursor);
	putblock8_8(mx, my, mcursor);

	putfonts8_asc(8,  8, "Chinese");
	putfonts8_asc(31, 31, "Haribote OS.");
	putfonts8_asc(30, 30, "Haribote OS.");

	for (;;) {
		io_hlt();
	}
}

void init_palette(void)
{
	/* 设置调色盘默认颜色 */
	static unsigned char table_rgb[16 * 3] = {
		0xff, 0xff, 0xff,	/*  0:白   填充默认颜色*/
		0xff, 0x00, 0x00,	/*  1:亮红 边框默认颜色*/
		0xff, 0xff, 0xff,	/*  2:白   字体默认颜色*/
		0xff, 0xff, 0xff,	/*  3:白   鼠标填充默认颜色*/
		0x00, 0x00, 0xff,	/*  4:亮蓝 屏幕默认背景颜色 */
		0x00, 0x00, 0x00,	/*  5:黑   鼠标边框默认颜色*/
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
void set_fill_color(RGB rgb)
{
	/* 调色盘中的0号颜色就是填充图案中的颜色，默认为白色*/
	unsigned char temp_color[3] = { rgb.Red, rgb.Green, rgb.Blue };
	unsigned char* temp_rgb = temp_color;
	set_palette(FILL_COLOR,FILL_COLOR,temp_rgb);
}

/* 设置边框颜色 */
void set_border_color(RGB rgb)
{
	/* 调色盘中的1号颜色就是边框颜色，默认为亮红色*/
	unsigned char temp_color[3] = { rgb.Red, rgb.Green, rgb.Blue };
	unsigned char* temp_rgb = temp_color;
	set_palette(BORDER_COLOR,BORDER_COLOR,temp_rgb);
}

/* 设置字体颜色 */
void set_font_color(RGB rgb)
{
	/* 调色盘中的2号颜色就是字体颜色， 默认为白色*/
	unsigned char temp_color[3] = { rgb.Red, rgb.Green, rgb.Blue };
	unsigned char* temp_rgb = temp_color;
	set_palette(FONT_COLOR,FONT_COLOR,temp_rgb);
}

/* 设置屏幕背景颜色 */
void set_screen_color(RGB rgb)
{
	/* 调色盘中的4号颜色就是屏幕背景颜色, 默认为亮蓝色*/
	unsigned char temp_color[3] = { rgb.Red, rgb.Green, rgb.Blue };
	unsigned char* temp_rgb = temp_color;
	set_palette(SCREEN_COLOR,SCREEN_COLOR,temp_rgb);
}

/* 画填充矩形 */
void fillrectangle(int x0, int y0, int x1, int y1)
{

	int x, y;
	int xsize = binfo->scrnx;
	/* 根据 0xa0000 + x + y * 320 计算坐标 8*/
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = FILL_COLOR;
	}

	return;
}

void init_screen(RGB rgb)
{
	int x = 0;
	int y = binfo->scrny;
	int xsize = binfo->scrnx;
	vram = binfo->vram; 

	init_palette(); /* 设定调色板 */
	
	set_screen_color(rgb);

	for (; y >= 0; y--) {
		for (x = xsize; x >= 0; x--)
			vram[y * xsize + x] = SCREEN_COLOR;
	}

	return;
}

void putfont( int x, int y,char *font)
{
	int xsize = binfo->scrnx;
	char *p, d ;
	int i;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = FONT_COLOR; }
		if ((d & 0x40) != 0) { p[1] = FONT_COLOR; }
		if ((d & 0x20) != 0) { p[2] = FONT_COLOR; }
		if ((d & 0x10) != 0) { p[3] = FONT_COLOR; }
		if ((d & 0x08) != 0) { p[4] = FONT_COLOR; }
		if ((d & 0x04) != 0) { p[5] = FONT_COLOR; }
		if ((d & 0x02) != 0) { p[6] = FONT_COLOR; }
		if ((d & 0x01) != 0) { p[7] = FONT_COLOR; }
	}
	return;
}

void putfonts8_asc(int x, int y, unsigned char *s)
{
	extern char hankaku[4096];
	/* C语言中，字符串都是以0x00结尾 */
	for (; *s != 0x00; s++) {
		putfont(x, y,hankaku + *s * 16);
		x += 8;
	}
	return;
}

void init_mouse_cursor(char *mouse)
/* 准备鼠标光标（16x16） */
{
	static char cursor[16][16] = {
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

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = MOUSE_BORDER_COLOR;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = MOUSE_FILL_COLOR;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = SCREEN_COLOR;
			}
		}
	}
	return;
}

void putblock8_8( int px0, int py0, char *buf)
{
	int vxsize = binfo->scrnx;
	int pxsize = MOUSE_SIZE, pysize = MOUSE_SIZE;
	int bxsize = MOUSE_SIZE;

	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}

void init_gdtidt(void)
{
	SEGMENT_DESCRIPTOR *gdt = (SEGMENT_DESCRIPTOR *) 0x00270000;
	GATE_DESCRIPTOR    *idt = (GATE_DESCRIPTOR    *) 0x0026f800;
	int i;

	/* GDT初始化 */
	for (i = 0; i < 8192; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
	load_gdtr(0xffff, 0x00270000);

	/* IDT初始化 */
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7ff, 0x0026f800);

	return;
}

void set_segmdesc(SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
