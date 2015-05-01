#include "main.hpp"

#include "GPIO.hpp"
#include "Leds.hpp"
#include "Peripheral.hpp"

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
    
    // Configure button interrupt
    //Interrupts::EXTIInt::enable_int(GPIOA, {GPIO::Pin::P0}, Interrupts::Mode::FallingEdgeInterrupt, EXTI0_IRQn, 2, 0);

    GPIO::GPIOPins led(GPIOD, {GPIO::Pin::P12}, GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low);
    GPIO::GPIOPins testowy(GPIOA, {GPIO::Pin::P6}, GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low);

    Peripheral::Screen * screen = new Peripheral::Screen;
    screen->Initialize();

    led.turn_on();

    screen->WriteString("NAJLEPSZY EKRAN");
    screen->SetCursorPosition(1,0);
    screen->WriteString("hehe smieszne");

    for(;;)
    {
        
    }

    return 0;
}
