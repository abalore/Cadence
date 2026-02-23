#include "Headers/PPI.h"
#include "Headers/Z80.h"
#include "Headers/CPC.h"
#include "Headers/CRTC.h"
#include "Headers/PSG.h"
#include "Headers/Keyboard.h"
#include "Headers/Tape.h"

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

void PPI::Init()
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

void PPI::RD()
{
    switch((Z80::AR & 0x0300) >> 8)
    {
    case 0: // 8255 PPI Port A (PSG Data)   (R/W)
        Z80::DR = aIO  || aMode == 2 ? PSG::ReadData() : 0x00;
        break;
    case 1: // 8255 PPI Port B (Vsync,PrnBusy,Tape In,etc.) (R)
        Z80::DR = bIO ? (Tape::GetLevel() << 7 ) + CRTC::VSYNC + 0x3E : 0x00;
        break;
    case 2: // 8255 PPI Port C (KeybRow,Tape Out,PSG Control) (W)
        if (!lCIO)
        {
            // bits 2..0
            if (bMode == 0)
                Z80::DR = lC;
            else
                Z80::DR = bHandshake;
            // bit 3
            if (aMode != 0)
            {
                Z80::DR &= 0xF7;
                Z80::DR |= aHandshake & 0x08;
            }
        }
        else
            Z80::DR &= 0xF0;
        if (!hCIO)
        {
            switch(aMode)
            {
            case 0:
                Z80::DR |= hC;
                break;
            case 1:
            case 2:
                Z80::DR |= aHandshake & 0xF0;
                break;
            }
        }
        else
            Z80::DR &= 0x0F;

        break;
    case 3: // 8255 PPI Control-Register (W)
        Z80::DR = 0x00;
        break;
    }
}

void PPI::WR()
{
    switch((Z80::AR & 0x0300) >> 8)
    {
    case 0: // 8255 PPI Port A (PSG Data)   (R/W)
        if (!aIO || aMode == 2)
            PSG::WriteData(Z80::DR);
        break;
    case 1: // 8255 PPI Port B (Vsync,PrnBusy,Tape In,etc.) (R)
        if (!bIO)
        {

        }
        break;
    case 2: // 8255 PPI Port C (KeybRow,Tape Out,PSG Control) (W)
        if (!lCIO)
        {
            lC = Z80::DR & 0x0F;
            ApplyLC();
        }
        if (!hCIO)
        {
            hC = Z80::DR & 0xF0;
            ApplyHC();
        }
        break;
    case 3: // 8255 PPI Control-Register (W)
        if (Z80::DR & 0x80)
        {
            controlWord = Z80::DR;
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
            BYTE bit = (Z80::DR & 0x0E) >> 1;
            if (bit < 4)
            {
                BYTE value = 1 << bit;
                lC &= value ^0x0F;
                lC |= value;
                ApplyLC();
            }
            else
            {
                BYTE value = (Z80::DR & 0x01) << bit;
                hC &= value ^0x0F;
                hC |= value;
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
