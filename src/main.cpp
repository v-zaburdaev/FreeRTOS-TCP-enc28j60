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

// TCP Stack - IP configuration
static uint8_t ucMACAddress[ 6 ] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
static const uint8_t ucIPAddress[ 4 ] = { 192, 168, 0, 2 };
static const uint8_t ucNetMask[ 4 ] = { 255, 255, 255, 0 };
static const uint8_t ucGatewayAddress[ 4 ] = { 192, 168, 0, 100 };

// The following is the address of an OpenDNS server.
static const uint8_t ucDNSServerAddress[ 4 ] = { 208, 67, 222, 222 };

void prvInit( void *pvParameters );
void prvPingTask(void *pvParameters);


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

void prvInit( void *pvParameters )
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

void prvPingTask(void *pvParameters)
{
    for(;;) {
        debug("Sending ping request...\n");
        FreeRTOS_SendPingRequest(FreeRTOS_inet_addr("192.168.0.1") , 8, 100 / portTICK_PERIOD_MS );
        vTaskDelay(2000);
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

