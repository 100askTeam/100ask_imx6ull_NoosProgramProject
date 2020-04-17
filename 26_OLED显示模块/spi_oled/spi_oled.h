#ifndef _OLED_H_
#define _OLED_H_

void oled_write_cmd_data(unsigned char uc_data,unsigned char uc_cmd);
unsigned char oled_init(void);
unsigned char oled_fill_data(unsigned char fill_Data);
void OLED_DIsp_Clear(void);
void OLED_DIsp_All(void);
void OLED_DIsp_Set_Pos(unsigned char x, unsigned char y);
void OLED_DIsp_Char(unsigned char x,unsigned char y,unsigned char chr,unsigned char Char_Size);
void OLED_DIsp_String(unsigned char x,unsigned char y,unsigned char *chr,unsigned char Char_Size);
void OLED_DIsp_CHinese(unsigned char x,unsigned char y,unsigned char no);
void OLED_DIsp_Test();

		  			 		  						  					  				 	   		  	  	 	  
#endif

