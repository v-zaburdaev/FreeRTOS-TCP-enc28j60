#include "FreeRTOSConfig.h"

namespace ADC {
    enum class ClockPrescaler : uint32_t {
        DIV2 = ADC_CLOCKPRESCALER_PCLK_DIV2,
        DIV4 = ADC_CLOCKPRESCALER_PCLK_DIV4,
        DIV6 = ADC_CLOCKPRESCALER_PCLK_DIV6,
        DIV8 = ADC_CLOCKPRESCALER_PCLK_DIV8
    }
    
    enum class Resolution : uint32_t {
        6b = ADC_RESOLUTION6b,
        8b = ADC_RESOLUTION8b,
        10b = ADC_RESOLUTION10b,
        12b = ADC_RESOLUTION12b
    }
    
    enum class DataAlign : uint32_t {
        Right = ADC_DATAALIGN_RIGHT,
        Left = ADC_DATAALIGN_LEFT
    }
    
    enum class ConversionMode : bool {
        MultiChannel = ENABLE,
        SingleChannel = DISABLE
    }
    
    enum class EOCSelection : uint32_t {
        EndOfSingleConversion = EOC_SINGLE_CONV,
        EndOfAllConversions = EOC_SEQ_CONV
    }
    
    enum class ConversionMode : bool {
        Continuous = ENABLE,
        Single = DISABLE
    }
    
    enum class DMARequestsMode : bool {
        Continuous = ENABLE,
        Single = DISABLE
    }
    
    class ADCPort() {
        public:
            ADCPort(uint32_t clock_prescaler, );
            ~ADCPort();
    };

}