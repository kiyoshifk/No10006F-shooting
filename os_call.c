#define UP    0x112
#define DOWN  0x113
#define RIGHT 0x114
#define LEFT  0x115
#define ESC   0x1b
#define F1    0x180
#define F2    0x181
#define F3    0x182
#define F4    0x183
#define F5    0x184
#define F6    0x185
#define F7    0x186
#define F8    0x187
#define F9    0x188
#define F10   0x189
#define F11   0x18a
#define F12   0x18b
#define HOME  0x18c
#define END   0x18d
#define PGUP  0x18e
#define PGDN  0x18f
#define DEL   0x190


int disp_str     		= 0xa0006400;
int ut_gets      		= 0xa0006404;
int ut_getc      		= 0xa0006408;
int ut_putc      		= 0xa000640c;
int wait_ms      		= 0xa0006410;
int Pset         		= 0xa0006414;
int Pget         		= 0xa0006418;
int GetTickCount 		= 0xa000641c;
int cursor_set   		= 0xa0006420;
int display_xn   		= 0xa0006424;
int printf       		= 0xa0006428;
int SYS_FS_FileOpen		= 0xa000642c;
int SYS_FS_FileClose	= 0xa0006430;
int SYS_FS_FileRead		= 0xa0006434;
int SYS_FS_FileWrite	= 0xa0006438;
int ut_error_msg		= 0xa000643c;
int get_APP_chA			= 0xa0006440;
int buzzer				= 0xa0006444;	// void buzzer(int Hz, int msec);
int buzzer_wait			= 0xa0006448;	// void buzzer_wait(int Hz, int msec);


int scrn_clear()
{
	memset((char*)0xa0000000, 0, 0x6400);
	cursor_set(0,0);
}
