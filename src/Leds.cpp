#include "Leds.hpp"

Leds::Leds(std::initializer_list<GPIO::Pin> leds): GPIOPins{GPIOD, leds, GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low, GPIO::AlternateFunction::None} {}