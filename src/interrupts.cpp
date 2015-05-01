#include "interrupts.hpp"

namespace Interrupts {
    void EXTIInt::enable_int(GPIO_TypeDef *peripheral, std::initializer_list<GPIO::Pin> pins, Mode interrupt_mode,
            IRQn_Type irqn_type, int32_t PreemptionPriority, uint32_t SubPriority)
    {
        interrupt_list.push_back(new GPIO::GPIOPins(peripheral, pins, static_cast<GPIO::Mode>(interrupt_mode), GPIO::Pull::NoPull, GPIO::Speed::High));
        HAL_NVIC_SetPriority(irqn_type, PreemptionPriority, SubPriority);
        HAL_NVIC_EnableIRQ(irqn_type);
    }

    void EXTIInt::disable_int(GPIO_TypeDef *peripheral, std::initializer_list<GPIO::Pin> pins, IRQn_Type irqn_type)
    {
        std::bitset<16> pins_bitmask;
        for (const auto &pin : pins) {
            pins_bitmask |= std::bitset<16>(static_cast<uint16_t>(pin));
        }

        auto list_iter = find_gpiopins(interrupt_list, peripheral, pins_bitmask);
        if (list_iter == interrupt_list.end()) {
            debug("Interrupt GPIO: %x , pins_bitmask: %x hasn't been found.\n",
                    reinterpret_cast<unsigned int>(peripheral),
                    static_cast<unsigned int>(pins_bitmask.to_ulong()));
            return;
        } else {
            delete (*list_iter);
            interrupt_list.erase(list_iter);
        }
    }

    std::list<GPIO::GPIOPins *>::iterator EXTIInt::find_gpiopins(std::list<GPIO::GPIOPins *> gpiopins_list,
                                                        GPIO_TypeDef *peripheral, std::bitset<16> pins_bitmask)
    {
        auto return_list_iter = gpiopins_list.end();
        for (auto list_iter = gpiopins_list.begin();list_iter != gpiopins_list.end(); list_iter++)
        {
            if ( (*list_iter)->get_peripheral() == peripheral
               && (*list_iter)->get_pins_bitmask() == pins_bitmask) {
                return_list_iter = list_iter;
                break;
            }

        }
        return return_list_iter;
    }
    std::list<GPIO::GPIOPins *> EXTIInt::interrupt_list;
}

