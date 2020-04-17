#include "spi.h"
#include "spi_oled.h"

int  main()
{	
	//使用SPI_A接口调试
	unsigned char uc_cnt;
	oled_init(); //初始化TLC5615
	OLED_DIsp_Clear(); //清屏
	//OLED_DIsp_All();

#if 0//单独测试
	OLED_DIsp_String(0,0,"1234",8);
	OLED_DIsp_String(0,2,"1234",16);
	OLED_DIsp_String(0,4,"1234",24);
	OLED_DIsp_CHinese(40,0,0);
#endif

	OLED_DIsp_Test(0,0);
	return 0;
}
