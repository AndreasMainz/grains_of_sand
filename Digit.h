#ifndef DIGIT_H
#define DIGIT_H

class Digit {
  
  public:
    Digit(uint8_t value, uint16_t xo, uint16_t yo, uint8_t Rcolor, uint8_t Gcolor, uint8_t Bcolor);
    void    Draw(byte value);
    void    Morph(byte newValue);
    uint8_t Value();
    void    DrawColon(uint16_t c);
    
  private:
    uint8_t _value;
    uint8_t _Rcolor;
    uint8_t _Gcolor;
    uint8_t _Bcolor;
    uint16_t xOffset;
    uint16_t yOffset;

    void drawPixel(uint16_t x, uint16_t y, uint16_t c);
    void drawFillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c);
    void drawLine(uint16_t x, uint16_t y, uint16_t x2, uint16_t y2, uint16_t c);
    void drawSeg(byte seg);
    void Morph2();
    void Morph3();
    void Morph4();
    void Morph5();
    void Morph6();
    void Morph7();
    void Morph8();
    void Morph9();
    void Morph0();
    void Morph1();
};

#endif
