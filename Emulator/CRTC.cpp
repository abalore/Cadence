#include "Headers/CRTC.h"
#include "Headers/CPC.h"
#include "Headers/Z80.h"

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

void CRTC::Init()
{
    HCC = 0;
    VCC = 0;
    HSC = 0;
    VSC = 0;
    MA = 0;
    RA = 0;
}

void CRTC::IOClock()
{
    if (!Z80::IORQ && (CPC::AddressBUS & 0x4000) == 0)
    {
        if (!(Z80::WR && Z80::RD))
        {
            switch((CPC::AddressBUS & 0x0300) >> 8)
            {
            case 0: // Index in
                Index = CPC::DataBUS;
                break;
            case 1: // Data in
                Registers[Index] = CPC::DataBUS;
                break;
            case 2: // Status out
                //////////////////////////////////////////
                break;
            case 3: // Data out
                //////////////////////////////////////////
                CPC::DataBUS = Registers[Index];
                break;
            }
        }
    }
}

void CRTC::CRTClock()
{
    HCC++;
    if (HCC < Registers[1])
        MA++;
    if (HCC > Registers[0])
    {
        HCC = 0;
        RA++;
        if (RA > Registers[9])
        {
            VCC++;
            if (VCC == Registers[7])
                VSC = 2;// (BYTE)(Registers[3] >> 4);
            RA = 0;
            if (VCC > Registers[4])
            {
                VCC = 0;
                MA = ((Registers[12] & 0x3F) << 8) + (Registers[13]);
            }
        }
        if (!(RA == 0 && VCC == 0))
            MA = (MA & 0x3000) + VCC * Registers[1];
        if (VSC > 0)
        {
            VSYNC = true;
            VSC--;
        }
        else
            VSYNC = false;
    }
    if (HCC == Registers[2])
        HSC = 4; //(BYTE)(Registers[3] & 0x0F);
    if (HSC > 0)
    {
        HSYNC = true;
        HSC--;
    }
    else
        HSYNC = false;
    BORDER = HCC > Registers[1] || VCC > Registers[6];
}

