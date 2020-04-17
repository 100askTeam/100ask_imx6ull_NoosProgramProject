
#include "i2c.h"
void i2c_init(I2C_REGISTERS *I2C_BASE)
{
    /*I2C_I2CR是用来使能I2C和I2C中断的，而且还有bit位来选择主从模式.*/
    /*我们在初始化时主要是为了设置I2C的传输波特率*/

    /*访问I2C寄存器前，关闭I2C,操作完后打开*/
		  			 		  						  					  				 	   		  	  	 	  
    /* 设置波特率为100K
     * I2C的时钟源来源于IPG_CLK_ROOT=66Mhz
	 *	PLL2 = 528 MHz
	 *	PLL2_PFD2 = 528 MHz
	 *	IPG_CLK_ROOT = (PLL2_PFD2 / ahb_podf )/ ipg_podf = (528 MHz/4)/2 = 66Mhz
	 *	MHz
	 *	PER_CLK_ROOT = IPG_CLK_ROOT/perclk_podf = 66 MHz/1 = 66 MHz
	 * 设置I2C的波特率为100K， 因此当分频值=66000000/100000=660.	
	 * 参考Table 31-3. I2C_IFDR Register Field Values 中只有640对应的0x15最接近	
	 * 即寄存器IFDR的IC位设置为0X15
	 */	 
	I2C_BASE->I2CR &= ~(1 << 7);
	I2C_BASE->IFDR = 0x15;
	I2C_BASE->I2CR |= (1<<7);
}

uint8_t i2c_check(I2C_REGISTERS *I2C_BASE, uint32_t status)
{
	/* 检查是否发生仲裁丢失错误 */
	if(status & (1<<4))
	{
		I2C_BASE->I2SR &= ~(1<<4);		/* 清除仲裁丢失错误位 			*/

		I2C_BASE->I2CR &= ~(1 << 7);	/* 先关闭I2C 				*/
		I2C_BASE->I2CR |= (1 << 7);		/* 重新打开I2C 				*/
		return I2C_ARBITRATIONLOST;
	} 
	else if(status & (1 << 0))     	/* 没有接收到从机的应答信号 */
	{
		return I2C_NAK;		/* 返回NAK(No acknowledge) */
	}
	return I2C_OK;

}

uint8_t i2c_start(I2C_REGISTERS *I2C_BASE, uint8_t ucSlaveAddr, uint32_t ulOpcode)
{

	if(I2C_BASE->I2SR & (1 << 5))			/* I2C忙 */
		return 1;

	/*
     * 设置寄存器I2CR
     * bit[5]: 1 主模式
     * bit[4]: 1 发送
	 */
	I2C_BASE->I2CR |=  (1 << 5) | (1 << 4);

	/*
     * 设置寄存器I2DR
     * bit[7:0] : 要发送的数据，这里写入从设备地址
     *            参考资料:IMX6UL参考手册P1249
	 */ 
	I2C_BASE->I2DR = ((uint32_t)ucSlaveAddr << 1) | ((I2C_READ == ulOpcode)? 1 : 0);
	return 0;

}
uint8_t i2c_stop(I2C_REGISTERS *I2C_BASE)
{

	uint16_t usTimeout = 0xffff;

	/*
	 * 清除I2CR的bit[5:3]这三位
	 */
	I2C_BASE->I2CR &= ~((1 << 5) | (1 << 4) | (1 << 3));

	/* 等待忙结束 */
	while((I2C_BASE->I2SR & (1 << 5)))
	{
		usTimeout--;
		if(usTimeout == 0)	/* 超时跳出 */
			return I2C_TIMEOUT;
	}
	return I2C_OK;

}

uint8_t i2c_restart(I2C_REGISTERS *I2C_BASE, uint8_t ucSlaveAddr, uint32_t ulOpcode)
{

	/* I2C忙并且工作在从模式,跳出 */
	if(I2C_BASE->I2SR & (1 << 5) && (((I2C_BASE->I2CR) & (1 << 5)) == 0))		
		return 6;

	/*
     * 设置寄存器I2CR
     * bit[4]: 1 发送
     * bit[2]: 1 产生重新开始信号
	 */
	I2C_BASE->I2CR |=  (1 << 4) | (1 << 2);

	/*
     * 设置寄存器I2DR
     * bit[7:0] : 要发送的数据，这里写入从设备地址
     *            参考资料:IMX6UL参考手册P1249
	 */ 
	I2C_BASE->I2DR = ((uint32_t)ucSlaveAddr << 1) | ((I2C_READ == ulOpcode)? 1 : 0);
	
	return 0;

}


void i2c_write(I2C_REGISTERS *I2C_BASE, const uint8_t *pbuf, uint32_t len)
{
	/* 等待传输完成 */
	while(!(I2C_BASE->I2SR & (1 << 7))); 
	
	I2C_BASE->I2SR &= ~(1 << 1); 	                  /* 清除标志位 */
	I2C_BASE->I2CR |= 1 << 4;		                  /* 发送数据 */
	while(len--)
	{
		I2C_BASE->I2DR = *pbuf++; 	                  /* 将buf中的数据写入到I2DR寄存器 */
		
		while(!(I2C_BASE->I2SR & (1 << 1))); 	      /* 等待传输完成 */	
		I2C_BASE->I2SR &= ~(1 << 1);			      /* 清除标志位 */

		/* 检查ACK */
		if(i2c_check(I2C_BASE, I2C_BASE->I2SR))
			break;
	}
	
	I2C_BASE->I2SR &= ~(1 << 1);
	i2c_stop(I2C_BASE); 	                          /* 发送停止信号 */

}

void i2c_read(I2C_REGISTERS *I2C_BASE, uint8_t *pbuf, uint32_t len)
{
	volatile uint8_t dummy = 0;
	dummy++; 	/* 防止编译报错 */
	/* 等待传输完成 */
	while(!(I2C_BASE->I2SR & (1 << 7))); 
	
	I2C_BASE->I2SR &= ~(1 << 1); 				/* 清除中断挂起位 */
	I2C_BASE->I2CR &= ~((1 << 4) | (1 << 3));	/* 接收数据 */
	
	/* 如果只接收一个字节数据的话发送NACK信号 */
	if(len == 1)
        I2C_BASE->I2CR |= (1 << 3);

	dummy = I2C_BASE->I2DR; /* 假读 */


	while(len--)
	{
		while(!(I2C_BASE->I2SR & (1 << 1))); 	/* 等待传输完成 */	
		I2C_BASE->I2SR &= ~(1 << 1);			/* 清除标志位 */

	 	if(len == 0)
        {
        	i2c_stop(I2C_BASE); 			/* 发送停止信号 */
        }

        if(len == 1)
        {
            I2C_BASE->I2CR |= (1 << 3);
        }
		*pbuf++ = I2C_BASE->I2DR;
	}

}

uint8_t i2c_transfer(I2C_REGISTERS *I2C_BASE, I2C_TRANSFER *transfer)
{
	uint32_t ulRet = 0;
	uint32_t ulOpcode = transfer->ulOpcode;

	/*开始前准备工作，清除标志位
	 *bit-4 IAL 仲裁位，bit-1 IIF 中断标志位
	 */
	I2C_BASE->I2SR &= ~((1 << 1) | (1 << 4));
	/* 等待传输完成 */
	while(!((I2C_BASE->I2SR >> 7) & 0X1)){}; 

	/* 如果是读的话，要先发送寄存器地址，所以要先将方向改为写 */
    if ((transfer->ulSubAddressLen > 0) && (transfer->ulOpcode == I2C_READ))
    {
        ulOpcode = I2C_WRITE;
    }
	ulRet = i2c_start(I2C_BASE, transfer->ucSlaveAddress, ulOpcode);

	if (ulRet)
	{
		return ulRet;
	}
	/* 等待传输完成 */

	while(!(I2C_BASE->I2SR & (1 << 1))){};

	/* 不放心的话，就检查一下*/
	ulRet = i2c_check(I2C_BASE, I2C_BASE->I2SR);

	if (ulRet)
	{
	    i2c_stop(I2C_BASE); 			/* 发送停止信号 */
		return ulRet;
	}

	/*如果寄存器地址不为0，证明需要发送地址*/

	if (transfer->ulSubAddressLen)
	{
		do
		{
			/* 清除标志位 */
		    I2C_BASE->I2SR &= ~(1 << 1); 
			/*一次一个字节*/
			transfer->ulSubAddressLen--;

			I2C_BASE->I2DR = ((transfer->ulSubAddress) >> (8 * transfer->ulSubAddressLen)); 
  
			while(!(I2C_BASE->I2SR & (1 << 1)));  	/* 等待传输完成 */

            /* 检查是否有错误发生 */
            ulRet = i2c_check(I2C_BASE, I2C_BASE->I2SR);
            if(ulRet)
            {
             	i2c_stop(I2C_BASE); 				/* 发送停止信号 */
             	return ulRet;
            }
		}
		while ((transfer->ulSubAddressLen > 0) && (ulRet == I2C_OK));

		if (I2C_READ == transfer->ulOpcode)
		{
            I2C_BASE->I2SR &= ~(1 << 1);			/* 清除中断挂起位 */
            i2c_restart(I2C_BASE, transfer->ucSlaveAddress, I2C_READ); /* 发送重复开始信号和从机地址 */
    		while(!(I2C_BASE->I2SR & (1 << 1))){};/* 等待传输完成 */

            /* 检查是否有错误发生 */
			ulRet = i2c_check(I2C_BASE, I2C_BASE->I2SR);
			
            if(ulRet)
            {
             	ulRet = I2C_ADDRNAK;
                i2c_stop(I2C_BASE); 		/* 发送停止信号 */
                return ulRet;  
            }
           	       

		}
		
	}
    /* 发送数据 */
    if ((I2C_WRITE == transfer->ulOpcode) && (transfer->ulLenth > 0))
    {
    	i2c_write(I2C_BASE, transfer->pbuf, transfer->ulLenth);
	}

    /* 读取数据 */
    if ((I2C_READ == transfer->ulOpcode) && (transfer->ulLenth > 0))
    {
       	i2c_read(I2C_BASE, transfer->pbuf, transfer->ulLenth);
	}
	return 0;	

}
uint8_t i2c_write_one_byte(uint8_t addr,uint8_t reg, uint8_t data)
{
    uint8_t status = 0;
    uint8_t writedata=data;
    I2C_TRANSFER transfer;
	
    /* 配置I2C xfer结构体 */
   	transfer.ucSlaveAddress = addr; 			/* 设备地址 				*/
    transfer.ulOpcode = I2C_WRITE;			    /* 写入数据 				*/
    transfer.ulSubAddress = reg;				/* 要写入的寄存器地址 			*/
    transfer.ulSubAddressLen = 1;				/* 地址长度一个字节 			*/
    transfer.pbuf = &writedata;				    /* 要写入的数据 				*/
    transfer.ulLenth = 1;  					    /* 写入数据长度1个字节			*/

    status = i2c_transfer(I2C1, &transfer);
    return status;
}

uint8_t i2c_read_one_byte(uint8_t addr, uint8_t reg)
{
	uint8_t val=0;
    uint8_t status = 0;	
    I2C_TRANSFER transfer;

	transfer.ucSlaveAddress = addr;				/* 设备地址 				*/
    transfer.ulOpcode = I2C_READ;			    /* 读取数据 				*/
    transfer.ulSubAddress = reg;				/* 要读取的寄存器地址 			*/
    transfer.ulSubAddressLen = 1;				/* 地址长度一个字节 			*/
    transfer.pbuf = &val;						/* 接收数据缓冲区 				*/
    transfer.ulLenth = 1;					    /* 读取数据长度1个字节			*/

    status = i2c_transfer(I2C1, &transfer);
	return val;
}


