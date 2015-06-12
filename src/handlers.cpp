#include "handlers.hpp"

int click_counter = 0;
void HttpserverTask( void *pvParameters );

extern "C" {
    void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName) {
        debug("FATAL EXCEPTION: Stack overflow\n");
        exit(EXIT_FAILURE);
    }

    void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
    {
        if( eNetworkEvent == eNetworkUp )
        {
            debug("vApplicationIP: network up.\n");
            if (xTaskCreate(HttpserverTask, "Httpserver", 12000, NULL, 1, NULL) != pdPASS)
                debug("!! Creation of Httpserver task failed.\n");
        } else if ( eNetworkEvent == eNetworkDown) {
            debug("vApplicationIP: network down.\n");
        }
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

    void SysTick_Handler(void)
    {
         HAL_IncTick();
         xPortSysTickHandler();
    }

    void EXTI0_IRQHandler()
    {
        if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) {
            __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
            static uint32_t tickstart = 0;
            if((HAL_GetTick() - tickstart) > 200) {
                debug("Software int...");
                EXTI2_IRQHandler();
                tickstart = HAL_GetTick();
            }
        }
    }

    void HAL_GPIO_EXTI_Callback(uint16_t pin)
    {
        EXTI0_IRQHandlerCpp();
    }
}
