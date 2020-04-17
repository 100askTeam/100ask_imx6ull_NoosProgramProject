#include "ap3216c.h"
#include "i2c.h"
#include "type.h"

uint8_t ap3216c_init(void)
{
	unsigned char data = 0;
	int ret = 0;
	int i = 1000000;

		/* 1、IO初始化，配置I2C IO属性 
		 * I2C1_SCL -> UART4_TXD
		 * I2C1_SDA -> UART4_RXD
		 */
		IOMUXC_SetPinMux(IOMUXC_UART4_TX_DATA_I2C1_SCL, 1);
		IOMUXC_SetPinMux(IOMUXC_UART4_RX_DATA_I2C1_SDA, 1);
	
		/* 
		 *bit 16:0 HYS关闭
		 *bit [15:14]: 1 默认47K上拉
		 *bit [13]: 1 pull功能
		 *bit [12]: 1 pull/keeper使能 
		 *bit [11]: 0 关闭开路输出
		 *bit [7:6]: 10 速度100Mhz
		 *bit [5:3]: 110 驱动能力为R0/6
		 *bit [0]: 1 高转换率
		 */
		IOMUXC_SetPinConfig(IOMUXC_UART4_TX_DATA_I2C1_SCL, 0x70B0);
		IOMUXC_SetPinConfig(IOMUXC_UART4_RX_DATA_I2C1_SDA, 0X70B0);
	
		i2c_init(I2C1); 	/* 初始化I2C1 */
	
		/* 2、初始化AP3216C */
		/* 复位AP3216C		*/
		ret = i2c_write_one_byte(AP3216C_ADDR, AP3216C_SYSTEMCONG, 0X4);
		while(i--){};												        /* AP33216C复位至少10ms */
		ret = i2c_write_one_byte(AP3216C_ADDR, AP3216C_SYSTEMCONG, 0X3);	/* 开启ALS、PS+IR*/

		data = i2c_read_one_byte(AP3216C_ADDR, AP3216C_SYSTEMCONG);	        /* 读取刚刚写进去的0X03 */

		if(data == 0x03)
		{
			return 0;	/* AP3216C正常	*/
		}
		else 
		{
			return 1;	/* AP3216C失败	*/
		}
		  			 		  						  					  				 	   		  	  	 	  

}

void ap3216c_read_data(uint16_t *ir, uint16_t *ps, uint16_t *als)
{
    uint8_t buf[6];
    uint8_t i;

    for(i = 0; i < 6; i++)	/* 循环读取所有传感器数据 */
    {
        buf[i] = i2c_read_one_byte(AP3216C_ADDR, AP3216C_IRDATALOW + i);	
    }
	
    if(buf[0] & 0X80)      /* IR_OF位为1,则数据无效 */
	{
		*ir = 0;			
	}
		
	else 				   /* 读取IR传感器的数据   */
	{
		*ir = ((uint16_t)buf[1] << 2) | (buf[0] & 0X03); 	
	}
						   /* 读取ALS传感器的数据 */  
	*als = ((uint16_t)buf[3] << 8) | buf[2];	
	
    if(buf[4] & 0x40)	  /* IR_OF位为1,则数据无效*/
	{
		*ps = 0;    	
	}										
	else 			       /* 读取PS传感器的数据    */	
	{
		*ps = ((uint16_t)(buf[5] & 0X3F) << 4) | (buf[4] & 0X0F); 
	}
	return;
}

