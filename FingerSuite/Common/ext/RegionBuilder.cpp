//////////////////////////////////////////////////////////////////////////
// CRegionBuilder - fast HRGN creation for WinCE/PocketPC/Win32
// coded by dzolee - http://dzolee.blogspot.com
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "RegionBuilder.h"

//////////////////////////////////////////////////////////////////////////
CRegionBuilder::CRegionBuilder()
{
}

//////////////////////////////////////////////////////////////////////////
CRegionBuilder::~CRegionBuilder()
{
}

//////////////////////////////////////////////////////////////////////////
RegionBuilderError CRegionBuilder::BuildRegion(HBITMAP hBmp, HRGN *pDest)
{
register LONG x, y, lW;
register DWORD *pCurrentDW, dwCurrent, nCurrentBit;
RegionBuilderError retval;
BITMAP bmp;
BITMAPINFO *pBmpInfo;
void *pBits;
HBITMAP hDIB;
LONG lH, nTotalDWsPerRow, nUsedBits, nUsedRects, nCurrentRect, start_x;
HDC dcDisplay, dcSrc, dcDest;
HGDIOBJ hSrcOldObj, hDestOldObj;
DWORD dwTemp;
bool bNotTransparent;
HRGN hRgn;
RGNDATA *pRgnData;
RECT *pRect;

	//zero result
	*pDest=NULL;

	//get bitmap dimensions
	if(GetObject(hBmp, sizeof(BITMAP), &bmp) == 0)
	{
		return rbeGDIError;
	}

	lW=bmp.bmWidth;
	lH=bmp.bmHeight;

	//alloc buffer for bmp header plus 2 colors (black and white)
	pBmpInfo=(BITMAPINFO *)malloc(sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));

	//if failed to alloc...
	if(pBmpInfo == NULL)
	{
		return rbeNoMem;
	}

	//size of structure included in structure
	pBmpInfo->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	//copy dimensions
	pBmpInfo->bmiHeader.biWidth=lW;
	pBmpInfo->bmiHeader.biHeight=lH;
	//always one plane
	pBmpInfo->bmiHeader.biPlanes=1;
	//monochrome
	pBmpInfo->bmiHeader.biBitCount=1;
	//colors stored as RGBQUADs
	pBmpInfo->bmiHeader.biCompression=BI_RGB;
	//number of bytes. each scanline is padded to a multiple of 4 -- courtesy of Lucian Wischik
	pBmpInfo->bmiHeader.biSizeImage = (DWORD)(((lW+7) & 0xfffffff8) * lH/8);
	//dummy values
	pBmpInfo->bmiHeader.biXPelsPerMeter=1000000;
	pBmpInfo->bmiHeader.biYPelsPerMeter=1000000;
	//2 colors used: black and white
	pBmpInfo->bmiHeader.biClrUsed=2;
	pBmpInfo->bmiHeader.biClrImportant=2;
	//now the RGBQUAD table: first entry is "black", second entry is "white".
	pBmpInfo->bmiColors[0].rgbBlue=pBmpInfo->bmiColors[0].rgbGreen=pBmpInfo->bmiColors[0].rgbRed=pBmpInfo->bmiColors[0].rgbReserved=0;
	pBmpInfo->bmiColors[1].rgbBlue=pBmpInfo->bmiColors[1].rgbGreen=pBmpInfo->bmiColors[1].rgbRed=pBmpInfo->bmiColors[1].rgbReserved=255;
	//create bitmap	
	hDIB=CreateDIBSection(NULL, pBmpInfo, DIB_RGB_COLORS, &pBits, NULL, 0);
	//free unused mem
	free(pBmpInfo);

	if(hDIB == NULL)
	{
		//error
		return rbeGDIError;
	}

	//get a DC to the display
	dcDisplay=::GetDC(NULL);
	if(dcDisplay == NULL)
	{
		//failed	
		DeleteObject(hDIB);
		return rbeGDIError;
	}
	
	//create compatible DCs for source (input) and destination (monochrome) bitmap
	dcSrc=CreateCompatibleDC(dcDisplay);
	//failed to create DC?
	if(dcSrc == NULL)
	{
		//clean up
		DeleteObject(hDIB);
		::ReleaseDC(NULL, dcDisplay);
		return rbeGDIError;
	}

	dcDest=CreateCompatibleDC(dcDisplay);
	//release display DC as soon as it is not needed anymore
	::ReleaseDC(NULL, dcDisplay);

	if(dcDest == NULL)
	{
		DeleteDC(dcSrc);
		DeleteObject(hDIB);
		return rbeGDIError;
	}

	//select source and destination bitmaps into DCs
	hSrcOldObj=SelectObject(dcSrc, hBmp);
	hDestOldObj=SelectObject(dcDest, hDIB);

	retval=rbeGDIError;

	//copy source bitmap to destination (monochrome)
	if(TRUE == ::BitBlt(dcDest, 0, 0, lW, lH, dcSrc, 0, 0, SRCCOPY))
	{
		//copy ok - now we have bits at pBits

		//first calculate the number of RECTs required

		//calculate DWORDs per row
		if(lW > 32)
		{
			nTotalDWsPerRow=lW / 32;
			//if number of pixels does not exactly fit into DWORD boundary
			if(lW % 32 != 0)
			{
				//one more DWORD needed to store row			
				nTotalDWsPerRow++;
			}
		}
		else
		{
			//simple case, DWORD count is 1
			nTotalDWsPerRow=1;
		}

		//for every row (backwards scan because image is flipped)
		nUsedBits=nUsedRects=0;
		bNotTransparent=false;
		for(y=lH-1; y>=0; y--)
		{
			//pointer to first DWORD in row
			pCurrentDW=((DWORD *)pBits + y * nTotalDWsPerRow);
			nCurrentBit=0;
			start_x=0;
			for(x=0; x<lW; x++)
			{
				if(nCurrentBit == 0)
				{
					//we can access the buffer by DWORDs but the result needs to be bswapped
					dwCurrent=ByteSwap(*pCurrentDW);
				}

				if(dwCurrent & 0x80000000)
				{
					//bit 1
					nUsedBits++;
					if(bNotTransparent == false)
					{
						//start line
						bNotTransparent=true;
						start_x=x;
					}
				}
				else
				{
					if(bNotTransparent == true)
					{
						//if was in a line, end line
						bNotTransparent=false;
						nUsedRects++;
					}
				}

				dwCurrent<<=1;
				//next bit
				nCurrentBit++;
				if(nCurrentBit == 32)
				{
					nCurrentBit=0;
					pCurrentDW++;
				}

				//end of row reached?
				if(x == lW-1)
				{
					if(bNotTransparent == true)
					{
						//finish line
						bNotTransparent=false;
						nUsedRects++;
					}
				}

			}//for x
		}//for y

		//alloc mem for region data
		dwTemp=sizeof(RGNDATAHEADER) + nUsedRects * sizeof(RECT);
		pRgnData=(RGNDATA *)malloc(dwTemp);		
		if(pRgnData == NULL)
		{
			//clean up and exit
			SelectObject(dcSrc, hSrcOldObj);
			SelectObject(dcDest, hDestOldObj);
			DeleteObject(hDIB);
			DeleteDC(dcSrc);
			DeleteDC(dcDest);
			return rbeGDIError;
		}

		//now build array of RECTs for region

		bNotTransparent=false;
		nCurrentRect=0;

		for(y=lH-1; y>=0; y--)
		{
			//pointer to first DWORD in row
			pCurrentDW=((DWORD *)pBits + y * nTotalDWsPerRow);
			nCurrentBit=0;
			start_x=0;
			for(x=0; x<lW; x++)
			{
				if(nCurrentBit == 0)
				{
					//we can access the buffer by DWORDs but the result needs to be bswapped
					dwCurrent=ByteSwap(*pCurrentDW);
				}

				if(dwCurrent & 0x80000000)
				{
					//bit 1
					nUsedBits++;
					if(bNotTransparent == false)
					{
						bNotTransparent=true;
						start_x=x;
					}
				}
				else
				{
					if(bNotTransparent == true)
					{
						bNotTransparent=false;
						pRect=&(((RECT *)&pRgnData->Buffer)[nCurrentRect]);
						pRect->left=start_x;
						pRect->right=x;
						pRect->top=lH-y-1;
						pRect->bottom=lH-y;
						nCurrentRect++;
					}
				}

				dwCurrent<<=1;
				//next bit
				nCurrentBit++;
				if(nCurrentBit == 32)
				{
					nCurrentBit=0;
					pCurrentDW++;
				}

				//end of row reached?
				if(x == lW-1)
				{
					if(bNotTransparent == true)
					{
						//finish line
						bNotTransparent=false;
						pRect=&(((RECT *)&pRgnData->Buffer)[nCurrentRect]);
						pRect->left=start_x;
						pRect->right=x;
						pRect->top=lH-y-1;
						pRect->bottom=lH-y;
						nCurrentRect++;
					}
				}
			}//for x
		}//for y

		//last step: region creation

		//build region data header
		pRgnData->rdh.dwSize=sizeof(RGNDATAHEADER);
		pRgnData->rdh.iType=RDH_RECTANGLES;
		pRgnData->rdh.nCount=nCurrentRect;
		pRgnData->rdh.nRgnSize=nCurrentRect * sizeof(RECT);
		pRgnData->rdh.rcBound.left=pRgnData->rdh.rcBound.top=0;
		pRgnData->rdh.rcBound.right=lW;
		pRgnData->rdh.rcBound.bottom=lH;

		//create region
		hRgn=ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + nUsedRects * sizeof(RECT), pRgnData);

		//free RECT data
		free(pRgnData);

		if(hRgn != NULL)
		{
			//clean up and return success
			*pDest=hRgn;
			retval=rbeOK;
		}
	}//if BitBlt succeeded

	//clean up and exit
	SelectObject(dcSrc, hSrcOldObj);
	SelectObject(dcDest, hDestOldObj);
	DeleteObject(hDIB);
	DeleteDC(dcSrc);
	DeleteDC(dcDest);
	return retval;
}

//////////////////////////////////////////////////////////////////////////
//code from stdlib
inline DWORD CRegionBuilder::ByteSwap(DWORD dwIn)
{
DWORD j;

	j =  (dwIn << 24);
	j += (dwIn <<  8) & 0x00ff0000;
	j += (dwIn >>  8) & 0x0000ff00;
	j += (dwIn >> 24);

	return j;
}
