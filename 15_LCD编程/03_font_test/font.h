
#ifndef _FONT_H
#define _FONT_H
/**********************************************************************
 * 函数名称： font_init
 * 功能描述： 获取LCD参数
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2020/02/26	     V1.0	  zh(angenao)	      创建
 ***********************************************************************/                
		  			 		  						  					  				 	   		  	  	 	  
void font_init(void);
/**********************************************************************
 * 函数名称： fb_print_char
 * 功能描述： 在lcd上打印单个字符
 * 输入参数： x坐标，y坐标，字符，颜色
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2020/02/26	     V1.0	  zh(angenao)	      创建
 ***********************************************************************/                

void fb_print_char(int x, int y, char c, unsigned int color);
/**********************************************************************
 * 函数名称： fb_print_string
 * 功能描述： 在lcd上打印字符串
 * 输入参数： x坐标，y坐标，字符串，颜色
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2020/02/26	     V1.0	  zh(angenao)	      创建
 ***********************************************************************/                

void fb_print_string(int x, int y, char* str, unsigned int color);

#endif /* _FONT_H */

