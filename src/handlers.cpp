#include "handlers.hpp"


extern "C" {
    void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName) {
        debug("FATAL EXCEPTION: Stack overflow\n");
        exit(EXIT_FAILURE);
    }

    void SysTick_Handler(void)
    {
         HAL_IncTick();
         //xPortSysTickHandler();
    }

     void EXTI0_IRQHandler(void)
     {
          HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
     }

     void HAL_GPIO_EXTI_Callback(uint16_t pin)
     {
          EXTI0_IRQHandlerCpp();
     }
}

void EXTI0_IRQHandlerCpp()
{
    static int button_click_counter = 0;
    for (int i = 0; i <= 1000000; i++);
    for (int i = 0; i <= 1000000; i++);
    for (int i = 0; i <= 1000000; i++);
    for (int i = 0; i <= 1000000; i++);
    for (int i = 0; i <= 1000000; i++);
    button_click_counter++;
    debug("Button pressed %d times.", button_click_counter);

}
