#include "PPI.h"
#include "Z80.h"
#include "CPC.h"
#include "CRTC.h"
#include "PSG.h"
#include "Keyboard.h"
#include "Tape.h"

BYTE PPI::controlWord;
BYTE PPI::aMode;
BYTE PPI::bMode;
BYTE PPI::aHandshake;
BYTE PPI::bHandshake;
bool PPI::lCIO;
bool PPI::hCIO;
bool PPI::aIO;
bool PPI::bIO;
BYTE PPI::lC;
BYTE PPI::hC;

void PPI::Reset()
{
    controlWord = 0;
    aMode = 0;
    bMode = 0;
    aHandshake = 0;
    bHandshake = 0;
    lCIO = false;
    hCIO = false;
    aIO = false;
    bIO = false;
    lC = 0;
    hC = 0;
}

BYTE PPI::RD(BYTE reg)
{
    BYTE value = 0;
    switch(reg)
    {
    case 0: // 8255 PPI Port A (PSG Data)   (R/W)
        return aIO  || aMode == 2 ? PSG::ReadData() : 0x00;
    case 1: // 8255 PPI Port B (Vsync,PrnBusy,Tape In,etc.) (R)
        return bIO ? (Tape::GetLevel() << 7 ) + CRTC::VSYNC + 0x3E : 0x00;
    case 2: // 8255 PPI Port C (KeybRow,Tape Out,PSG Control) (W)
        if (!lCIO)
        {
            // bits 2..0
            if (bMode == 0)
                value = lC;
            else
                value = bHandshake;
            // bit 3
            if (aMode != 0)
            {
                value &= 0xF7;
                value |= aHandshake & 0x08;
            }
        }
        else
            value &= 0xF0;
        if (!hCIO)
        {
            switch(aMode)
            {
            case 0:
                value |= hC;
                break;
            case 1:
            case 2:
                value |= aHandshake & 0xF0;
                break;
            }
        }
        else
            value &= 0x0F;
        return value;
    case 3: // 8255 PPI Control-Register (W)
        return 0x00;
    }
    return value;
}

void PPI::WR(BYTE reg, BYTE value)
{
    switch(reg)
    {
    case 0: // 8255 PPI Port A (PSG Data)   (R/W)
        if (!aIO || aMode == 2)
            PSG::WriteData(value);
        break;
    case 1: // 8255 PPI Port B (Vsync,PrnBusy,Tape In,etc.) (R)
        if (!bIO)
        {

        }
        break;
    case 2: // 8255 PPI Port C (KeybRow,Tape Out,PSG Control) (W)
        if (!lCIO)
        {
            lC = value & 0x0F;
            ApplyLC();
        }
        if (!hCIO)
        {
            hC = value & 0xF0;
            ApplyHC();
        }
        break;
    case 3: // 8255 PPI Control-Register (W)
        if (value & 0x80)
        {
            controlWord = value;
            PSG::WriteData(0);
            Keyboard::SetRow(0);
            PSG::SelectFunction(false, false);
            Tape::SetMotorState(false);
            aMode = (controlWord & 0x7F) >> 5;
            bMode = (controlWord & 0x07) >> 2;
            lCIO = controlWord & 0x01;
            hCIO = controlWord & 0x08;
            aIO = controlWord & 0x10;
            bIO = controlWord & 0x02;
        }
        else
        {
            BYTE bit = (value & 0x0E) >> 1;
            bool setBit = (value & 0x01) != 0;
            if (bit < 4)
            {
                if (setBit) lC |= (1 << bit);
                else        lC &= ~(1 << bit);

                lC &= 0x0F;
                ApplyLC();
            }
            else
            {
                if (setBit) hC |= (1 << bit);
                else        hC &= ~(1 << bit);
                hC &= 0xF0;
                ApplyHC();
            }
        }
        break;
    }
}


void PPI::ApplyLC()
{
    if (aMode != 0)
    {
        aHandshake &= 0xF7;
        aHandshake |= lC & 0x08;
    }
    if (bMode == 0)
        Keyboard::SetRow(lC);
    else
        bHandshake = lC & 0x07;
}

void PPI::ApplyHC()
{
    switch(aMode)
    {
    case 0:
        PSG::SelectFunction((hC & 0x80) > 0, (hC & 0x40) > 0);
        Tape::SetMotorState(hC & 0x10);
        break;
    case 1:
    case 2:
        aHandshake |= hC;
        break;
    }
}
