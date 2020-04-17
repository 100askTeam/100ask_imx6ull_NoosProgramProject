
#include "led.h"
#include "common.h"
#include "uart.h"
#include "my_printf.h"
#include "gic.h"
#include "timer.h"
#include "24cxx.h"

void init_pins(void)
{
	/* led pins */
    IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER3_GPIO5_IO03, 0U);
    IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER3_GPIO5_IO03, 
                        IOMUXC_SW_PAD_CTL_PAD_SRE_MASK |
                        IOMUXC_SW_PAD_CTL_PAD_DSE(1U) |
                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);

	/* uart1 pins */	
	IOMUXC_SetPinMux(IOMUXC_UART1_RX_DATA_UART1_RX, 0U);
	IOMUXC_SetPinConfig(IOMUXC_UART1_RX_DATA_UART1_RX, 
						IOMUXC_SW_PAD_CTL_PAD_DSE(2U) |
						IOMUXC_SW_PAD_CTL_PAD_SPEED(2U) |
						IOMUXC_SW_PAD_CTL_PAD_PKE_MASK);
	IOMUXC_SetPinMux(IOMUXC_UART1_TX_DATA_UART1_TX, 0U);
	IOMUXC_SetPinConfig(IOMUXC_UART1_TX_DATA_UART1_TX, 
						IOMUXC_SW_PAD_CTL_PAD_DSE(2U) |
						IOMUXC_SW_PAD_CTL_PAD_SPEED(2U) |
						IOMUXC_SW_PAD_CTL_PAD_PKE_MASK);
	/* uart6 pins */
    IOMUXC_SetPinMux(IOMUXC_CSI_MCLK_UART6_TX, 0x08);
    IOMUXC_SetPinMux(IOMUXC_CSI_PIXCLK_UART6_RX, 0x08);
    IOMUXC_SetPinConfig(IOMUXC_CSI_MCLK_UART6_TX, 0x10b0);
    IOMUXC_SetPinConfig(IOMUXC_CSI_PIXCLK_UART6_RX, 0x10b0);

	/* KEY1 pins GPIO5_1 SNVS_TAMPER1 */
	IOMUXC_SetPinMux(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 0U);
	IOMUXC_SetPinConfig(IOMUXC_SNVS_SNVS_TAMPER1_GPIO5_IO01, 
						IOMUXC_SW_PAD_CTL_PAD_DSE(6U) |
						IOMUXC_SW_PAD_CTL_PAD_SPEED(2U) |
						IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);

	/* KEY2 pins GPIO4_IO14 */
	IOMUXC_SetPinMux(IOMUXC_NAND_CE1_B_GPIO4_IO14, 0U);
	IOMUXC_SetPinConfig(IOMUXC_NAND_CE1_B_GPIO4_IO14, 
						IOMUXC_SW_PAD_CTL_PAD_DSE(6U) |
						IOMUXC_SW_PAD_CTL_PAD_SPEED(2U) |
						IOMUXC_SW_PAD_CTL_PAD_PKE_MASK |
						IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);

}

void delay(volatile unsigned int d)
{
	while(d--);
}

void boot_clk_gate_init(void)
{
    CCM->CCGR0 = 0xFFFFFFFFU;
    CCM->CCGR1 = 0xFFFFFFFFU;
    CCM->CCGR2 = 0xFFFFFFFFU;
    CCM->CCGR3 = 0xFFFFFFFFU;
    CCM->CCGR4 = 0xFFFFFFFFU;
    CCM->CCGR5 = 0xFFFFFFFFU;
    CCM->CCGR6 = 0xFFFFFFFFU;
}

void boot_clk_init(void)
{
	if ((CCM->CCSR & (1 << 2)) == 0 ) /* CPU runs on ARM PLL : bit2 pll1_sw_clk_sel*/
	{
		CCM->CCSR &= ~(1 << 8);/* Set Step MUX to OSC : bit8 STEP_SEL */
		CCM->CCSR |= (1 << 2);/* Let CPU run on Step MUX : bit2 pll1_sw_clk_sel*/
	}

	CCM_ANALOG->PLL_ARM = (1 << 13) | (88 << 0); /*Configure ARM PLL to 1056M:bit13 Enable, bit0~6 DIV_SELECT*/
	CCM->CCSR &= ~(1 << 2); /* Now CPU runs again on ARM PLL*/
	CCM->CACRR = 1; /* Configure ARM clock root with divide 2 */

#if 0
	/* Set periph2_clk2 and periph_clk2 MUX to OSC */
	CCM->CBCMR &= ~((3 << 20) | ( 3 << 12));
	CCM->CBCMR |= (1 << 20) | ( 1 << 12);
	/* Let BUS and mmdc clock run on OSC */
	CCM->CBCDR &= ~((1 << 6) | ( 1 << 25) | ( 1 << 26));
	CCM->CBCDR |= (0 << 6) | ( 1 << 25) | ( 1 << 26);
	/* Wait handshake process */
	while (CCM->CDHIPR & ((1 << 5) | (1 << 3)))
	{
	}

	/* Configure SYS PLL to 528M */
	CCM_ANALOG->PLL_SYS_SS &= (1 << 15);
	CCM_ANALOG->PLL_SYS_NUM = 0; 
	CCM_ANALOG->PLL_SYS = (1 << 13) | (1 << 0);
	/* wait for PLL is locked */
	while ((CCM_ANALOG->PLL_SYS & (1 << 31)) == 0)
	{
	}
#endif

	 /* Configure PFD_528
	 * PFD0     : Enable, set divider 27 352M
	 * PFD1     : Enable, set divider 16 594M
	 * PFD2     : Enable, set divider 24 396M
	 * PFD3     : Enable, set divider 48 198M
	 */
	 /* Disable all clock output first. */
	//CCM_ANALOG->PFD_528 |= (1 << 31) | ( 1 << 23) | (1 << 15) | ( 1 << 7);
	/* Clear first, then Set default divide value for all PFD. */
	CCM_ANALOG->PFD_528 &= ~((0x3F << 24) | (0x3F << 16) | (0x3F << 8) | (0x3F << 0));
	CCM_ANALOG->PFD_528 |= (0x30 << 24) | (0x18 << 16) | (0x10 << 8) | (0x1B << 0);
	/* Enable PFD_528 PFD0~3*/
	//CCM_ANALOG->PFD_528 &= ~((1 << 31) | ( 1 << 23) | (1 << 15) | ( 1 << 7));
	

	/* Configure PFD_480
	 * PFD0     : Enable, set divider 12 720M
	 * PFD1     : Enable, set divider 16 540M
	 * PFD2     : Enable, set divider 17 508.2M
	 * PFD3     : Enable, set divider 19 454.7M
	 */
	/* Disable all clock output. */
	//CCM_ANALOG->PFD_480 |= (1 << 31) | ( 1 << 23) | (1 << 15) | ( 1 << 7);
	/* Clear first, then Set default divide value for all PFD. */
	CCM_ANALOG->PFD_480 &= ~((0x3F << 24) | (0x3F << 16) | (0x3F << 8) | (0x3F << 0));
	CCM_ANALOG->PFD_480 |= (0x13 << 24) | (0x11 << 16) | (0x10 << 8) | (0xC << 0);
	/* Enable PFD_528 PFD0~3*/
	//CCM_ANALOG->PFD_480 &= ~((1 << 31) | ( 1 << 23) | (1 << 15) | ( 1 << 7));

    /* Configure BUS clcok
     * AHB     : Sourced from SYS PLL PFD2 396M with divider 3 132M
     * IPG     : Sourced from AHB with divider 2 66M
     * AXI     : Sourced from SYS PLL PFD2 396M with divider 2 198M
     * MMDC    : Sourced from SYS PLL PFD2 396M with divider 1 396M
     */
     /*
      *AHB_CLK_ROOT<= PLL2 PFD2(pre_periph_clk=1 periph_clk=0 ahb_podf=3)
      *IPG_CLK_ROOT<=AHB_CLK_ROOT(ipg_pdof=2)
      *AXI_CLK_ROOT<= PLL2 PFD2(AXI_pdof=2)
      */

    /* Set pre_periph2_clk and pre_periph_clk MUX to SYS PLL PFD2*/
    CCM->CBCMR &= ~((3 << 18) | ( 3 << 21));
	CCM->CBCMR |= (1 << 18) | ( 1 << 21);

    /* Set periph2_clk and periph_clk MUX to PLL2*/
    CCM->CBCDR &= ~((1 << 26) | ( 1 << 25));
	CCM->CBCDR |= (0 << 26) | ( 0 << 25);//no use, just for tidy

	/* Config AXI divide by 2, AHB divide by 3, IPG divide by 2 */
	//CCM->CBCDR &= ~((7 << 16) | ( 7 << 10) | ( 3 << 8));
	//CCM->CBCDR |= (1 << 16) | ( 2 << 10) | ( 1 << 8);
	/* Config AXI divide by 2, IPG divide by 2 */
	CCM->CBCDR &= ~((7 << 16) | ( 3 << 8));
	CCM->CBCDR |= (1 << 16) | ( 1 << 8);

	/* Config other filed in CBCDR to certain value */
	/* AXI root clock from PLL2 PFD2, PERIPH_CLK2_PODF=0,PERIPH2_CLK2_PODF=0*/
	CCM->CBCDR &= ~((1 << 7) | ( 7 << 27) | ( 7 << 0));
	CCM->CBCDR |= (0 << 7) | ( 0 << 27) | ( 0 << 0);

	/* Wait handshake process */
	while (CCM->CDHIPR & ((1 << 5) | (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0)))
	{
	}

	/* Select PLL3 to generate pll3_sw_clk from pll3_main_clk */
    CCM->CCSR &= ~(1 << 0);

    CCM->CSCDR1 &= ~(1 << 6); /* Set UART source to PLL3 80M */
    CCM->CSCDR1 &= ~(0x3F << 0); /* Set UART divider to 1 */

	/* Set PERCLK_CLK_ROOT (I2C/EPIT source) to IPG 66M */
	CCM->CSCMR1 &= ~(1 << 6); /* Set I2C/EPIT source to IPG 66M */
    CCM->CSCMR1 &= ~(0x3F << 0); /* Set I2C/EPIT divider to 1 */

}

void printException(unsigned int cpsr, char *str)
{
	printf("Exception! cpsr is 0x%x\r\n", cpsr);
	printf("%s\r\n", str);
}

void printSWIVal(unsigned int *pSWI)
{
	printf("SWI val = 0x%x\r\n", *pSWI & ~0xff000000);
}

void key_irq_init(void)
{
	/* if set detects any edge on the corresponding input signal*/
	GPIO5->EDGE_SEL |= (1 << 1);
	/* if set 1, unmasked, Interrupt n is enabled */
	GPIO5->IMR |= (1 << 1); 

	GPIO4->EDGE_SEL |= (1 << 14);
	GPIO4->IMR |= (1 << 14); 
}
const char *fixmode_tbl[4]={"Fail", "Fail", " 2D ", " 3D "};


void system_init()
{
	init_pins();
	gic_init();
	led_gpio_init();
	led_ctl(0);//turn off led
	boot_clk_gate_init();
	boot_clk_init();
	puts("hello world\r\n");
	key_irq_init();

	gic_enable_irq(GPIO5_Combined_0_15_IRQn);
	gic_enable_irq(GPIO4_Combined_0_15_IRQn);
	gic_enable_irq(EPIT1_IRQn);
	gic_enable_irq(UART1_IRQn);
	gic_enable_irq(UART6_IRQn);
	uart_init(UART1, 115200, 80000000);
	
	// 初始化AT24CXX
	AT24CXX_Init();
}

int main()
{
	unsigned short temp = 0;
	//要写入到24c02的字符串数组
	const unsigned char TEXT_Arr[] = {"IMX6ULL 24CXX TEST!"};
	const unsigned char TEXT_None[] =  {"                   "};
	#define TEXT_SIZE sizeof(TEXT_Arr)
	unsigned char print_data[TEXT_SIZE];

	AT24CXX_Write(10, (unsigned char *)TEXT_None, TEXT_SIZE);
	AT24CXX_Read(10, print_data, TEXT_SIZE);
	printf("The Data Readed Is: %s \r\n\r\n", print_data);
	read_state = 0;
	write_state = 0;

	while(1) {

		// 24cxx
		if(read_state)
		{
            read_state = 0;
            printf("Start Read 24CXX....    \r\n\r\n");
			AT24CXX_Read(0, print_data, TEXT_SIZE);
            printf("The Data Readed Is: %s \r\n\r\n", print_data);
		} else if(write_state) {
            write_state = 0;
            printf("Start Write 24CXX....   \r\n\r\n");
			AT24CXX_Write(0, (unsigned char *)TEXT_Arr, TEXT_SIZE);
            printf("24CXX Write Finished!   \r\n\r\n");
		}
		
		if(temp){
			GPIO5->DR &= ~(1<<3);
		} else {
			GPIO5->DR |= (1<<3);
		}
		temp = !temp;
		gpt2_chan1_delay_us(1000000);
	}
	return 0;
}


