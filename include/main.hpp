#ifndef MAIN_H
#define MAIN_H

#include "system_config.hpp"
#include "debug.hpp"
#include "Leds.hpp"
#include "interrupts.hpp"
extern "C" {
#include "FreeRTOS.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_Sockets.h"
}
#include "task.h"
#include "handlers.hpp"
#include "enc28j60.h"

extern "C" {
    // buffer allocation declaration
    void vReleaseNetworkBufferAndDescriptor( xNetworkBufferDescriptor_t * const pxNetworkBuffer );
    xNetworkBufferDescriptor_t *pxGetNetworkBufferWithDescriptor( size_t xRequestedSizeBytes, TickType_t xBlockTimeTicks );
}

#endif /* MAIN_H */
