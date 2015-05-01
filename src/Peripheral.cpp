#include "Peripheral.hpp"

//GPIO
#include "GPIO.hpp"

namespace Peripheral
{
	void Screen::Initialize(GPIO::Pin RS, GPIO::Pin RW, GPIO::Pin E, GPIO::Pin D4, GPIO::Pin D5, GPIO::Pin D6, GPIO::Pin D7)
	{
		// GPIO Pins Initialization
        _RS = new GPIO::GPIOPins(GPIOA, {RS}, GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low);
        _RW = new GPIO::GPIOPins(GPIOA, {RW}, GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low);
        _E  = new GPIO::GPIOPins(GPIOA, {E},  GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low);
        _D4 = new GPIO::GPIOPins(GPIOA, {D4}, GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low);
        _D5 = new GPIO::GPIOPins(GPIOA, {D5}, GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low);
        _D6 = new GPIO::GPIOPins(GPIOA, {D6}, GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low);
        _D7 = new GPIO::GPIOPins(GPIOA, {D7}, GPIO::Mode::OutputPushPull, GPIO::Pull::NoPull, GPIO::Speed::Low);

        HAL_Delay(100);

        /** Special initialization **/

        this->WriteHalf(0, 0b00110000, false); // ????
        HAL_Delay(5);

        this->WriteHalf(0, 0b00110000, false); // ????
        HAL_Delay(1);

        this->WriteHalf(0, 0b00110000, false); // ????
        HAL_Delay(1);

        this->WriteHalf(0, 0b00100000, false); // Set 4 bit mode
        HAL_Delay(1);

        /** End special initialization **/

        /** Personalization **/

        this->FunctionSet(1,1);
        this->SetDisplay(1,0,0);
        this->Clear();
        this->EntryMode(1,0);
        this->SetDisplay(1,1,1);

        /** End personalization **/
    }

    void Screen::WaitForUnblock()
    {
        while(this->Read(0) & 0b10000000) { }
    }

    void Screen::Write(bool RS, uint8_t data, bool checkForBlockFlag)
    {
        WriteHalf(RS, data, checkForBlockFlag);

        _E->set_state(true);

        _D4->set_state(data & 0b00000001); //1
        _D5->set_state(data & 0b00000010); //2
        _D6->set_state(data & 0b00000100); //4
        _D7->set_state(data & 0b00001000); //8

        _E->set_state(false);
    }

    void Screen::WriteHalf(bool RS, uint8_t data, bool checkForBlockFlag)
    {
        if(checkForBlockFlag)
            WaitForUnblock();

        _E->set_state(true);

        _RS->set_state(RS);
        _RW->set_state(false); //Write mode

        _D4->set_state(data & 0b00010000); //16
        _D5->set_state(data & 0b00100000); //32
        _D6->set_state(data & 0b01000000); //64
        _D7->set_state(data & 0b10000000); //128

        _E->set_state(false);
    }

    void Screen::WriteString(const char* string)
    {
        for(int i=0; string[i] != '\0'; i++)
        {
            this->Write(1, string[i]);
            HAL_Delay(1);
        }
    }

    uint8_t Screen::Read(bool RS)
    {
        uint8_t result = 0;

        _RS->set_state(RS);
        _RW->set_state(true); //Read mode

        _D4->set_state(false);
        _D5->set_state(false);
        _D6->set_state(false);
        _D7->set_state(false);

        _E->set_state(false);

        result |= (_D4->get_state() * 0b1000);
        result |= (_D5->get_state() * 0b1000);
        result |= (_D6->get_state() * 0b1000);
        result |= (_D7->get_state() * 0b1000);

        // Wait ?
        HAL_Delay(1);

        _E->set_state(true);

        _E->set_state(false);

        result |= _D4->get_state();
        result |= _D5->get_state();
        result |= _D6->get_state();
        result |= _D7->get_state();

        return result;
    }

    void Screen::Clear()
    {
        Screen::Write(0, 0b00000001);
    }

    void Screen::Home()
    {
        Screen::Write(0, 0b00000010);
    }

    void Screen::SetDisplay(bool displayOn, bool cursorOn, bool cursorBlinkOn)
    {
        //0b00001DCB
        uint8_t data = 0b1000 + (displayOn * 0b100) + (cursorOn * 0b10) + cursorBlinkOn;
        this->Write(0, data);
    }

    void Screen::FunctionSet(bool lineCount, bool dotCount)
    {
        //0b0010NF00
        uint8_t data = 0b100000 + (lineCount * 0b1000) + (dotCount * 0b100);
        this->Write(0, data);
    }

    void Screen::EntryMode(bool increment, bool displayShift)
    {
        //0b000001(I/D)S
        uint8_t data = 0b100 + (increment * 0b10) + displayShift;
        this->Write(0, data);
    }

    void Screen::SetCursorPosition(bool line, uint8_t pos)
    {
        if(pos > 0b111111) return;
        this->Write(0, 0b10000000 + (line * 0b1000000) + pos);
    }
}
