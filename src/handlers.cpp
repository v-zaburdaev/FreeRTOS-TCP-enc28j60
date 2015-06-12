#include "handlers.hpp"

int click_counter = 0;

extern "C" {
    void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName) {
        debug("FATAL EXCEPTION: Stack overflow\n");
        exit(EXIT_FAILURE);
    }

    void SysTick_Handler(void)
    {
         HAL_IncTick();
         xPortSysTickHandler();
    }

    void HAL_GPIO_EXTI_Callback(uint16_t pin)
    {
        EXTI0_IRQHandlerCpp();
    }
}

