

/********************************************************************************/
/*		disp  ‰ð‘œ“x 448x320													*/
/********************************************************************************/
int disp_hex_sub(int data)
{
	data &= 0xf;
	
	if(data<10)
		return '0'+data;
	else
		return 'A'+data-10;
}

int disp_hexA(int data)
{
	ut_putc(disp_hex_sub(data>>28));
	ut_putc(disp_hex_sub(data>>24));
	ut_putc(disp_hex_sub(data>>20));
	ut_putc(disp_hex_sub(data>>16));
	ut_putc(disp_hex_sub(data>>12));
	ut_putc(disp_hex_sub(data>>8));
	ut_putc(disp_hex_sub(data>>4));
	ut_putc(disp_hex_sub(data));
//	ut_putc('\n');
}

int disp_hex(int data)
{
	disp_hexA(data);
	ut_putc('\n');
}
/********************************************************************************/
/*		l_to_a																	*/
/********************************************************************************/
char l_to_a_buf[20];

char *l_to_a(unsigned int l)
{
	int i;
	
	l_to_a_buf[20-1] = 0;
	for(i=20-2; ; i=i-1){
		l_to_a_buf[i] = l % 10 + '0';
		l = l/10;
		if(l==0)
			return (l_to_a_buf + i);
	}
}
/********************************************************************************/
/*		disp_dec																*/
/********************************************************************************/
int disp_decA(int num)
{
	if(num < 0){
		ut_putc('-');
		num = 0-num;
	}
	disp_str(l_to_a(num));
//	ut_putc('\n');
}

int disp_dec(int num)
{
	disp_decA(num);
	ut_putc('\n');
}
