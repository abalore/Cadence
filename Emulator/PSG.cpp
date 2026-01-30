#include "Headers/PSG.h"
#include "Headers/Keyboard.h"

void PSG::Init()
{
    BDIR = false;
    BC1 = true;
    counterA = 0;
    counterB = 0;
    counterC = 0;
    outputA = 0;
    outputB = 0;
    outputC = 0;
    divider = 0;
    bitA = false;
    bitB = false;
    bitC = false;
    periodA = 0;
    periodB = 0;
    periodC = 0;
    mixA = false;
    mixB = false;
    mixC = false;
}

void PSG::Clock()
{
    if (BDIR)
    {
        if (BC1)
            selectedRegister = inputRegister;
        else
        {
            registers[selectedRegister] = inputRegister;
            switch (selectedRegister)
            {
            case 0:
            case 1:
                periodA = ((registers[1] & 0x0F ) * 256 + registers[0]) >> 1;
                break;
            case 2:
            case 3:
                periodB = ((registers[3] & 0x0F ) * 256 + registers[2]) >> 1;
                break;
            case 4:
            case 5:
                periodC = ((registers[5] & 0x0F ) * 256 + registers[4]) >> 1;
                break;
            case 7:
                mixA = inputRegister & 0x01;
                mixB = inputRegister & 0x02;
                mixC = inputRegister & 0x04;
                break;
            }
        }
    }
    else
    {
        if (BC1)
        {
            if (selectedRegister < 0x0E)
            {
                outputRegister = registers[selectedRegister];
            }
            else if (selectedRegister == 0x0E)
                outputRegister = Keyboard::Read();
        }
        else
            outputRegister = 0xFF;
    }
    divider++;
    if (divider == 16)
    {
        divider = 0;
        if (mixA)
        {
            counterA++;
            if (counterA >= periodA)
            {
                bitA = !bitA;
                outputA = /*registers[8] * */bitA * 5;
            }
        }
        else outputA = 0;
        if (mixB)
        {
            counterB++;
            if (counterB >= periodB)
            {
                bitB = !bitB;
                outputB = /*registers[9] * */bitB * 5;
            }
        }
        else outputB = 0;
        if (mixC)
        {
            counterC++;
            if (counterC >= periodC)
            {
                bitC = !bitC;
                outputC = /*registers[10] * */bitC * 5;
            }
        }
        else outputC = 0;
        output = outputA + outputB + outputC;
        DataAvailable = true;
    }
}

BYTE PSG::ReadData()
{
    return outputRegister;
}

void PSG::WriteData(BYTE data)
{
    inputRegister = data;
}
