#include "GPIO.hpp"

namespace GPIO {
    GPIOPins::GPIOPins(GPIO_TypeDef *peripheral, std::initializer_list<Pin> pins, Mode mode, Pull pull, Speed speed, AlternateFunction alternate /*= AlternateFunction::None*/) {
        this->peripheral = peripheral;
        
        for (const auto &pin : pins) {
            pins_bitmask |= std::bitset<16>(static_cast<uint16_t>(pin));
        }
        
        if (std::find(initialized_peripherals.begin(), initialized_peripherals.end(), peripheral) == initialized_peripherals.end()) {
            if (peripheral == GPIOA)
                __GPIOA_CLK_ENABLE();
            else if (peripheral == GPIOB)
                __GPIOB_CLK_ENABLE();
            else if (peripheral == GPIOC)
                __GPIOC_CLK_ENABLE();
            else if (peripheral == GPIOD)
                __GPIOD_CLK_ENABLE();
            else if (peripheral == GPIOE)
                __GPIOE_CLK_ENABLE();
            else if (peripheral == GPIOF)
                __GPIOF_CLK_ENABLE();
            else if (peripheral == GPIOG)
                __GPIOG_CLK_ENABLE();
            else if (peripheral == GPIOH)
                __GPIOH_CLK_ENABLE();
            else if (peripheral == GPIOI)
                __GPIOI_CLK_ENABLE();
        }
        
        initialized_peripherals.push_back(peripheral);
        
        auto init_configuration = GPIO_InitTypeDef();
        init_configuration.Pin = pins_bitmask.to_ulong();
        init_configuration.Mode = static_cast<uint32_t>(mode);
        init_configuration.Pull = static_cast<uint32_t>(pull);
        init_configuration.Speed = static_cast<uint32_t>(speed);        
        if (alternate != AlternateFunction::None)
            init_configuration.Alternate = static_cast<uint8_t>(alternate);
        
        HAL_GPIO_Init(peripheral, &init_configuration);
    }
    
    GPIOPins::~GPIOPins() {
        HAL_GPIO_DeInit(peripheral, pins_bitmask.to_ulong());
        
        auto element_to_erase = std::find(initialized_peripherals.begin(), initialized_peripherals.end(), peripheral);
        initialized_peripherals.erase(element_to_erase);
        
        if (std::find(initialized_peripherals.begin(), initialized_peripherals.end(), peripheral) == initialized_peripherals.end()) {
            if (peripheral == GPIOA)
                __GPIOA_CLK_DISABLE();
            else if (peripheral == GPIOB)
                __GPIOB_CLK_DISABLE();
            else if (peripheral == GPIOC)
                __GPIOC_CLK_DISABLE();
            else if (peripheral == GPIOD)
                __GPIOD_CLK_DISABLE();
            else if (peripheral == GPIOE)
                __GPIOE_CLK_DISABLE();
            else if (peripheral == GPIOF)
                __GPIOF_CLK_DISABLE();
            else if (peripheral == GPIOG)
                __GPIOG_CLK_DISABLE();
            else if (peripheral == GPIOH)
                __GPIOH_CLK_DISABLE();
            else if (peripheral == GPIOI)
                __GPIOI_CLK_DISABLE();
        }
    }
    
    void GPIOPins::turn_on() {
        HAL_GPIO_WritePin(peripheral, pins_bitmask.to_ulong(), GPIO_PinState(GPIO_PIN_SET));
    }
    
    void GPIOPins::turn_off() {
        HAL_GPIO_WritePin(peripheral, pins_bitmask.to_ulong(), GPIO_PinState(GPIO_PIN_RESET));
    }

    void GPIOPins::set_state(bool state) {
        if(state) turn_on();
        else turn_off();
    }

    bool GPIOPins::get_state() {
        return HAL_GPIO_ReadPin(peripheral, pins_bitmask.to_ulong());
    }
    
    std::list<GPIO_TypeDef *> GPIOPins::initialized_peripherals = std::list<GPIO_TypeDef *>();
}
