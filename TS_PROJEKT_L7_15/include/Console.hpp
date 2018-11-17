#pragma once
#include "ThreadSafe.hpp"
#include <Windows.h>
#include <conio.h>

class CONSOLE_MANIP {
private:
	static void CURSOR_POS(const COORD &c) noexcept {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
	}

	static void CURSOR_POS(const unsigned int &x, const unsigned int &y) noexcept {
		COORD c;
		c.X = x;
		c.Y = y;
		CURSOR_POS(c);
	}

	static const COORD GET_CURSOR_POS() noexcept {
		CONSOLE_SCREEN_BUFFER_INFO cbsi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cbsi);
		return cbsi.dwCursorPosition;
	}

	static void CURSOR_MOVE(const int &movX, const int &movY) noexcept {
		COORD c = GET_CURSOR_POS();
		c.X += movX;
		c.Y += movY;

		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
	}

	static void SHOW_CONSOLE_CURSOR(const bool &showFlag) noexcept {
		HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_CURSOR_INFO     cursorInfo;

		GetConsoleCursorInfo(out, &cursorInfo);
		cursorInfo.bVisible = showFlag; // set the cursor visibility
		SetConsoleCursorInfo(out, &cursorInfo);
	}



	//Checking Keyboard Input ---------------
	static const bool CHECK_ALT() noexcept { return GetAsyncKeyState(VK_MENU) & 0x8000; }
	static void CHECK_ALT_F4() noexcept { if (CHECK_ALT() && GetAsyncKeyState(VK_F4) & 0x8000) exit(0); }

	static const bool CHECK_OTHER_THAN_NUM(const char c) {
		if (c >= 0x00 && c <= 0x2f) { return true; }
		//Zakres liczb
		else if (c >= 0x30 && c <= 0x39) { return false; }
		else if (c >= 0x3A && c <= 0x40) { return true; }
		//Zakres znak�w
		else if (c >= 0x41 && c <= 0x5A) { return true; }
		else if (c >= 0x5b) { return true; }
		return false;
	}

public:
	inline static void INPUT_STRING(std::string &str, const unsigned int &limit) {
		while (true) {
			CURSOR_MOVE(-int(str.size()), 0);
			sync_cout << str;
			SHOW_CONSOLE_CURSOR(true);
			const char c = _getch();
			SHOW_CONSOLE_CURSOR(false);

			CHECK_ALT_F4();

			if (c == 0x0d) { break; }
			else if (c == 0x08) {	//When backspace pressed
				if (str.size() > 0) {
					CURSOR_MOVE(-1, 0);
					sync_cout << ' ';
					CURSOR_MOVE(-1, 0);
					str.pop_back();
				}
			}
			else if (CHECK_OTHER_THAN_NUM(c)) { continue; }
			else if (c >= '0' && c <= '9' || c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z') {
				if (str.size() < limit) {
					CURSOR_MOVE(1, 0);
					str += c;
				}
			}
		}
		SHOW_CONSOLE_CURSOR(true);
	}
};
