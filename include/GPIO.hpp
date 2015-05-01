#ifndef GPIO_HPP
#define GPIO_HPP

#include <bitset>
#include <list>
#include <algorithm>

#include "stm32f4xx_hal.h"

namespace GPIO {
    /**
     * Contains convenient names for GPIO pins.
     */
    enum class Pin : uint16_t {
        P0 = GPIO_PIN_0,
        P1 = GPIO_PIN_1,
        P2 = GPIO_PIN_2,
        P3 = GPIO_PIN_3,
        P4 = GPIO_PIN_4,
        P5 = GPIO_PIN_5,
        P6 = GPIO_PIN_6,
        P7 = GPIO_PIN_7,
        P8 = GPIO_PIN_8,
        P9 = GPIO_PIN_9,
        P10 = GPIO_PIN_10,
        P11 = GPIO_PIN_11,
        P12 = GPIO_PIN_12,
        P13 = GPIO_PIN_13,
        P14 = GPIO_PIN_14,
        P15 = GPIO_PIN_15
    };


    /**
     * Specifies the operating mode for the selected pins: GPIO mode or EXTI mode.
     */
    enum class Mode : uint32_t {
        Input = GPIO_MODE_INPUT, // Input Floating
        OutputPushPull = GPIO_MODE_OUTPUT_PP, // Output Push Pull
        OutputOpenDrain = GPIO_MODE_OUTPUT_OD, // Output Open Drain
        AlternateFunctionPushPull = GPIO_MODE_AF_PP, // Alternate Function Push Pull
        AlternateFunctionOpenDrain = GPIO_MODE_AF_OD, // Alternate Function Open Drain
        Analog = GPIO_MODE_ANALOG, // Analog mode
        RisingEdgeInterrupt = GPIO_MODE_IT_RISING, // Rising edge trigger detection
        FallingEdgeInterrupt = GPIO_MODE_IT_FALLING, // Falling edge trigger detection
        RisingAndFallingEdgeInterrupt = GPIO_MODE_IT_RISING_FALLING // Rising/Falling edge trigger 
    };


    /**
     * Specifies the Pull-up or Pull-down activation for the selected pins.
     */
    enum class Pull : uint32_t {
        NoPull = GPIO_NOPULL,
        PullUp = GPIO_PULLUP,
        PullDown = GPIO_PULLDOWN
    };


    /**
     * Specifies the speed for the selected pins.
     */
    enum class Speed : uint32_t {
        Low = GPIO_SPEED_LOW,
        Medium = GPIO_SPEED_MEDIUM,
        Fast = GPIO_SPEED_FAST,
        High = GPIO_SPEED_HIGH
    };


    /**
     * Specifies peripheral to be connected to the selected pins.
     */
    enum class AlternateFunction : uint8_t {
        
        AlternateFunction_1_Timer_1 = GPIO_AF1_TIM1,
        AlternateFunction_1_Timer_2 = GPIO_AF1_TIM2,
        AlternateFunction_2_Timer_3 = GPIO_AF2_TIM3,
        AlternateFunction_2_Timer_4 = GPIO_AF2_TIM4,
        AlternateFunction_2_Timer_5 = GPIO_AF2_TIM5,
        AlternateFunction_3_Timer_8 = GPIO_AF3_TIM8,
        AlternateFunction_3_Timer_9 = GPIO_AF3_TIM9,
        AlternateFunction_3_Timer_10 = GPIO_AF3_TIM10,
        AlternateFunction_3_Timer_11 = GPIO_AF3_TIM11,
        AlternateFunction_9_Timer_12 = GPIO_AF9_TIM12,
        AlternateFunction_9_Timer_13 = GPIO_AF9_TIM13,
        AlternateFunction_9_Timer_14 = GPIO_AF9_TIM14,
        AlternateFunction_5_SPI_1 = GPIO_AF5_SPI1,
        AlternateFunction_5_SPI_2 = GPIO_AF5_SPI2,
        // AlternateFunction_5_SPI_3 = GPIO_AF5_SPI3,
        // AlternateFunction_5_SPI_4 = GPIO_AF5_SPI4,
        // AlternateFunction_5_SPI_5 = GPIO_AF5_SPI5,
        // AlternateFunction_5_SPI_6 = GPIO_AF5_SPI6,
        // AlternateFunction_6_SPI_2 = GPIO_AF6_SPI2,
        AlternateFunction_6_SPI_3 = GPIO_AF6_SPI3,
        // AlternateFunction_6_SPI_4 = GPIO_AF6_SPI4,
        // AlternateFunction_6_SPI_5 = GPIO_AF6_SPI5,
        None = 0
    };


    /**
     * Implements basic operations with on-board leds.
     * Requires the __init_.hpp header to be included after this class inclusion.
     */
    class GPIOPins {
        private:
            static std::list<GPIO_TypeDef *> initialized_peripherals;
            GPIO_TypeDef *peripheral;
            std::bitset<16> pins_bitmask = std::bitset<16>();
        public:
            GPIOPins(GPIO_TypeDef *peripheral, std::initializer_list<Pin> pins, Mode mode, Pull pull, Speed speed, AlternateFunction alternate = AlternateFunction::None);
            ~GPIOPins();
            void turn_on();
            void turn_off();
            bool get_state();
            void set_state(bool state);
            GPIO_TypeDef* get_peripheral();
            std::bitset<16> get_pins_bitmask();
    };

    inline GPIO_TypeDef* GPIOPins::get_peripheral()
    {
        return peripheral;
    }

    inline std::bitset<16> GPIOPins::get_pins_bitmask()
    {
        return pins_bitmask;
    }
}
#endif // GPIO_HPP
