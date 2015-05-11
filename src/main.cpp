#include "main.hpp"
extern "C" {
#include "enc28j60.h"
}

#include "GPIO.hpp"
#include "Leds.hpp"
#include "Peripheral.hpp"


extern int click_counter;

static UBaseType_t ulNextRand=1234;

UBaseType_t uxRand( void )
{
const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

	/* Utility function to generate a pseudo random number. */

	ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
	return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}

void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    if( eNetworkEvent == eNetworkUp )
    {
        debug("vApplicationIP: network up.\n");
    } else if ( eNetworkEvent == eNetworkDown) {
        debug("vApplicationIP: network down.\n");
    }
}
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

    // GPIO::GPIOPins led(GPIOD, {GPIO::Pin::P12}, GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low);
    // GPIO::GPIOPins testowy(GPIOA, {GPIO::Pin::P6}, GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low);

    // Peripheral::Screen * screen = new Peripheral::Screen;
    // screen->Initialize();

    // led.turn_on();

    // screen->WriteString("NAJLEPSZY EKRAN");
    // screen->SetCursorPosition(1,0);
    // screen->WriteString("hehe smieszne");
    uint8_t mac_address[6] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

    /*
    uint8_t ping_echo_array[] = {
        0x3c, 0x97, 0x0e, 0xd0, 0x6f, 0xb1,
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 
        0x08, 0x00, 0x45, 0x00, 0x00, 0x54, 0xd4, 0xad, 0x40, 0x00, 0x40, 0x01, 0x67, 0xf9, 0xc0, 0xa8, 0x00, 0x02, 0xc0, 0xa8, 0x00, 0x01, 0x08, 0x00, 0xfa, 0x61, 0x07, 0xef, 0x00, 0x01, 0xf1, 0x4f, 0x4e, 0x55, 0x00, 0x00, 0x00, 0x00, 0xee, 0x35, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
        */

    uint8_t arp_req[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x08, 0x06, 0x00, 0x01,
        0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xc0, 0xa8, 0x00, 0x02,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8, 0x00, 0x01
    };
    debug("enc28j60: init\n");
    enc28j60_init(mac_address);

    int last_seen_count = click_counter;
    uint8_t network_buf[256] = {0};
    int length;

    //debug("--> Ready to button click\n");
    debug("--> Starting loop...\n");
    for(;;)
    {
        HAL_Delay(1000);
        //if (last_seen_count < click_counter) {
            //debug("Sending arp...\n");
            //enc28j60_send_packet(arp_req, sizeof(arp_req));
            //HAL_Delay(400);
            length = enc28j60_recv_packet(network_buf, sizeof(network_buf));
            debug("Received response (len: %d):\n", length);
        //    for (int i = 0; i < 256; i += 16) {
        //        debug("\t%#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x\n",
        //                network_buf[i + 0], network_buf[i + 1], network_buf[i + 2], network_buf[i + 3],
        //                network_buf[i + 4], network_buf[i + 5], network_buf[i + 6], network_buf[i + 7],
        //                network_buf[i + 8], network_buf[i + 9], network_buf[i + 10], network_buf[i + 11],
        //                network_buf[i + 12], network_buf[i + 13], network_buf[i + 14], network_buf[i + 15]);
        //    }
        //    last_seen_count = click_counter;
        //}
        
        
    }

    return 0;
}
