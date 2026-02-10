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

}

void CRTC::Clock_IO_RD()
{
    switch((CPC::AddressBUS & 0x0300) >> 8)
    {
    case 2: // Status out
        //////////////////////////////////////////
        break;
    case 3: // Data out
        CPC::DataBUS = Registers[Index];
        break;
    }
}

void CRTC::Clock_IO_WR()
{
    switch((CPC::AddressBUS & 0x0300) >> 8)
    {
    case 0: // Index in
        Index = CPC::DataBUS;
        break;
    case 1: // Data in
        Registers[Index] = CPC::DataBUS;
        break;
    }
}

void CRTC::CheckHSync()
{
    if (HCC == Registers[2])
    {
        HSC = ((BYTE)(Registers[3] & 0x0F));
        HSYNC = true;
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
                // MA = ((Registers[12] & 0x3F) << 8) + (Registers[13]);
            }
            if (VCC == Registers[7])
            {
                VSC = ((BYTE)(Registers[3] >> 4));
                VSYNC = true;
            }
        }
        GateArray::GenerateVSync();
        if (VSC > 0)
        {
            VSC--;
            if (VSC == 0)
                VSYNC = false;
        }
    }
    if (HSC > 0)
    {
        HSC--;
        if (HSC == 0)
        {
            HSYNC = false;
        }
    }
}

void CRTC::DoVSync()
{

}


void CRTC::Clock()
{
    HCC++;
    CheckHSync();
    if (HCC > Registers[0])
        HCC = 0;
    BORDER = HCC >= Registers[1] || VCC >= Registers[6];
    MA = ((R12 & 0x3F) << 8) + R13 + VCC * Registers[1] + HCC;
}

