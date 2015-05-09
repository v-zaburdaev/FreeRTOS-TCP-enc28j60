#include "enc28j60.h"

#include "stm32f4xx_hal.h"
#include "debug.hpp"


static void enc28j60_select()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

static void enc28j60_release()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

volatile uint8_t enc28j60_current_bank = 0;
volatile uint16_t enc28j60_rxrdpt = 0;

SPI_HandleTypeDef SpiHandle;

void enc28j60_init(uint8_t *macadr)
{
	uint32_t i;
    /*
	// Initialize SPI
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    */
    
    // Initialize clock on SPI and for MISO/MOSI
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;

    /*
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB, &GPIO_InitStruct);
    */

    // Initialize SCLK
	GPIO_InitStruct.Pin       = GPIO_PIN_13;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;

    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Initialize MOSI
	GPIO_InitStruct.Pin       = GPIO_PIN_15;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /*
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &GPIO_InitStruct);

    */

    // Initialize MISO

	GPIO_InitStruct.Pin       = GPIO_PIN_14;
    GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;    // ICE: official examples show another way of configuration
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    //GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    //GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*
    // SS
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB, &GPIO_InitStruct);
    */

	// Initialize SS (Slave Select, a.k.a. CS)
	GPIO_InitStruct.Pin       = GPIO_PIN_12;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    //GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    //GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
/*
	// RESET
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOA, &GPIO_InitStruct);
    
*/

    // Set RESET pin
	GPIO_InitStruct.Pin       = GPIO_PIN_7;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    //GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = 0;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/*
   

  
	SPI_InitTypeDef SPI_InitStruct;

	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStruct);



    SPI_Cmd(SPI2, ENABLE);

*/

    SpiHandle.Instance               = SPI2;

    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.Mode              = SPI_MODE_MASTER;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;


    if(HAL_SPI_Init(&SpiHandle) != HAL_OK)
    {
        /* Initialization Error */
        debug("PANIC: SPI initialization error at %s, line %d .\n", __FILE__, __LINE__);
        while(1)
            ;
    }

    /*
       enc28j60_release();
       for (i=0; i<720000; i++) {
       GPIO_ResetBits(GPIOA, GPIO_Pin_7);
       }
       GPIO_SetBits(GPIOA, GPIO_Pin_7);
       */

       enc28j60_release();
       for (i = 0; i < 720000; i++) {
           HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
       }
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

       

    // Reset ENC28J60
    enc28j60_soft_reset();

    
    // Setup Rx/Tx buffer
    enc28j60_wcr16(ERXST, ENC28J60_RXSTART);
    enc28j60_rcr16(ERXST);
    enc28j60_wcr16(ERXRDPT, ENC28J60_RXSTART);
    enc28j60_wcr16(ERXND, ENC28J60_RXEND);
    enc28j60_rxrdpt = ENC28J60_RXSTART;

    // Setup MAC
    enc28j60_wcr(MACON1, MACON1_TXPAUS| // Enable flow control
    MACON1_RXPAUS|MACON1_MARXEN); // Enable MAC Rx
    enc28j60_wcr(MACON2, 0); // Clear reset
    enc28j60_wcr(MACON3, MACON3_PADCFG0| // Enable padding,
    MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX); // Enable crc & frame len chk
    enc28j60_wcr16(MAMXFL, ENC28J60_MAXFRAME);
    enc28j60_wcr(MABBIPG, 0x15); // Set inter-frame gap
    enc28j60_wcr(MAIPGL, 0x12);
    enc28j60_wcr(MAIPGH, 0x0c);
    enc28j60_wcr(MAADR5, macadr[0]); // Set MAC address
    enc28j60_wcr(MAADR4, macadr[1]);
    enc28j60_wcr(MAADR3, macadr[2]);
    enc28j60_wcr(MAADR2, macadr[3]);
    enc28j60_wcr(MAADR1, macadr[4]);
    enc28j60_wcr(MAADR0, macadr[5]);

    
    // Setup PHY
    enc28j60_write_phy(PHCON1, PHCON1_PDPXMD); // Force full-duplex mode
    enc28j60_write_phy(PHCON2, PHCON2_HDLDIS); // Disable loopback
    enc28j60_write_phy(PHLCON, PHLCON_LACFG2| // Configure LED ctrl
    PHLCON_LBCFG2|PHLCON_LBCFG1|PHLCON_LBCFG0|
    PHLCON_LFRQ0|PHLCON_STRCH);

    // Enable Rx packets
    enc28j60_bfs(ECON1, ECON1_RXEN);
    
}

void enc28j60_soft_reset()
{
	enc28j60_select();
	enc28j60_txrx_byte(ENC28J60_SPI_SC);
	enc28j60_release();
	
	enc28j60_current_bank = 0;
    HAL_Delay(5);
	//_delay_ms(1); // Wait until device initializes
}

void enc28j60_send_packet(uint8_t *data, uint16_t len)
{
	while(enc28j60_rcr(ECON1) & ECON1_TXRTS)
	{
		// TXRTS may not clear - ENC28J60 bug. We must reset
		// transmit logic in cause of Tx error
		if(enc28j60_rcr(EIR) & EIR_TXERIF)
		{
			enc28j60_bfs(ECON1, ECON1_TXRST);
			enc28j60_bfc(ECON1, ECON1_TXRST);
		}
	}

	enc28j60_wcr16(EWRPT, ENC28J60_TXSTART);
	enc28j60_write_buffer((uint8_t*)"\x00", 1);
	enc28j60_write_buffer(data, len);

	enc28j60_wcr16(ETXST, ENC28J60_TXSTART);
	enc28j60_wcr16(ETXND, ENC28J60_TXSTART + len);

	enc28j60_bfs(ECON1, ECON1_TXRTS); // Request packet send
}

uint16_t enc28j60_recv_packet(uint8_t *buf, uint16_t buflen)
{
	uint16_t len = 0, rxlen, status, temp;

	if(enc28j60_rcr(EPKTCNT))
	{
		enc28j60_wcr16(ERDPT, enc28j60_rxrdpt);

		enc28j60_read_buffer((void *)(&enc28j60_rxrdpt), sizeof(enc28j60_rxrdpt));
		enc28j60_read_buffer((void *)(&rxlen), sizeof(rxlen));
		enc28j60_read_buffer((void *)(&status), sizeof(status));

		if(status & 0x80) //success
		{
			len = rxlen - 4; //throw out crc
			if(len > buflen) len = buflen;
			enc28j60_read_buffer(buf, len);	
		}

		// Set Rx read pointer to next packet
		temp = (enc28j60_rxrdpt - 1) & ENC28J60_BUFEND;
		enc28j60_wcr16(ERXRDPT, temp);

		// Decrement packet counter
		enc28j60_bfs(ECON2, ECON2_PKTDEC);
	}

	return len;
}

// Set register bank
void enc28j60_set_bank(uint8_t adr)
{
	uint8_t bank;

	if( (adr & ENC28J60_ADDR_MASK) < ENC28J60_COMMON_CR )
	{
		bank = (adr >> 5) & 0x03; //BSEL1|BSEL0=0x03
		if(bank != enc28j60_current_bank)
		{
			enc28j60_write_op(ENC28J60_SPI_BFC, ECON1, 0x03);
			enc28j60_write_op(ENC28J60_SPI_BFS, ECON1, bank);
			enc28j60_current_bank = bank;
		}
	}
}

// Read register
uint8_t enc28j60_rcr(uint8_t adr)
{
	enc28j60_set_bank(adr);
	return enc28j60_read_op(ENC28J60_SPI_RCR, adr);
}

// Read register pair
uint16_t enc28j60_rcr16(uint8_t adr)
{
	enc28j60_set_bank(adr);
	return enc28j60_read_op(ENC28J60_SPI_RCR, adr) |
		(enc28j60_read_op(ENC28J60_SPI_RCR, adr+1) << 8);
}

// Write register
void enc28j60_wcr(uint8_t adr, uint8_t arg)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
}

// Write register pair
void enc28j60_wcr16(uint8_t adr, uint16_t arg)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr, arg);
	enc28j60_write_op(ENC28J60_SPI_WCR, adr+1, arg>>8);
}

// Clear bits in register (reg &= ~mask)
void enc28j60_bfc(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_BFC, adr, mask);
}

// Set bits in register (reg |= mask)
void enc28j60_bfs(uint8_t adr, uint8_t mask)
{
	enc28j60_set_bank(adr);
	enc28j60_write_op(ENC28J60_SPI_BFS, adr, mask);
}

uint8_t enc28j60_txrx_byte(uint8_t data)
{
    /*
	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE)==RESET);
	SPI_I2S_SendData(SPI2,data);

	while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE)==RESET);
	return SPI_I2S_ReceiveData(SPI2);
    */
    uint8_t recv_data;

    switch(HAL_SPI_TransmitReceive(&SpiHandle, &data, &recv_data, 1, 100)) {
        case HAL_OK:  
            break;  
        case HAL_TIMEOUT:
            debug("PANIC: SPI transmit/receive timeout occured at %s, line %d .\n", __FILE__, __LINE__);
            break;  
        case HAL_ERROR:
            debug("PANIC: SPI transmit/receive error at %s, line %d.\n", __FILE__, __LINE__);
            break;
        default:
            break;
    }
    return recv_data;
}

// Read Rx/Tx buffer (at ERDPT)
void enc28j60_read_buffer(volatile uint8_t *buf, uint16_t len)
{
	enc28j60_select();
	enc28j60_txrx_byte(ENC28J60_SPI_RBM);
	while(len--)
		*(buf++) = enc28j60_txrx_byte(0xFF);
	enc28j60_release();
}

// Write Rx/Tx buffer (at EWRPT)
void enc28j60_write_buffer(uint8_t *buf, uint16_t len)
{
	enc28j60_select();
	enc28j60_txrx_byte(ENC28J60_SPI_WBM);
	while(len--)
		enc28j60_txrx_byte(*(buf++));
	enc28j60_release();
}

// Generic SPI read command
uint8_t enc28j60_read_op(uint8_t cmd, uint8_t adr)
{
	uint8_t data;

	enc28j60_select();
	enc28j60_txrx_byte(cmd | (adr & ENC28J60_ADDR_MASK));
	if(adr & 0x80) // throw out dummy byte 
		enc28j60_txrx_byte(0xFF); // when reading MII/MAC register
	data = enc28j60_txrx_byte(0xFF);
	enc28j60_release();
	return data;
}

// Generic SPI write command
void enc28j60_write_op(uint8_t cmd, uint8_t adr, uint8_t data)
{
	enc28j60_select();
	enc28j60_txrx_byte(cmd | (adr & ENC28J60_ADDR_MASK));
	enc28j60_txrx_byte(data);
	enc28j60_release();
}

// Read PHY register
uint16_t enc28j60_read_phy(uint8_t adr)
{
	enc28j60_wcr(MIREGADR, adr);
	enc28j60_bfs(MICMD, MICMD_MIIRD);
	while(enc28j60_rcr(MISTAT) & MISTAT_BUSY)
		;
	enc28j60_bfc(MICMD, MICMD_MIIRD);
	return enc28j60_rcr16(MIRD);
}

// Write PHY register
void enc28j60_write_phy(uint8_t adr, uint16_t data)
{
	enc28j60_wcr(MIREGADR, adr);
	enc28j60_wcr16(MIWR, data);
	while(enc28j60_rcr(MISTAT) & MISTAT_BUSY)
		;
}