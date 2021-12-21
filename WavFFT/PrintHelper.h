#pragma once

#include <iostream>
#include <Windows.h>

namespace console_helper
{
	extern bool inited;

	extern short leftAlign;
	extern short centerAlign;
	extern short rightAlign;

	extern short verticalTopAlign;
	extern short verticalCenterAlign;
	extern short verticalBottomAlign;

	void init();

	void centerPrint(const char* text, short height);
	void leftPrint(const char* text, short height);
	void rightPrint(const char* text, short height);
	void printCoords(const char* text, short x, short y);

	void setColor(short color);
	void unsetColor();

	void showCursor(bool show);
}