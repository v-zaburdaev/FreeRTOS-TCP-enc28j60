#ifndef LEDS_HPP
#define LEDS_HPP

#include "GPIO.hpp"

#include "stm32f4xx.h"


/**
 * Convenient names for LEDs
 */
#define GreenLed GPIO::Pin::P12
#define OrangeLed GPIO::Pin::P13
#define RedLed GPIO::Pin::P14
#define BlueLed GPIO::Pin::P15

/**
 * Implements basic operations with on-board leds.
 * Requires the __init_.hpp header to be included after this class inclusion.
 */
class Leds : public GPIO::GPIOPins {
    public:
		Leds(std::initializer_list<GPIO::Pin> leds);
};
#endif // LEDS_HPP