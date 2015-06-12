#include "main.hpp"

#include "GPIO.hpp"
#include "Leds.hpp"
#include "Peripheral.hpp"

#include "server.hpp"
#include "request.hpp"
#include "response.hpp"
#include "constants.hpp"
#include "file_resources.hpp"

#include <string>
#include <unordered_map>


extern int click_counter;

static UBaseType_t ulNextRand=1234;
static void prvHttpserverTask( void *pvParameters );
static void prvRecvPacketTask( void *pvParameters );
static void prvInit( void *pvParameters );

void* operator new (std::size_t n) throw(std::bad_alloc)
{
    return pvPortMalloc(n);
}
void* operator new[](std::size_t n) throw(std::bad_alloc)
{
    return pvPortMalloc(n);
}
void  operator delete (void* p) noexcept
{
    vPortFree(p);
}
void  operator delete[](void* p) noexcept
{
    vPortFree(p);
}

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
        if (xTaskCreate(prvHttpserverTask, "Httpserver", 12000, NULL, 1, NULL) != pdPASS)
            debug("!! Creation of Httpserver task failed.\n");
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

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    FreeRTOS_IPInit( ucIPAddress,
                     ucNetMask,
                     ucGatewayAddress,
                     ucDNSServerAddress,
                     ucMACAddress );



    xTaskCreate(prvInit, "Init", 1000, NULL, 3, NULL);
    vTaskStartScheduler();

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

void prvHttpserverTask( void *pvParameters )
{
    std::unordered_map<std::string, Html_file*> file_umap = populate_file_umap();

    Server::create(80);

    debug("Listening...\n");
    Server::listen();

    // main loop
    while (1) {
        debug("Wait for connetion...\n");

        xSocket_t clientid = Server::accept();
        if (clientid != NULL) {
            debug("Connetion established...\n");

            std::string request = Server::receive(clientid);

            if (!request.empty()) {
                debug("== Received message: ==\n");

                Request_data req_data = Request::handle_request(request);
                debug("== Parsed args: ==\n%d %s\n", req_data.req_type, req_data.url.c_str());


                if (file_umap.count(req_data.url) == 0) {
                    std::string response_str = Response::generate_response(*file_umap["/404.html"], Response::type::NOT_FOUND);
                    debug("<< Sending message 404: >>..");
                    Server::send(clientid, response_str);
                    debug("...Done\n");
                } else {
                    std::string response_str = Response::generate_response(*file_umap[req_data.url], Response::type::OK);
                    debug("<< Sending message: >>..");
                    Server::send(clientid, response_str);
                    debug("...Done\n");
                }
            }
            FreeRTOS_shutdown(clientid, FREERTOS_SHUT_RDWR);
            FreeRTOS_closesocket(clientid);
        }

    }
}

void prvCheckFlagsTask(void *args)
{
    volatile uint8_t pktcnt, flags;
    while (1) {
        pktcnt = enc28j60_rcr(EPKTCNT);
        flags = enc28j60_rcr(EIR);
        debug("encPacketCNT: %d, flags: %x\n", pktcnt, flags);
        vTaskDelay(2500);
        //        __HAL_GPIO_EXTI_GENERATE_SWIT(GPIO_PIN_0);
    }
}

extern "C" { 
    void EXTI0_IRQHandler()
    {
        if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) {
            __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
            static uint32_t tickstart = 0;
            if((HAL_GetTick() - tickstart) > 200) {
                debug("Software int...");
                __HAL_GPIO_EXTI_GENERATE_SWIT(EXTI0_IRQn);
                tickstart = HAL_GetTick();
            }
        }
    }
}

static void prvInit( void *pvParameters )
{
    vTaskSuspendAll();

	GPIO_InitTypeDef GPIO_InitStruct;

    // Initialize button int
	GPIO_InitStruct.Pin       = GPIO_PIN_0;
    GPIO_InitStruct.Mode      = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = 0;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    IRQn_Type irqn_line = EXTI0_IRQn;
    HAL_NVIC_SetPriority(irqn_line, 7, 7);
    HAL_NVIC_EnableIRQ(irqn_line);

    debug("enc28j60: init\n");
    enc28j60_init(ucMACAddress);
    uint8_t revision_id = 0;
    revision_id = enc28j60_rcr(EREVID);
    debug("enc28j60: revision %#x\n", revision_id);
    debug("enc28j60: checked MAC address %x:%x:%x:%x:%x:%x filter: %x\n",
            enc28j60_rcr(MAADR5), enc28j60_rcr(MAADR4), enc28j60_rcr(MAADR3),
            enc28j60_rcr(MAADR2), enc28j60_rcr(MAADR1), enc28j60_rcr(MAADR0),
            enc28j60_rcr(ERXFCON));
    xTaskResumeAll();
    //xTaskCreate(prvCheckFlagsTask, "Check", 1000, NULL, 2, NULL);
    vTaskDelete(NULL);
}
