#ifndef INTERRUPTS_HPP
#define INTERRUPTS_HPP

#include "GPIO.hpp"

#include <list>

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "debug.hpp"

namespace Interrupts {
    enum class Mode : uint32_t {
        RisingEdgeInterrupt = GPIO_MODE_IT_RISING, // Rising edge trigger detection
        FallingEdgeInterrupt = GPIO_MODE_IT_FALLING, // Falling edge trigger detection
        RisingAndFallingEdgeInterrupt = GPIO_MODE_IT_RISING_FALLING // Rising/Falling edge trigger 
    };

    class EXTIInt {
        public:
            static void enable_int(GPIO_TypeDef *peripheral, std::initializer_list<GPIO::Pin> pins, Mode interrupt_mode,
                                   IRQn_Type irqn_type, int32_t PreemptionPriority, uint32_t SubPriority);
            static void disable_int(GPIO_TypeDef *peripheral, std::initializer_list<GPIO::Pin> pins, IRQn_Type irqn_type);

            static std::list<GPIO::GPIOPins *> interrupt_list;
        private:
            static std::list<GPIO::GPIOPins *>::iterator find_gpiopins(std::list<GPIO::GPIOPins *> gpiopins_list,
                                                        GPIO_TypeDef *peripheral, std::bitset<16> pins_bitmask);

    };
}

#endif /* end of include guard: INTERRUPTS_HPP */

