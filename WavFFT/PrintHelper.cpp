#include "PrintHelper.h"


namespace console_helper
{
	bool inited(false);

	short leftAlign;
	short centerAlign;
	short rightAlign;

	short verticalTopAlign;
	short verticalCenterAlign;
	short verticalBottomAlign;

	HANDLE hConsole;


	void init()
	{
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(hConsole, &csbi);

		leftAlign = 0;
		centerAlign = csbi.dwSize.X / 2;
		rightAlign = csbi.dwSize.X;

		verticalTopAlign = 0;
		verticalCenterAlign = csbi.dwSize.Y / 2;
		verticalBottomAlign = csbi.dwSize.Y;

		inited = true;
	}

	void centerPrint(const char* text, short height)
	{
		if (!inited)
			throw "No aligns available";

		COORD printPos{ centerAlign - short(strlen(text) + 1) / 2, height };
		SetConsoleCursorPosition(hConsole, printPos);
		std::cout << text;
	}

	void leftPrint(const char* text, short height)
	{
		if (!inited)
			throw "No aligns available";

		COORD printPos{ leftAlign, height };
		SetConsoleCursorPosition(hConsole, printPos);
		std::cout << text;
	}

	void rightPrint(const char* text, short height)
	{
		if (!inited)
			throw "No aligns available";

		COORD printPos{ rightAlign - short(strlen(text) + 1), height };
		SetConsoleCursorPosition(hConsole, printPos);
		std::cout << text;
	}

	void printCoords(const char* text, short x, short y)
	{
		COORD printPos{ x, y };
		SetConsoleCursorPosition(hConsole, printPos);
		std::cout << text;
	}

	void setColor(short color)
	{
		SetConsoleTextAttribute(hConsole, color);
	}
	void unsetColor()
	{
		SetConsoleTextAttribute(hConsole, 7);
	}

	void showCursor(bool show)
	{
		CONSOLE_CURSOR_INFO cursorInfo;
		GetConsoleCursorInfo(hConsole, &cursorInfo);
		cursorInfo.bVisible = show;
		SetConsoleCursorInfo(hConsole, &cursorInfo);
	}
}