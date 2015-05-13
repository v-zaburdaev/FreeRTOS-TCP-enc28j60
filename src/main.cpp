#include "main.hpp"

extern "C" {
#include "enc28j60.h"
    // buffer allocation declaration
    void vReleaseNetworkBufferAndDescriptor( xNetworkBufferDescriptor_t * const pxNetworkBuffer );
    xNetworkBufferDescriptor_t *pxGetNetworkBufferWithDescriptor( size_t xRequestedSizeBytes, TickType_t xBlockTimeTicks );
}

#include "GPIO.hpp"
#include "Leds.hpp"
#include "Peripheral.hpp"
#include "FreeRTOS_IP_Private.h"


extern int click_counter;


static UBaseType_t ulNextRand=1234;
static void prvRecvPacketTask( void *pvParameters );
static void prvInitEnc28j60( void *pvParameters );

UBaseType_t uxRand( void )
{
const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

	/* Utility function to generate a pseudo random number. */

	ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
	return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}

void prvPingTask(void *pvParameters);

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    if( eNetworkEvent == eNetworkUp )
    {
        debug("vApplicationIP: network up.\n");
    } else if ( eNetworkEvent == eNetworkDown) {
        debug("vApplicationIP: network down.\n");
    }
}

static uint8_t ucMACAddress[ 6 ] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

/* Define the network addressing.  These parameters will be used if either
ipconfigUDE_DHCP is 0 or if ipconfigUSE_DHCP is 1 but DHCP auto configuration
failed. */
static const uint8_t ucIPAddress[ 4 ] = { 192, 168, 0, 2 };
static const uint8_t ucNetMask[ 4 ] = { 255, 255, 255, 0 };
static const uint8_t ucGatewayAddress[ 4 ] = { 192, 168, 0, 100 };

/* The following is the address of an OpenDNS server. */
static const uint8_t ucDNSServerAddress[ 4 ] = { 208, 67, 222, 222 };

int main()
{
    /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, Flash preread and Buffer caches
       - Systick timer is configured by default as source of time base, but user
             can eventually implement his proper time base source (a general purpose
             timer for example or other time source), keeping in mind that Time base
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
             handled in milliseconds basis.
       - Low Level Initialization
     */
    HAL_Init();

    /* Configure the system clock to 168 MHz */
    SystemClock_Config();
    

//    FreeRTOS_IPInit( ucIPAddress,
//                     ucNetMask,
//                     ucGatewayAddress,
//                     ucDNSServerAddress,
//                     ucMACAddress );
 uint8_t arp_req[] = {
       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x08, 0x06, 0x00, 0x01,
       0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xc0, 0xa8, 0x00, 0x02,
       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x00, 0x01
   }; 

    debug("enc28j60: init\n");
    enc28j60_init(ucMACAddress);
    uint8_t revision_id = 0;
    revision_id = enc28j60_rcr(EREVID);
    debug("enc28j60: revision %#x\n", revision_id);
    debug("enc28j60: checked MAC address %x:%x:%x:%x:%x:%x filter: %x\n",
            enc28j60_rcr(MAADR5), enc28j60_rcr(MAADR4), enc28j60_rcr(MAADR3),
            enc28j60_rcr(MAADR2), enc28j60_rcr(MAADR1), enc28j60_rcr(MAADR0),
            enc28j60_rcr(ERXFCON));

   /* arp testing */
   uint8_t network_buf[256] = {0};
   int length;
   for (;;) {
       HAL_Delay(1000);
           debug("Sending arp...\n");
           enc28j60_send_packet(arp_req, sizeof(arp_req));
           length = enc28j60_recv_packet(network_buf, sizeof(network_buf));
           debug("Received response (len: %d)\n", length);
           for (int i = 0; i < 256; i += 16) {
               debug("\t%#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x\n",
                       network_buf[i + 0], network_buf[i + 1], network_buf[i + 2], network_buf[i + 3],
                       network_buf[i + 4], network_buf[i + 5], network_buf[i + 6], network_buf[i + 7],
                       network_buf[i + 8], network_buf[i + 9], network_buf[i + 10], network_buf[i + 11],
                       network_buf[i + 12], network_buf[i + 13], network_buf[i + 14], network_buf[i + 15]);
           }
   }
//    xTaskCreate(prvInitEnc28j60, "Init", 600, NULL, 3, NULL);
//    vTaskStartScheduler();

    return 0;
}

void vApplicationPingReplyHook( ePingReplyStatus_t eStatus, uint16_t usIdentifier )
{
    switch( eStatus )
    {
        case eSuccess    :
            /* A valid ping reply has been received.  Post the sequence number
            on the queue that is read by the vSendPing() function below.  Do
            not wait more than 10ms trying to send the message if it cannot be
            sent immediately because this function is called from the TCP/IP
            RTOS task - blocking in this function will block the TCP/IP RTOS task. */
            debug("vAppPingReply: Received ping reply.\n");
            break;

        case eInvalidChecksum :
        case eInvalidData :
            /* A reply was received but it was not valid. */
            break;
    }
}

/* The deferred interrupt handler is a standard RTOS task.  FreeRTOS's centralised
   deferred interrupt handling capabilities can also be used. */
void prvRecvPacketTask( void *pvParameters )
{
    xNetworkBufferDescriptor_t *pxBufferDescriptor;
    /* Used to indicate that xSendEventStructToIPTask() is being called because
       of an Ethernet receive event. */
    xIPStackEvent_t xRxEvent;

    uint16_t xBytesReceived = 0, rxlen, status, temp;
    for ( ;; )
    {

        if (enc28j60_rcr(EPKTCNT) == 0) { // when there is no packet
            taskYIELD();
        } else {
            // Set read pointer of enc28j60 to read new packet
            enc28j60_wcr16(ERDPT, enc28j60_rxrdpt);

            // Read information about packet
            enc28j60_read_buffer(reinterpret_cast<volatile uint8_t *>(&enc28j60_rxrdpt), sizeof(enc28j60_rxrdpt));
            enc28j60_read_buffer(reinterpret_cast<uint8_t *>(&rxlen), sizeof(rxlen));
            enc28j60_read_buffer(reinterpret_cast<uint8_t *>(&status), sizeof(status));

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

                    // Set enc28j60 Rx read pointer to next packet
                    temp = (enc28j60_rxrdpt - 1) & ENC28J60_BUFEND;
                    enc28j60_wcr16(ERXRDPT, temp);

                    // Decrement packet counter
                    enc28j60_bfs(ECON2, ECON2_PKTDEC);

                    /* See if the data contained in the received Ethernet frame needs
                       to be processed.  NOTE! It is preferable to do this in
                       the interrupt service routine itself, which would remove the need
                       to unblock this task for packets that don't need processing. */
                    if( eConsiderFrameForProcessing( pxBufferDescriptor->pucEthernetBuffer )
                            == eProcessBuffer )
                    {
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
                        /* The Ethernet frame can be dropped, but the Ethernet buffer
                           must be released. */
                        vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );
                    }
                }
                else
                {
                    /* The event was lost because a network buffer was not available.
                       Call the standard trace macro to log the occurrence. */
                    iptraceETHERNET_RX_EVENT_LOST();
                }
            }
        }
    } // end of infinite loop
}

void prvPingTask(void *pvParameters)
{
    for(;;) {
        debug("Sending ping request...\n");
        FreeRTOS_SendPingRequest(FreeRTOS_inet_addr("192.168.0.1") , 8, 100 / portTICK_PERIOD_MS );
        vTaskDelay(2000);
    }
}

static void prvInitEnc28j60( void *pvParameters )
{
    uint8_t revision_id = 0;
    revision_id = enc28j60_rcr(EREVID) >> 3;
    debug("enc28j60: revision %#x\n", revision_id);
    debug("enc28j60: init\n");
    enc28j60_init(ucMACAddress);
    debug("enc28j60: checked MAC address %x:%x:%x:%x:%x:%x filter: %x\n",
            enc28j60_rcr(MAADR5), enc28j60_rcr(MAADR4), enc28j60_rcr(MAADR3),
            enc28j60_rcr(MAADR2), enc28j60_rcr(MAADR1), enc28j60_rcr(MAADR0),
            enc28j60_rcr(ERXFCON));

    xTaskCreate(prvRecvPacketTask, "Recv_packet", 5000, NULL, 3, NULL);
    xTaskCreate(prvPingTask, "Pinging", 1000, NULL, 3, NULL);

    vTaskDelete(NULL);
}
