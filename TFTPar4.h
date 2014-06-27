#ifndef _TFTPAR4_H
#define _TFTPAR4_H

#include <TFT.h>

/*! The TFTPar4 class creates a new 4-bit interface compatible with popular text only LCD screens */
class TFTPar4 : public TFTCommunicator 
{
    public:
        /*! Create a new TFTPar4 object
         *  ===========================
         *  This creates a new 4-bit TFT interface, including the Data/Command (dc) pin, Clock (clk) (sometimes called E) and data bits d0 - d3.
         */
        TFTPar4(uint8_t dc, uint8_t clk, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
        /*! Create a new TFTPar4 object
         *  ===========================
         *  This creates a new 4-bit full TFT interface, including the Chip Select (cs) pin, Data/Command (dc) pin, Clock (clk) (sometimes called E) and data bits d0 - d3.
         */
        TFTPar4(uint8_t cs, uint8_t dc, uint8_t clk, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
        void writeCommand8(uint8_t command);
        void writeCommand16(uint16_t command);
        void writeCommand32(uint32_t command);
        void writeData8(uint8_t data);
        void writeData16(uint16_t data);
        void writeData32(uint32_t data);
        void streamStart();
        void streamEnd();
        void streamCommand8(uint8_t);
        void streamCommand16(uint16_t);
        void streamCommand32(uint32_t);
        void streamData8(uint8_t);
        void streamData16(uint16_t);
        void streamData32(uint32_t);

        uint8_t nativeWidth() { return 4; };

    private:
        uint8_t _cs;
        uint8_t _dc;
        uint8_t _clk;
        uint8_t _d0;
        uint8_t _d1;
        uint8_t _d2;
        uint8_t _d3;

        void clock();
        void setBus(uint8_t);
};

#endif

