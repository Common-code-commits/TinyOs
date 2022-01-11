/* asmhead.nas */
/* 
   如果存在结构体是被某个魔数地址带来的数据直接赋值，
   该结构体保存为操作系统信息
*/
#ifndef _BOOKPACK_H
#define _BOOKPACK_H

typedef struct { /* 0x0ff0-0x0fff */
	char cyls; /* 启动区读磁盘读到此为止 */
	char leds; /* 启动时键盘的LED的状态 */
	char vmode; /* 显卡模式为多少位彩色 */
	char reserve;
	short scrnx, scrny; /* 画面分辨率 */
	char *vram;
}BOOTINFO;

#define ADR_BOOTINFO	0x00000ff0

/* naskfunc.nas */
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
void io_stihlt(void);


/* graphic.c */
typedef struct{
	unsigned char Red;	       // 颜色的红色部分
	unsigned char Green;	   // 颜色的绿色部分
	unsigned char Blue;	       // 颜色的蓝色部分
} RGB;

typedef struct{
	int width;                 
	int height;                
} RECT;

#define FILL_COLOR		     7      // 填充默认色号
#define BORDER_COLOR	     1      // 边框默认色号
#define FONT_COLOR		     0      // 字体默认色号
#define MOUSE_FILL_COLOR     7      // 鼠标填充默认色号
#define SCREEN_COLOR	     3      // 屏幕背景默认色号
#define MOUSE_BORDER_COLOR   0      // 鼠标边框默认色号

#define MOUSE_SIZE      16          // 鼠标大小

#define PALETTE_SIZE  256           // 调色板大小
#define PALETTE_INIT_SIZE 16        // 调色板初始颜色数量

void set_fill_color(RGB *rgb);
void fillrectangle( int x0, int y0, int x1, int y1);
void putfont(int x, int y, char *font);
void putfonts8_asc(int x, int y,unsigned char *s);
void init_mouse_cursor(char *mouse);
void putblock8_8(int px0,int py0,char *buf);
void init_screen(RGB *rgb);
void set_border_color(RGB *rgb);
void set_font_color(RGB *rgb);
void set_screen_color(RGB *rgb);
RECT* get_screen_src(void);
/* dsctbl.c */

/* 屏幕信息结构体 */
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

void init_gdtidt(void);
void set_segmdesc(SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e

/* int.c */
void init_pic(void);
void inthandler21(int *esp);
void inthandler27(int *esp);
void inthandler2c(int *esp);
int get_keyboard_status(void);
int get_mouse_status(void);
int get_key(void);
int get_mouse(void);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

/* fifo.c */
typedef struct {
	unsigned char *buf;
	int p, q, size, free, flags;
} FIFO8;

typedef struct {
	unsigned char buf[3], phase;
	int x, y, btn;
} MOUSE_DEC;

void fifo8_init(FIFO8 *fifo, int size, unsigned char *buf);
int fifo8_put(FIFO8 *fifo, unsigned char data);
int fifo8_get(FIFO8 *fifo);
int fifo8_status(FIFO8 *fifo);
void wait_KBC_sendready(void);
void enable_mouse(MOUSE_DEC *mdec);
int mouse_decode(MOUSE_DEC *mdec, unsigned char dat);
void init_keyboard(void);

#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

#endif
