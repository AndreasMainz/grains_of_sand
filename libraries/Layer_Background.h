/*
 * SmartMatrix Library - Background Layer Class
 *
 * Copyright (c) 2015 Louis Beaudoin (Pixelmatix)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _LAYER_BACKGROUND_H_
#define _LAYER_BACKGROUND_H_

#include "Layer.h"
#include "MatrixCommon.h"
#include "MatrixFontCommon.h"

#define SM_BACKGROUND_OPTIONS_NONE     0

template <typename RGB, unsigned int optionFlags>
class SMLayerBackground : public SM_Layer {
    public:
        SMLayerBackground(RGB * buffer, uint16_t width, uint16_t height);
        SMLayerBackground(uint16_t width, uint16_t height);
        void begin(void);
        void frameRefreshCallback();
        void fillRefreshRow(uint16_t hardwareY, rgb48 refreshRow[]);
        void fillRefreshRow(uint16_t hardwareY, rgb24 refreshRow[]);

        void swapBuffers(bool copy = true);
        bool isSwapPending();
        void copyRefreshToDrawing(void);
        void drawPixel(int16_t x, int16_t y, const RGB& color);
		void drawPixel(int16_t x, int16_t y, int16_t x0, int16_t y0, const RGB& color);
        void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const RGB& color);
		void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x0, int16_t y0, const RGB& color);
        void drawFastVLine(int16_t x, int16_t y0, int16_t y1, const RGB& color);
        void drawFastHLine(int16_t x0, int16_t x1, int16_t y, const RGB& color);
        void drawCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& color);
        void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& outlineColor, const RGB& fillColor);
        void fillCircle(int16_t x0, int16_t y0, uint16_t radius, const RGB& color);
        void drawEllipse(int16_t x0, int16_t y0, uint16_t radiusX, uint16_t radiusY, const RGB& color);
        void drawTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& color);
        void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& fillColor);
        void fillTriangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& outlineColor, const RGB& fillColor);
        void drawRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
        void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, const RGB& color);
		void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& color);
        void fillRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, const RGB& outlineColor, const RGB& fillColor);
        void drawRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const RGB& outlineColor);
        void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const RGB& fillColor);
        void fillRoundRectangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t radius, const RGB& outlineColor, const RGB& fillColor);
        void fillScreen(const RGB& color);
     // void drawChar(int16_t x, int16_t y, const RGB& charColor, char character);
		void drawChar(int16_t x, int16_t y, const RGB& charColor, const char character, int xScale, int yScale);
		
      //void drawString(int16_t x, int16_t y, const RGB& charColor, const char text[]);
		void drawString(int16_t x, int16_t y, const RGB& charColor, char text[], int xScale, int yScale);
      //void drawString(int16_t x, int16_t y, const RGB& charColor, const RGB& backColor, const char text[]);
		void drawString(int16_t x, int16_t y, const RGB& charColor, const RGB& backColor, const char text[], int xScale, int yScale);
		//////////////////////////////////////////////////
		//////////////  Extensions for morphing clock ////
		void Digit0(void);
		void Digit1(void);
		void Digit2(void);
		void Digit3(void);
		void Digit4(void);
		void Digit5(void);
		void drawSeg(int16_t seg);
		void drawNumber(int16_t value);
		void morphNumber(int16_t newValue, uint8_t cnt);
		void drawColon(const RGB& color);
		/////////////////////////////////////////////////
        void drawMonoBitmap(int16_t x, int16_t y, uint8_t width, uint8_t height, const RGB& bitmapColor, const uint8_t *bitmap);

        // reads pixel from drawing buffer, not refresh buffer
        const RGB readPixel(int16_t x, int16_t y);

        RGB *backBuffer(void);
        void setBackBuffer(RGB *newBuffer);

        RGB *getRealBackBuffer();

        void setFont(fontChoices newFont);
        void setBrightness(uint8_t brightness);
        void enableColorCorrection(bool enabled);
		uint16_t MorphingStarted;
		uint8_t _value;

    private:
        bool ccEnabled = sizeof(RGB) <= 3 ? true : false;

        RGB *currentDrawBufferPtr;
        RGB *currentRefreshBufferPtr;

        RGB *backgroundBuffers[2];

        RGB *getCurrentRefreshRow(uint16_t y);

        void loadPixelToDrawBuffer(int16_t hwx, int16_t hwy, const RGB& color);
        const RGB readPixelFromDrawBuffer(int16_t hwx, int16_t hwy);
        void getBackgroundRefreshPixel(uint16_t x, uint16_t y, RGB &refreshPixel);
        bool getForegroundRefreshPixel(uint16_t x, uint16_t y, RGB &xyPixel);

        // drawing functions not meant for user
        void drawHardwareHLine(uint16_t x0, uint16_t x1, uint16_t y, const RGB& color);
        void drawHardwareVLine(uint16_t x, uint16_t y0, uint16_t y1, const RGB& color);
        void bresteepline(int16_t x3, int16_t y3, int16_t x4, int16_t y4, const RGB& color);
        void fillFlatSideTriangleInt(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3, const RGB& color);
        // todo: move somewhere else
        static bool getBitmapPixelAtXY(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);

        uint8_t backgroundBrightness = 255;
        color_chan_t backgroundColorCorrectionLUT[256];
        bitmap_font *font;
		
		/////////////////////////////////////////////////////
		void Morph2(uint8_t cnt);
		void Morph3(uint8_t cnt);
		void Morph4(uint8_t cnt);
		void Morph5(uint8_t cnt);
		void Morph6(uint8_t cnt);
		void Morph7(uint8_t cnt);
		void Morph8(uint8_t cnt);
		void Morph9(uint8_t cnt);
		void Morph0(uint8_t cnt);
		void Morph1(uint8_t cnt);
		void Digit(uint8_t value, uint16_t xo, uint16_t yo, const RGB& color);
        uint16_t xOffset;
        uint16_t yOffset;
		rgb24 _color;
		uint32_t Morph_clock;
		uint16_t Morphing_started;

		////////////////////////////////////////////////////

        // keeping track of drawing buffers
        volatile unsigned char currentDrawBuffer;
        volatile unsigned char currentRefreshBuffer;
        volatile bool swapPending;
        void handleBufferSwap(void);
};

#include "Layer_Background_Impl.h"

#endif