#include "enc28j60.h"

#ifdef DEBUG
    #include <stdio.h>
    
    #define debug(format, ...)     printf(format,##__VA_ARGS__)
    
    /*
     * Disable STDOUT buffering to enable printing before a newline 
     * character or buffer flush.
     */
#else
    #define debug(format, ...)
#endif /* DEBUG */

// private functions declarations
static void enc28j60_select();
static void enc28j60_release();
static void enc28j60_soft_reset();
static uint8_t enc28j60_txrx_byte(uint8_t data);
static uint8_t enc28j60_read_op(uint8_t cmd, uint8_t adr);
static void enc28j60_write_op(uint8_t cmd, uint8_t adr, uint8_t data);
static void prvReceivePacket(void *buf, uint32_t pktlen);
static void prvSoftTimerInt(void *unused, uint32_t unused2);
// private global variables definition
static SPI_HandleTypeDef SpiHandle;

void *pvPortMallocISR( size_t xWantedSize );
size_t xPortGetFreeHeapSize( void );

volatile uint8_t enc28j60_current_bank = 0;
volatile uint16_t enc28j60_rxrdpt = 0;

void EXTI2_IRQHandler(void)
{
//    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET) {

        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (xTimerPendFunctionCallFromISR( prvReceivePacket,
                    NULL,
                    0,
                    &xHigherPriorityTaskWoken ) == pdFALSE) {
            debug("PANIC: enc28j60: daemon's queue full\n");
            while(1);
        }
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
 //   }
}

void prvSoftTimerInt(void *unused, uint32_t unused2)
{
    HAL_Delay(10);
    HAL_SPI_StateTypeDef spi_state = HAL_SPI_GetState(&SpiHandle);
    if (enc28j60_rcr(EIR) == 0xFF) {
        if (xTimerPendFunctionCall( prvSoftTimerInt, NULL, 0, 10) == pdFALSE) {
            debug("PANIC: enc28j60: daemon's queue full\n");
            while(1);
        }
    } else {
       taskENTER_CRITICAL();
       EXTI2_IRQHandler();
       taskEXIT_CRITICAL();
    }

}

    uint8_t enc28j60_init(uint8_t *macadr)
    {
	uint32_t i;
    
    // Initialize clock on SPI and for MISO/MOSI
    __HAL_RCC_SPI2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitTypeDef GPIO_InitStruct;

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
    // GPIO_InitStruct.Pull      = GPIO_NOPULL;
    // GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    // GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Initialize MISO
	GPIO_InitStruct.Pin       = GPIO_PIN_14;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    // GPIO_InitStruct.Pull      = GPIO_NOPULL;
    // GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    // GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Initialize SS (Slave Select, a.k.a. CS)
	GPIO_InitStruct.Pin       = GPIO_PIN_12;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    // GPIO_InitStruct.Pull      = GPIO_NOPULL;
    // GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    // GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Set RESET pin
	GPIO_InitStruct.Pin       = GPIO_PIN_1;
    GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
    // GPIO_InitStruct.Pull      = GPIO_NOPULL;
    // GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = 0;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Set INTERRUPT pin
	GPIO_InitStruct.Pin       = GPIO_PIN_2;
    GPIO_InitStruct.Mode      = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = 0;

    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    IRQn_Type irqn_line = EXTI2_IRQn;
    HAL_NVIC_SetPriority(irqn_line, 13, 0);

    // Initialize SPI
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
        debug("SPI: initialization error at %s, line %d .\n", __FILE__, __LINE__);
        return 1;
    }

    enc28j60_release();
    for (i = 0; i < 720000; i++) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    }
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);

    // Reset ENC28J60
    enc28j60_soft_reset();

    // Setup Rx/Tx buffer
    enc28j60_wcr16(ERXST, ENC28J60_RXSTART);
    enc28j60_rcr16(ERXST);
    enc28j60_wcr16(ERXRDPT, ENC28J60_RXSTART);
    enc28j60_wcr16(ERXND, ENC28J60_RXEND);
    enc28j60_rxrdpt = ENC28J60_RXSTART;

    // set read packet pointer
    enc28j60_wcr16(ERDPT, enc28j60_rxrdpt);

    // set autoincrement of pointers mode
    enc28j60_bfs(ECON2, ECON2_AUTOINC);
    
    // Setup MAC
    enc28j60_wcr(MACON1, MACON1_TXPAUS| // Enable flow control
    MACON1_RXPAUS|MACON1_MARXEN); // Enable MAC Rx
    enc28j60_wcr(MACON2, 0); // Clear reset
    enc28j60_wcr(MACON3, 
            MACON3_PADCFG0 | MACON3_PADCFG2 | // Enable padding and automatic vlan frames recognition
            MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX); // Enable crc & frame len chk
    enc28j60_wcr16(MAMXFL, ENC28J60_MAXFRAME);
    enc28j60_wcr(MABBIPG, 0x15); // Set inter-frame gap
    enc28j60_wcr(MAIPGL, 0x12);
    enc28j60_wcr(MAIPGH, 0x0c); // ICE
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

    // Enable interrupt line
    HAL_NVIC_EnableIRQ(irqn_line);
    
    // Enable enc28j60 receive packet pending interrupt
    // and transmit and receive error interrupt
    enc28j60_bfs(EIE, EIE_INTIE | EIE_TXIE | EIE_PKTIE | EIE_TXERIE | EIE_RXERIE);
    // Enable Rx packets
    // enc28j60_wcr(ERXFCON, 0x9F); // packet filtering
    enc28j60_bfs(ECON1, ECON1_RXEN);
    
    return 0;
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

void prvReceivePacket(void *buf, uint32_t pktlen)
{
        enc28j60_bfc(EIE, EIE_INTIE); // mask enc28j60 interrupts

        volatile uint8_t eir_flags = enc28j60_rcr(EIR);

        if(eir_flags & EIR_PKTIF) { /* if there is pending packet */
            // retrieve packet from enc28j60
            uint16_t xBytesReceived = 0, rxlen, status, temp;
            xNetworkBufferDescriptor_t *pxBufferDescriptor;
            xIPStackEvent_t xRxEvent;

            enc28j60_read_buffer((void *)(&enc28j60_rxrdpt), sizeof(enc28j60_rxrdpt));
            enc28j60_read_buffer((void *)(&rxlen), sizeof(rxlen));
            enc28j60_read_buffer((void *)(&status), sizeof(status));

            if(status & 0x80) //success
            {
                // Throw out crc
                xBytesReceived = rxlen - 4;

                // Allocate buffer for packet
                pxBufferDescriptor = pxGetNetworkBufferWithDescriptor( xBytesReceived, 0 );

                if( pxBufferDescriptor != NULL )
                {
                    // Read packet content
                    enc28j60_read_buffer( pxBufferDescriptor->pucEthernetBuffer, xBytesReceived );
                    pxBufferDescriptor->xDataLength = xBytesReceived;

                    /* The event about to be sent to the TCP/IP is an Rx event. */
                    xRxEvent.eEventType = eNetworkRxEvent;

                    /* pvData is used to point to the network buffer descriptor that
                       now references the received data. */
                    xRxEvent.pvData = ( void * ) pxBufferDescriptor;

                    /* Send the data to the TCP/IP stack. */
                    if( xSendEventStructToIPTask( &xRxEvent, 0 ) == pdFALSE )
                    {
                        /* The buffer could not be sent to the IP task so the buffer
                           must be released. */
                        vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );

                        /* Make a call to the standard trace macro to log the
                           occurrence. */
                        iptraceETHERNET_RX_EVENT_LOST();
                    }
                    else
                    {
                        /* The message was successfully sent to the TCP/IP stack.
                           Call the standard trace macro to log the occurrence. */
                        iptraceNETWORK_INTERFACE_RECEIVE();
                    }
                }
                else
                {
                    /* The event was lost because a network buffer was not available.
                       Call the standard trace macro to log the occurrence. */
                    iptraceETHERNET_RX_EVENT_LOST();
                }
            }

            // Set Rx buffer guard to next packet
            if (enc28j60_rxrdpt == ENC28J60_RXSTART)
                temp = ENC28J60_RXEND;
            else
                temp = enc28j60_rxrdpt - 1;

            enc28j60_wcr16(ERXRDPT, temp);

            // Set Rx pointer to next packet
            enc28j60_wcr16(ERDPT, enc28j60_rxrdpt);

            // Decrement packet counter
            enc28j60_bfs(ECON2, ECON2_PKTDEC);

        } else if (eir_flags & EIR_TXIF) {
            debug("enc28j60: transmit done\n");
            enc28j60_bfc(EIR, EIR_TXIF);
        } else if (eir_flags & EIR_TXERIF) {
            debug("enc28j60: transmit error !!\n");
            enc28j60_bfc(EIR, EIR_TXERIF);
        } else if (eir_flags & EIR_RXERIF) {
            debug("enc28j60: receive error !!\n");
            enc28j60_bfc(EIR, EIR_RXERIF);
        } else {
            debug("enc28j60: unknown interrupt flag, we shouldn't be here\n");
        }

        enc28j60_bfs(EIE, EIE_INTIE); // unmask enc28j60 interrupts
}

void enc28j60_send_packet(uint8_t *data, uint16_t len)
{

    uint32_t tickstart = HAL_GetTick();
	while(enc28j60_rcr(ECON1) & ECON1_TXRTS)
	{
		// TXRTS may not clear - ENC28J60 bug. We must reset
		// transmit logic in cause of Tx error
        
		if(enc28j60_rcr(EIR) & EIR_TXERIF)
		{
			enc28j60_bfs(ECON1, ECON1_TXRST);
			enc28j60_bfc(ECON1, ECON1_TXRST);
		}

        // If previous packet won't be sended within 100 ms, abort previous packet
        if((HAL_GetTick() - tickstart) > 100) {
            enc28j60_bfc(ECON1, ECON1_TXRTS);
        }
	}

	enc28j60_wcr16(EWRPT, ENC28J60_TXSTART);
	enc28j60_write_buffer((uint8_t*)"\x00", 1);
	enc28j60_write_buffer(data, len);

	enc28j60_wcr16(ETXST, ENC28J60_TXSTART);
	enc28j60_wcr16(ETXND, ENC28J60_TXSTART + len);

	enc28j60_bfs(ECON1, ECON1_TXRTS); // Request packet send
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

static void enc28j60_select()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

static void enc28j60_release()
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

static void enc28j60_soft_reset()
{
	enc28j60_select();
	enc28j60_txrx_byte(ENC28J60_SPI_SC);
	enc28j60_release();
	
	enc28j60_current_bank = 0;
    HAL_Delay(5);
	//_delay_ms(1); // Wait until device initializes
}

static uint8_t enc28j60_txrx_byte(uint8_t data)
{
    uint8_t recv_data;

    taskENTER_CRITICAL();
    switch(HAL_SPI_TransmitReceive(&SpiHandle, &data, &recv_data, 1, 100)) {
        case HAL_OK:  
            break;
        case HAL_BUSY:
            debug("PANIC: SPI: busy state occured at %s, line %d .\n", __FILE__, __LINE__);
            while(1);
            break;
        case HAL_TIMEOUT:
            debug("PANIC: SPI: transmit/receive timeout occured at %s, line %d .\n", __FILE__, __LINE__);
            while(1);
            break;  
        case HAL_ERROR:
            debug("PANIC: SPI: transmit/receive error at %s, line %d.\n", __FILE__, __LINE__);
            while(1);
            break;
        default:
            debug("PANIC: SPI: transmit/receive WUT HAPPEND at %s, line %d.\n", __FILE__, __LINE__);
            while(1);
            break;
    }
    taskEXIT_CRITICAL();

    return recv_data;
}

// Generic SPI read command
static uint8_t enc28j60_read_op(uint8_t cmd, uint8_t adr)
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
static void enc28j60_write_op(uint8_t cmd, uint8_t adr, uint8_t data)
{
	enc28j60_select();
	enc28j60_txrx_byte(cmd | (adr & ENC28J60_ADDR_MASK));
	enc28j60_txrx_byte(data);
	enc28j60_release();
}

