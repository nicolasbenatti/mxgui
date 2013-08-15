/***************************************************************************
 *   Copyright (C) 2013 by Terraneo Federico                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#ifdef _BOARD_SONY_NEWMAN

#include "display_sony-newman.h"

using namespace std;
using namespace miosix;

namespace mxgui {

//
// Class DisplayImpl
//

DisplayImpl::DisplayImpl(): textColor(), font(droid11)
{
    turnOn();
    setTextColor(Color(0xffff),Color(0x0000));
}

void DisplayImpl::clear(Point p1, Point p2, Color color)
{
    imageWindow(p1,p2);
    SPITransaction t;
    writeRamBegin();
    int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    for(int i=0;i<numPixels;i++) writeRam(color);
    writeRamEnd();
}

void DisplayImpl::drawRectangle(Point a, Point b, Color c)
{
    line(a,Point(b.x(),a.y()),c);
    line(Point(b.x(),a.y()),b,c);
    line(b,Point(a.x(),b.y()),c);
    line(Point(a.x(),b.y()),a,c);
}

void DisplayImpl::turnOn()
{
    power::ENABLE_2V8_Pin::high();
    oled::OLED_nSS_Pin::high();
    oled::OLED_A0_Pin::high();
    
    {
        FastInterruptDisableLock dLock;
        //Gpios are already configured in the BSP
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
        SPI1->CR2=0;
        SPI1->CR1=SPI_CR1_SSM  //Software cs
                | SPI_CR1_SSI  //Hardware cs internally tied high
                | SPI_CR1_MSTR //Master mode
                | SPI_CR1_SPE; //SPI enabled
    }
    
    /*
     * Initialization sequence taken from underverk's
     * SmartWatch_Toolchain/src/driver_display.c. Since there's no publicly
     * available LD7131 datasheet there's no other choice.
     */
    oled::OLED_Reset_Pin::low();
    Thread::sleep(10);
    oled::OLED_Reset_Pin::high();
    Thread::sleep(10);
    writeReg(0x01); //SW Reset
    Thread::sleep(10);
    
    /*
     * The way the mxgui library works depends heavily on the possibility to
     * efficiently implement the imageWindow() and textWindow() primitives.
     * However, the information available from underverk's code is not enough,
     * as it uses a fixed window setting which is neither of the two that we
     * need. Luckily, he at least left comments in the code, and in particular
     * the 0x05 command was labeled as "RAM write direction". By trial and error
     * it was possible to reverse-engineer the missing piece.
     * The 0x05 register takes a single byte value, whose meaning is:
     * bit 7      0
     *     ????wdss
     *     ||||||++-- Start point: 00 cursor is at top left after window command
     *     ||||||                  01 cursor is at top right
     *     ||||||                  10 cursor is at bottom left
     *     ||||||                  11 cursor in at bottom right
     *     |||||+---- Direction:    0 first move horizontally, then vertically
     *     |||||                    1 first move vertically, then horizontally
     *     ||||+----- Word order: it takes two byte to transfer a pixel through
     *     ||||       SPI, and this bit allows to swap them.
     *     ++++------ Didn't try to mess with those, thei meaning remains unknown
     */
    //0x0c: starts from top left, goes down, then right <- this is DR
    //0x04: starts from top left, goes down, then right (16 bit are swapped)
    //0x08: starts from top left, goes right, then down <- this is RD
    //0x09: starts from top right, goes left, then down
    //0x0a: starts from bottom left, goes right, the up
    //0x0b: starts from bottom right, goes left, then up
    
    //Initialization command list
    const unsigned char initCmds[]=
    {
        0x03, 0x01, 0x00, //Stand-by OFF
        0x07, 0x08, 0x00, 0x00, 0x07, 0x0F, 0x01, 0x0C, 0x09, 0x0C, //Display size
        0x05, 0x01, 0x0A, //Set RAM write "direction"
        0x06, 0x01, 0x01, //Set RAM scanning "direction"
        0x04, 0x01, 0x02, //Set internal oscillator to 90Hz
        0x1F, 0x01, 0x30, //Set row scan operations = ALL
        0x30, 0x01, 0x12, //Set regulator to ???
        0x1C, 0x01, 0x0F, //Set pre-charge width
        0x1E, 0x01, 0x00, //Set peak pulse delay
        0x0F, 0x03, 0x0A, 0x0A, 0x0A, //Set peak currents (RGB)
        0x1D, 0x03, 0x05, 0x05, 0x05, //Set peak pulse widths (RGB)
        0x08, 0x01, 0x01, //Set interface type to 8-bit
        0x00 //END
    };
    
    //Send initializiation commands
    const unsigned char *cmds=initCmds;
    while(*cmds)
    {
        unsigned char cmd=*cmds++;
        unsigned char numArgs=*cmds++;
        writeReg(cmd,cmds,numArgs);
        cmds+=numArgs;
    }
    
    setBrightness(40);
    
    clear(black);
    
    //Turn on display
    writeReg(0x02,0x01);
    
    /*
     * underverk's comment says oled::OLED_V_ENABLE_Pin drives a 1.8V rail. This
     * is probably wrong, as it most likely drives the 16V rail that powers the
     * OLED panel. Also, I'm not very confident with this code, as the lack of
     * explicit delays makes the loop timings depend on CPU speed.
     */
    
    //Ramps up the 1.8V (directly on = unstable behaviour)
    const int count=2000; //Sony says minimum 500
    for(int outer=0;outer<count;outer++)
    {
        oled::OLED_V_ENABLE_Pin::high();
        for(int inner=0;inner<count;inner++)
            if(inner==outer) oled::OLED_V_ENABLE_Pin::low();
    }
    oled::OLED_V_ENABLE_Pin::high();
}

void DisplayImpl::turnOff()
{
    setBrightness(0);
    oled::OLED_V_ENABLE_Pin::low();
    power::ENABLE_2V8_Pin::low();
    oled::OLED_Reset_Pin::low();
    oled::OLED_A0_Pin::low();
}

void DisplayImpl::setBrightness(unsigned char brt)
{
    //Taken from underverk's SmartWatch_Toolchain/src/driver_display.c
    unsigned char buffer[6];
    brt=min<unsigned char>(brt,90);
    buffer[0]=buffer[2]=buffer[4]=brt>>4;
    buffer[1]=buffer[3]=buffer[5]=brt & 15;
    writeReg(0x0e,buffer,sizeof(buffer));
}

DisplayImpl::pixel_iterator DisplayImpl::begin(Point p1,
        Point p2, IteratorDirection d)
{
    if(p1.x()<0 || p1.y()<0 || p2.x()<0 || p2.y()<0) return pixel_iterator();
    if(p1.x()>=width || p1.y()>=height || p2.x()>=width || p2.y()>=height)
        return pixel_iterator();
    if(p2.x()<p1.x() || p2.y()<p1.y()) return pixel_iterator();

    if(d==DR) textWindow(p1,p2);
    else imageWindow(p1,p2);
    
    miosix::oled::OLED_nSS_Pin::low(); //Will be deasserted by the iterator
    writeRamBegin();
    
    unsigned int numPixels=(p2.x()-p1.x()+1)*(p2.y()-p1.y()+1);
    return pixel_iterator(numPixels);
}

void DisplayImpl::window(Point p1, Point p2)
{
    //Taken from underverk's SmartWatch_Toolchain/src/driver_display.c
    unsigned char buffer[8];
    unsigned char y0=p1.y()+0x1c;
    unsigned char y1=p2.y()+0x1c;
    buffer[0]=p1.x()>>4; buffer[1]=p1.x() & 15;
    buffer[2]=p2.x()>>4; buffer[3]=p2.x() & 15;
    buffer[4]=y0>>4;     buffer[5]=y0 & 15;
    buffer[6]=y1>>4;     buffer[7]=y1 & 15;
    writeReg(0x0a,buffer,sizeof(buffer));
}

void DisplayImpl::writeReg(unsigned char reg, unsigned char data)
{
    SPITransaction t;
    {
        CommandTransaction c;
        writeRam(reg);
    }
    writeRam(data);
}

void DisplayImpl::writeReg(unsigned char reg, const unsigned char *data, int len)
{
    SPITransaction t;
    {
        CommandTransaction c;
        writeRam(reg);
    }
    if(data) for(int i=0;i<len;i++) writeRam(*data++);
}

} //namespace mxgui

#endif //_BOARD_SONY_NEWMAN