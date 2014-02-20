#pragma once

#include "sdk_Theme.h"

//////////////////////////////////////////////////////////////////////////////////////////////

class UtilGraphics
{
public:
	static void BitBltSimple(HDC theDest, HDC theSrc, int theX, int theY, const BitmapInfo & theInfo)
	{
		if ( theInfo.IsTransparent() )
		{
			// 1. Create a mask
			HBITMAP aMask = ::CreateBitmap(theInfo.myWidth, theInfo.myHeight, 1, 1, NULL);
			HDC aMaskDC = ::CreateCompatibleDC(theDest);
			HGDIOBJ aPrev_Mask = ::SelectObject(aMaskDC, aMask);

			// 2. Set the transparent color
			COLORREF aPrevClr = ::SetBkColor( theSrc, theInfo.myTransparentColor );

			// 3. Generate mask
			::BitBlt(aMaskDC, 0,    0,    theInfo.myWidth, theInfo.myHeight, theSrc,  0, 0, SRCCOPY);

			// 4. Draw image transparently
			::BitBlt(theDest, theX, theY, theInfo.myWidth, theInfo.myHeight, theSrc,  0, 0, SRCINVERT);
			::BitBlt(theDest, theX, theY, theInfo.myWidth, theInfo.myHeight, aMaskDC, 0, 0, SRCAND);
			::BitBlt(theDest, theX, theY, theInfo.myWidth, theInfo.myHeight, theSrc,  0, 0, SRCINVERT);


			// 5. Cleanup
			::SetBkColor(theSrc, aPrevClr);

			::SelectObject(aMaskDC, aPrev_Mask);
			::DeleteDC(aMaskDC);
			::DeleteObject(aMask);
		}
		else
		{
			::BitBlt(
				theDest,
				theX,
				theY,
				theInfo.myWidth,
				theInfo.myHeight,

				theSrc,
				0,
				0,
				SRCCOPY );
		}
	}

	static void BitBltSemiTransparent(HDC theDest, HDC theSrc, int theX, int theY, const BitmapInfo & theInfo, int theTransparency)
	{
		if ( theInfo.IsTransparent() )
		{
			HDC aTempDC = CreateCompatibleDC(theSrc);

			BITMAPINFO bmi;
			SecureZeroMemory(&bmi, sizeof BITMAPINFO);
			bmi.bmiHeader.biSize = sizeof BITMAPINFOHEADER;
			bmi.bmiHeader.biWidth = theInfo.myWidth;
			bmi.bmiHeader.biHeight = theInfo.myHeight;
			bmi.bmiHeader.biPlanes = 1;
			bmi.bmiHeader.biBitCount = 32;
			bmi.bmiHeader.biCompression = BI_RGB;
			bmi.bmiHeader.biSizeImage = bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight * 4;

			void * pvBits;
			HBITMAP hBmp = CreateDIBSection(aTempDC, &bmi, DIB_RGB_COLORS, &pvBits, NULL, 0);
			HGDIOBJ aPrev_Temp = SelectObject(aTempDC, hBmp);

			BitBlt(aTempDC, 0, 0, theInfo.myWidth, theInfo.myHeight, theSrc, 0, 0, SRCCOPY);
			GdiFlush();

			{
				RGBQUAD * aPixels = (RGBQUAD*) pvBits;
				size_t aSizeBytes = bmi.bmiHeader.biWidth * bmi.bmiHeader.biHeight;

				UINT aTrColor = theInfo.myTransparentColor & 0x00ffffff;

				for (size_t i = 0; i < aSizeBytes; i++)
				{
					UINT aSrcColor = *( (UINT *)&aPixels[i] ) & 0x00ffffff;

					if (aSrcColor != aTrColor)
					{
						// pre-multiply
						aPixels[i].rgbRed      = (BYTE) ((int) (aPixels[i].rgbRed   * theTransparency) / 0xff);
						aPixels[i].rgbGreen    = (BYTE) ((int) (aPixels[i].rgbGreen * theTransparency) / 0xff);
						aPixels[i].rgbBlue     = (BYTE) ((int) (aPixels[i].rgbBlue  * theTransparency) / 0xff);
						aPixels[i].rgbReserved = theTransparency;
					}
					else
					{
						aPixels[i].rgbRed = aPixels[i].rgbGreen =
						aPixels[i].rgbBlue = aPixels[i].rgbReserved = 0;
					}
				}
			}

			BLENDFUNCTION m_bf;
			m_bf.BlendOp = AC_SRC_OVER;
			m_bf.BlendFlags = 0;
			m_bf.SourceConstantAlpha = 255;
			m_bf.AlphaFormat = AC_SRC_ALPHA;

			AlphaBlend(theDest, theX, theY, theInfo.myWidth, theInfo.myHeight, aTempDC, 0, 0, theInfo.myWidth, theInfo.myHeight, m_bf);

			SelectObject(aTempDC, aPrev_Temp);
			DeleteObject(hBmp);
			DeleteDC(aTempDC);
		}
		else
		{
			BLENDFUNCTION m_bf;
			m_bf.BlendOp = AC_SRC_OVER;
			m_bf.BlendFlags = 0;
			m_bf.SourceConstantAlpha = theTransparency;
			m_bf.AlphaFormat = 0;

			AlphaBlend(theDest, theX, theY, theInfo.myWidth, theInfo.myHeight, theSrc, 0, 0, theInfo.myWidth, theInfo.myHeight, m_bf);
		}
	}
};