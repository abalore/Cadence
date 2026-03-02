#include "Headers/CRTC.h"
#include "Headers/CPC.h"
#include "Headers/Z80.h"
#include "Headers/GateArray.h"

BYTE CRTC::Registers[18];
BYTE CRTC::Index;
BYTE CRTC::RA;
word CRTC::MA;
bool CRTC::HSYNC;
bool CRTC::VSYNC;
bool CRTC::BORDER;
word CRTC::HCC;
word CRTC::VCC;
BYTE CRTC::HSC;
BYTE CRTC::VSC;
BYTE CRTC::R12;
BYTE CRTC::R13;
BYTE CRTC::verticalAdjust;

void CRTC::Init()
{
    HCC = 0;
    VCC = 0;
    HSC = 0;
    VSC = 0;
    MA = 0;
    RA = 0;
    HSYNC = false;
    VSYNC = false;
    verticalAdjust = 0;
}

void CRTC::RD()
{
    switch((Z80::AR & 0x0300) >> 8)
    {
    case 2: // Status out
        //////////////////////////////////////////
        break;
    case 3: // Data out
        Z80::DR = Registers[Index];
        break;
    }
}

void CRTC::WR()
{
    switch((Z80::AR & 0x0300) >> 8)
    {
    case 0: // Index in
        Index = Z80::DR;
        break;
    case 1: // Data in
        Registers[Index] = Z80::DR;
        break;
    }
}

void CRTC::Clock()
{
    HCC++;
    if (HCC == Registers[2])
    {
        HSC = ((BYTE)(Registers[3] & 0x0F));
        HSYNC = true;

    }
    if (HSC > 0)
    {
        HSC--;
        if (HSC == 0)
        {
            HSYNC = false;
        }
    }
    if (HCC > Registers[0])
    {
        HCC = 0;
        if (verticalAdjust > 0)
            verticalAdjust--;
        else
        {
            RA++;
            if (RA > Registers[9])
            {
                RA = 0;
                VCC++;
                if (VCC > Registers[4])
                {
                    VCC = 0;
                    R12 = Registers[12];
                    R13 = Registers[13];
                    verticalAdjust = Registers[5];
                }
                if (VCC == Registers[7])
                {
                    VSC = ((BYTE)(Registers[3] >> 4));
                    if (VSC == 0)
                        VSC = 16;
                    VSYNC = true;
                }
            }
        }
        //GateArray::GenerateVSync();
        if (VSC > 0)
        {
            VSC--;
            if (VSC == 0)
            {
                R12 = Registers[12];
                R13 = Registers[13];
                VSYNC = false;
            }
        }
        else if (VSYNC == true)
            VSYNC = false;
    }
    BORDER = HCC >= Registers[1] || VCC >= Registers[6] || verticalAdjust > 0;
    if (!BORDER)
        MA = ((R12 & 0x3F) << 8) + R13 + VCC * Registers[1] + HCC;
}

void CRTC::Update()
{
    //Update();
}

