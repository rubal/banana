// CONLIB.c : imlementation of console helper functions for Advanced Programming Workshop
//
// This is a part of the Message-Driven Enviroment C library.
// Copyright (C) 1999 Denis V. Kosykh
// All rights reserved.

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#ifdef __GNUC__

#include <windows.h>

#include <mingw32/conio.h>

#else	// default: MSVC

#include <wtypes.h>
#include <wincon.h>
#include <conio.h>

#endif // __GNUC__

#include "../console.h"

ConsoleColors::ConsoleColors(int bits)
{
	this->bits = bits;
}

ConsoleColors::ConsoleColors(ConsoleColor foreground,
							 ConsoleColor background)
{
	this->bits = ((background & 0xF) << 4) | ((foreground & 0x0F) << 0);
}

int Console::GotoXY(int x, int y)
{
	COORD pos = { (short)x, (short)y };

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	return SetConsoleCursorPosition(hStdOut, pos);
}

int Console::GetXY(int *px, int *py)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
	{
		*px = csbi.dwCursorPosition.X;
		*py = csbi.dwCursorPosition.Y;
		return 1;
	}
	return 0; // error
}

int Console::MaxXY(int *px, int *py)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
	{
		*px = csbi.srWindow.Right - csbi.srWindow.Left;
		*py = csbi.srWindow.Bottom - csbi.srWindow.Top;
		return 1;
	}
	return 0; // error
}

int Console::Clear()
{
	int mx, my;

	if (MaxXY(&mx, &my))
	{
		COORD szBuf = {(short)(mx + 1), (short)(my + 1)};
		COORD BufferCoord= {0, 0};
		SMALL_RECT srWriteRegion = {0, 0, (short)mx, (short)my};

		CHAR_INFO *lpBuffer = (CHAR_INFO *)calloc(szBuf.X * szBuf.Y, sizeof(CHAR_INFO));
		
		HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
		return WriteConsoleOutput(hStdOut, lpBuffer, szBuf, BufferCoord, &srWriteRegion);
	}
	return 0; // error
}

int Console::OutTxt(const char * format , ...)
{
	va_list arglist;
	char buffer[1024];
	int len;

	va_start(arglist, format);
	
	assert(format);
	len = _vsnprintf(buffer, sizeof(buffer)-1, format, arglist);
	assert(len >= 0);

	return _cputs(buffer) == 0 ? len : -1;
}

int Console::SetColor(ConsoleColors const  & color)
{
	if (color.IsInvalid())
	{
		assert(("invalid color value", false));
		return -1;
	}
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	return SetConsoleTextAttribute(hStdOut, (short)color.bits);
}

ConsoleColors Console::GetColor()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleScreenBufferInfo(hStdOut, &csbi))
		return ConsoleColors(csbi.wAttributes & 0xFF);
	else
		return ConsoleColors(-1); // error
}

int Console::ShowCursor(bool visible)
{
	CONSOLE_CURSOR_INFO ci;

	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleCursorInfo(hStdOut, &ci))
	{
		ci.bVisible = visible;
		return SetConsoleCursorInfo(hStdOut, &ci);
	}
	return 0; // error
}
////////////////////////////////////////////////////////////////

struct Console::IMG
{
	int cx;
	int cy;
	
	char *charbuf; // size is cx*cy
	short *attrbuf;
};

Console::IMG * Console::ImgAlloc(int width, int height)
{
	IMG *pImg = NULL;
	assert(width > 0 && height > 0);
	
	pImg = (IMG*) malloc(sizeof(IMG));
	if (pImg)
	{
		pImg->cx = width;
		pImg->cy = height;

		pImg->charbuf = (char*)calloc(height, sizeof(char)*width);
		if (pImg->charbuf == NULL)
		{
			free(pImg);
			return NULL;
		}
		pImg->attrbuf = (short*)calloc(height, sizeof(short)*width);
		if (pImg->attrbuf == NULL)
		{
			free(pImg->charbuf);
			free(pImg);
			return NULL;
		}
	}
	return pImg;
} /* ImgAlloc */

void Console::ImgFree(IMG * pImg)
{
	if (pImg)
	{
		free(pImg->charbuf);
		free(pImg->attrbuf);
		free(pImg);
	}
} /* ImgFree */

int Console::ImgPutChar(IMG * pImg, int x, int y, char ch, short attr)
{
	assert(pImg);
	if (x >= pImg->cx)
		return 0;
	if (y >= pImg->cy)
		return 0;
	
	{
		int pos = y * pImg->cx + x;
		pImg->charbuf[pos] = ch;
		pImg->attrbuf[pos] = attr;
	}
	return 1;
} /* ImgPutChar */

int Console::ImgFill(IMG * pImg, char ch, short attr)
{
	assert(pImg);
	{
		int pos;
		for (pos = 0; pos < pImg->cx * pImg->cy; ++pos)
		{
			pImg->charbuf[pos] = ch;
			pImg->attrbuf[pos] = attr;
		}
	}
	return 1;
} /* ImgPutChar */

int	Console::ImgPut(const IMG * pImg, int left, int top)
{
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	COORD szBuf = {(short)pImg->cx, (short)pImg->cy};
	COORD coord = {0, 0};
	SMALL_RECT rect = {(short)left, (short)top, (short)(left + pImg->cx), (short)(top + pImg->cy)};

	CHAR_INFO * ciBuffer = (CHAR_INFO *)malloc((pImg->cx * pImg->cy) * sizeof(CHAR_INFO));
	if (ciBuffer != NULL)
	{
		int res;
		int y;
		for (y = 0; y < pImg->cy; ++y) // rows
		{
			int x;
			for (x = 0; x < pImg->cx; ++x) // in row
			{
				int idx = y * pImg->cx + x;
				ciBuffer[idx].Char.AsciiChar = pImg->charbuf[idx];
				ciBuffer[idx].Attributes = pImg->attrbuf[idx];
			}
		}
		res = WriteConsoleOutput(hStdOut, ciBuffer, szBuf, coord, &rect);
		free(ciBuffer);
		return res;
	}

	SetLastError(ERROR_OUTOFMEMORY);
	return 0; // error
} /* ImgPut() */

////////////////////////////////////////////////////////
// Input functions

Console::Console() {}

int Console::KeyPressed()
{
	Console c;
	return _kbhit();
}

int Console::GetKey()
{
	int ch = _getch();

	if (ch == 0 || ch == 224) // extended code
		return - _getch();

	return ch;
}
