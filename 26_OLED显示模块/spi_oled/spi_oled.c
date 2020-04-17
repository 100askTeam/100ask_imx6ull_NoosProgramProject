#include "spi.h"
#include "spi_oled.h"
#include "font.h"
/*GPIO4_20用于控制指令和数据写入的切换，拉高为数据，拉低为指令*/
static volatile unsigned int *GPIO4_GDIR_s;
static volatile unsigned int *GPIO4_DR_s;  
//为0 表示命令，为1表示数据
#define OLED_CMD 	0
#define OLED_DATA 	1
		  			 		  						  					  				 	   		  	  	 	  
/**********************************************************************
	 * 函数名称： oled_write_cmd
	 * 功能描述： oled向特定地址写入数据或者命令
	 * 输入参数：@uc_data :要写入的数据
	 			@uc_cmd:为1则表示写入数据，为0表示写入命令
	 * 输出参数：无
	 * 返 回 值： 无
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/04		 V1.0	  芯晓		  创建
 ***********************************************************************/
void oled_write_cmd_data(unsigned char uc_data,unsigned char uc_cmd)
{

	unsigned char uc_read=0;
	if(uc_cmd==0)
	{
		*GPIO4_DR_s &= ~(1<<20);//拉低，表示写入指令
	}
	else
	{
		*GPIO4_DR_s |= (1<<20);//拉高，表示写入数据
	}
	spi_writeread(ESCPI1_BASE,uc_data);//写入
}
/**********************************************************************
	 * 函数名称： oled_init
	 * 功能描述： oled_init的初始化，包括SPI控制器得初始化
	 * 输入参数：无
	 * 输出参数： 初始化的结果
	 * 返 回 值： 成功则返回0，否则返回-1
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/15		 V1.0	  芯晓		  创建
 ***********************************************************************/
unsigned char oled_init(void)
{
	unsigned char uc_dev_id = 0;

	GPIO4_GDIR_s = (volatile unsigned int *)(0x20a8000 + 0x4);
	GPIO4_DR_s   = (volatile unsigned int *)(0x20a8000);  
	spi_init(ESCPI1_BASE);
		  			 		  						  					  				 	   		  	  	 	  
	oled_write_cmd_data(0xae,OLED_CMD);//关闭显示

	oled_write_cmd_data(0x00,OLED_CMD);//设置 lower column address
	oled_write_cmd_data(0x10,OLED_CMD);//设置 higher column address

	oled_write_cmd_data(0x40,OLED_CMD);//设置 display start line

	oled_write_cmd_data(0xB0,OLED_CMD);//设置page address

	oled_write_cmd_data(0x81,OLED_CMD);// contract control
	oled_write_cmd_data(0x66,OLED_CMD);//128

	oled_write_cmd_data(0xa1,OLED_CMD);//设置 segment remap

	oled_write_cmd_data(0xa6,OLED_CMD);//normal /reverse

	oled_write_cmd_data(0xa8,OLED_CMD);//multiple ratio
	oled_write_cmd_data(0x3f,OLED_CMD);//duty = 1/64

	oled_write_cmd_data(0xc8,OLED_CMD);//com scan direction

	oled_write_cmd_data(0xd3,OLED_CMD);//set displat offset
	oled_write_cmd_data(0x00,OLED_CMD);//

	oled_write_cmd_data(0xd5,OLED_CMD);//set osc division
	oled_write_cmd_data(0x80,OLED_CMD);//

	oled_write_cmd_data(0xd9,OLED_CMD);//ser pre-charge period
	oled_write_cmd_data(0x1f,OLED_CMD);//

	oled_write_cmd_data(0xda,OLED_CMD);//set com pins
	oled_write_cmd_data(0x12,OLED_CMD);//

	oled_write_cmd_data(0xdb,OLED_CMD);//set vcomh
	oled_write_cmd_data(0x30,OLED_CMD);//

	oled_write_cmd_data(0x8d,OLED_CMD);//set charge pump disable 
	oled_write_cmd_data(0x14,OLED_CMD);//

	oled_write_cmd_data(0xaf,OLED_CMD);//set dispkay on
	
	

	return 0;
}		  			 		  						  					  				 	   		  	  	 	  
/**********************************************************************
	 * 函数名称： oled_fill_data
	 * 功能描述： 整个屏幕显示填充某个固定数据
	 * 输入参数：@fill_Data：要填充的数据
	 * 输出参数： 填充结果
	 * 返 回 值： 成功则返回0
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/15		 V1.0	  芯晓		  创建
 ***********************************************************************/
unsigned char oled_fill_data(unsigned char fill_Data)
{
	unsigned char x,y;
	for(x=0;x<8;x++)
	{
	
		oled_write_cmd_data(0xb0+x,OLED_CMD);		//page0-page1
		oled_write_cmd_data(0x00,OLED_CMD);		//low column start address
		oled_write_cmd_data(0x10,OLED_CMD);		//high column start address	
		for(y=0;y<128;y++)
		{
					oled_write_cmd_data(fill_Data,OLED_DATA);//填充数据
		}				
	}
	return 0;
}
/**********************************************************************
	 * 函数名称： OLED_DIsp_Clear
	 * 功能描述： 整个屏幕显示数据清0
	 * 输入参数：无
	 * 输出参数： 无
	 * 返 回 值： 
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/15		 V1.0	  芯晓		  创建
 ***********************************************************************/
void OLED_DIsp_Clear(void)  
{  //GRAM清零
	unsigned char i,n;		    
	for(i=0;i<8;i++)  
	{  
		oled_write_cmd_data (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		oled_write_cmd_data (0x00,OLED_CMD);      //设置显示位置—列低地址
		oled_write_cmd_data (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)
			oled_write_cmd_data(0,OLED_DATA); //填充数据0
	} //更新显示
}
/**********************************************************************
	 * 函数名称： OLED_DIsp_All
	 * 功能描述： 整个屏幕显示全部点亮，可以用于检查坏点
	 * 输入参数：无
	 * 输出参数：无 
	 * 返 回 值：
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/15		 V1.0	  芯晓		  创建
 ***********************************************************************/
void OLED_DIsp_All(void)  
{  
	unsigned char i,n;		    
	for(i=0;i<8;i++)  
	{  
		oled_write_cmd_data (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		oled_write_cmd_data (0x00,OLED_CMD);      //设置显示位置—列低地址
		oled_write_cmd_data (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)oled_write_cmd_data(0xff,OLED_DATA); //填充数据0xff
	} //更新显示
}
//坐标设置
/**********************************************************************
	 * 函数名称： OLED_DIsp_Set_Pos
	 * 功能描述：设置要显示的位置
	 * 输入参数：@ x ：要显示的column address
	 			@y :要显示的page address
	 * 输出参数： 无
	 * 返 回 值： 
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/15		 V1.0	  芯晓		  创建
 ***********************************************************************/
void OLED_DIsp_Set_Pos(unsigned char x, unsigned char y) 
{ 	oled_write_cmd_data(0xb0+y,OLED_CMD);
	oled_write_cmd_data((x&0x0f),OLED_CMD); 
	oled_write_cmd_data(((x&0xf0)>>4)|0x10,OLED_CMD);
}   	      	   			 
/**********************************************************************
	  * 函数名称： OLED_DIsp_Char
	  * 功能描述：在某个位置显示字符 1-9
	  * 输入参数：@ x ：要显示的column address
		 			@y :要显示的page address
		 			@chr ：要显示的字符的ascii码
		 			@Char_Size：要显示的字符的大小，值为8/16/24三个大小
	  * 输出参数： 无
	  * 返 回 值： 
	  * 修改日期		版本号	  修改人 	   修改内容
	  * -----------------------------------------------
	  * 2020/03/15		  V1.0	   芯晓		   创建
***********************************************************************/
void OLED_DIsp_Char(unsigned char x,unsigned char y,unsigned char chr,unsigned char Char_Size)
{      	
	unsigned char c=0,i=0;	
	c=chr-0x31;//得到偏移后的值			
	if(Char_Size ==16)
	{
		OLED_DIsp_Set_Pos(x,y);	
		for(i=0;i<8;i++)
		{	//显示上半截字符
			oled_write_cmd_data(asc2_1608[c][i*2],OLED_DATA);
		}
		OLED_DIsp_Set_Pos(x,y+1);
		for(i=0;i<8;i++)
		{	//显示下半截字符
			oled_write_cmd_data(asc2_1608[c][i*2+1],OLED_DATA);			
		}
	}
	else if(Char_Size ==24)
	{
		OLED_DIsp_Set_Pos(x,y);	
		for(i=0;i<8;i++)
		{//显示上半截字符	
			oled_write_cmd_data(asc2_2408[c][i*3],OLED_DATA);
		}
		OLED_DIsp_Set_Pos(x,y+1);
		for(i=0;i<8;i++)
		{//显示中间半截字符	
			oled_write_cmd_data(asc2_2408[c][i*3+1],OLED_DATA);			
		}
		OLED_DIsp_Set_Pos(x,y+2);
		for(i=0;i<8;i++)
		{//显示下半截字符
			oled_write_cmd_data(asc2_2408[c][i*3+2],OLED_DATA);			
		}
	}
	else {	
		OLED_DIsp_Set_Pos(x,y);
		for(i=0;i<8;i++)
			oled_write_cmd_data(asc2_0808[c][i],OLED_DATA);		
	}
}
/**********************************************************************
	 * 函数名称： OLED_DIsp_String
	 * 功能描述： 在指定位置显示字符串
	 * 输入参数：@ x ：要显示的column address
		 			@y :要显示的page address
		 			@chr ：要显示的字符串，由数字1-9组成
		 			@Char_Size：要显示的字符的大小，值为8/16/24三个大小
	 * 输出参数： 无
	 * 返 回 值： 无
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/15		 V1.0	  芯晓		  创建
***********************************************************************/
		  			 		  						  					  				 	   		  	  	 	  
void OLED_DIsp_String(unsigned char x,unsigned char y,unsigned char *chr,unsigned char Char_Size)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{		
		OLED_DIsp_Char(x,y,chr[j],Char_Size);//显示单个字符
		x+=8;
		if(x>120){x=0;y+=2;}//移动显示位置
			j++;
	}
}
/**********************************************************************
	 * 函数名称： OLED_DIsp_CHinese
	 * 功能描述：在指定位置显示汉字
	 * 输入参数：@ x ：要显示的column address
		 			@y :要显示的page address
		 			@chr ：要显示的汉字，三个汉字“百问网”中选择一个
	 * 输出参数： 无
	 * 返 回 值： 无
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/15		 V1.0	  芯晓		  创建
 ***********************************************************************/

void OLED_DIsp_CHinese(unsigned char x,unsigned char y,unsigned char no)
{      			    
	unsigned char t,adder=0;
	OLED_DIsp_Set_Pos(x,y);	
    for(t=0;t<16;t++)
	{//显示上半截字符	
		oled_write_cmd_data(hz_1616[no][t*2],OLED_DATA);
		adder+=1;
    }	
	OLED_DIsp_Set_Pos(x,y+1);	
    for(t=0;t<16;t++)
	{//显示下半截字符
		oled_write_cmd_data(hz_1616[no][t*2+1],OLED_DATA);
		adder+=1;
    }					
}
/**********************************************************************
	 * 函数名称： OLED_DIsp_Test
	 * 功能描述： 整个屏幕显示测试
	 * 输入参数：无
	 * 输出参数： 无
	 * 返 回 值： 无
	 * 修改日期 	   版本号	 修改人		  修改内容
	 * -----------------------------------------------
	 * 2020/03/15		 V1.0	  芯晓		  创建
	 显示固定内容，大小为16X16大小
	 book.100ask.net
	 	百问网
	 book.100ask.net
	 	百问网	
 ***********************************************************************/
void OLED_DIsp_Test()
{ 	
	unsigned char uc_cnt = 0;
	unsigned char c=0,i=0;	
	unsigned char x=0;
	unsigned char y=0;
	for(uc_cnt=0;uc_cnt<15;uc_cnt++)
	{//显示book.100ask.net
		OLED_DIsp_Set_Pos(x+uc_cnt*8,y);	
		for(i=0;i<8;i++)
		{	
			oled_write_cmd_data(asc2_1616_info[c+uc_cnt][i*2],OLED_DATA);
		}
		OLED_DIsp_Set_Pos(x+uc_cnt*8,y+1);
		for(i=0;i<8;i++)
		{	oled_write_cmd_data(asc2_1616_info[c+uc_cnt][i*2+1],OLED_DATA);
			
		}
	}
	for(uc_cnt=0;uc_cnt<3;uc_cnt++)
	{//显示汉字 百问网
		OLED_DIsp_CHinese(32+uc_cnt*16,y+2,uc_cnt);
	}
	//调整显示位置
	x = 0;
	y = 4;
	for(uc_cnt=0;uc_cnt<15;uc_cnt++)
	{//显示book.100ask.net
		OLED_DIsp_Set_Pos(x+uc_cnt*8,y);	
		for(i=0;i<8;i++)
		{	
			oled_write_cmd_data(asc2_1616_info[c+uc_cnt][i*2],OLED_DATA);
		}
		OLED_DIsp_Set_Pos(x+uc_cnt*8,y+1);
		for(i=0;i<8;i++)
		{	oled_write_cmd_data(asc2_1616_info[c+uc_cnt][i*2+1],OLED_DATA);
			
		}
	}
	for(uc_cnt=0;uc_cnt<3;uc_cnt++)
	{//显示汉字 百问网
		OLED_DIsp_CHinese(32+uc_cnt*16,y+2,uc_cnt);
	}
} 

