#ifndef PERIPHERAL_HPP
#define PERIPHERAL_HPP

#include "GPIO.hpp"

namespace Peripheral
{    
	class Screen
	{
    private:
        GPIO::GPIOPins * _RS;   // Registry Set ( 0 = Instruction Register (W) Busy Flag: address counter (R), 1 = Data Register (RW) )
        GPIO::GPIOPins * _RW;   // Read/Write ( 0 = W, 1 = R)
        GPIO::GPIOPins * _E;    // Clock, screen starts reading on falling edge

        GPIO::GPIOPins * _D4;
        GPIO::GPIOPins * _D5;
        GPIO::GPIOPins * _D6;
        GPIO::GPIOPins * _D7;

        uint8_t * _charList;

        void WaitForUnblock();
	public:
		void Initialize(GPIO::Pin RS = GPIO::Pin::P0, GPIO::Pin RW = GPIO::Pin::P2, GPIO::Pin E = GPIO::Pin::P4, GPIO::Pin D4 = GPIO::Pin::P1, GPIO::Pin D5 = GPIO::Pin::P3, GPIO::Pin D6 = GPIO::Pin::P5, GPIO::Pin D7 = GPIO::Pin::P7);

        void Write(bool RS, uint8_t data, bool checkForBlockFlag = true);
        void WriteHalf(bool RS, uint8_t data, bool checkForBlockFlag = true);

        void WriteString(const char* string);

        uint8_t Read(bool RS);

        void Clear();
        void Home();
        void SetDisplay(bool displayOn, bool cursorOn, bool cursorBlinkOn);
        void FunctionSet(bool lineCount, bool dotCount);
        void EntryMode(bool increment, bool displayShift);

        void SetCursorPosition(bool line, uint8_t pos);
	};
}

#endif // PERIPHERAL_HPP
