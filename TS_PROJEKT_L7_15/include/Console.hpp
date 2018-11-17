#pragma once
#include "ThreadSafe.hpp"
#include <Windows.h>
#include <conio.h>

void CURSOR_POS(const COORD &c) noexcept {
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void CURSOR_POS(const unsigned int &x, const unsigned int &y) noexcept {
	COORD c;
	c.X = x;
	c.Y = y;
	CURSOR_POS(c);
}

const COORD GET_CURSOR_POS() noexcept {
	CONSOLE_SCREEN_BUFFER_INFO cbsi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cbsi);
	return cbsi.dwCursorPosition;
}

void CURSOR_MOVE(const int &movX, const int &movY) noexcept {
	COORD c = GET_CURSOR_POS();
	c.X += movX;
	c.Y += movY;

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void SHOW_CONSOLE_CURSOR(const bool &showFlag) noexcept {
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_CURSOR_INFO     cursorInfo;

	GetConsoleCursorInfo(out, &cursorInfo);
	cursorInfo.bVisible = showFlag; // set the cursor visibility
	SetConsoleCursorInfo(out, &cursorInfo);
}



//Checking Keyboard Input ---------------
const bool CHECK_ENTER() noexcept {
	return GetAsyncKeyState(VK_RETURN) & 0x8000;
}
const bool CHECK_SPACE() noexcept {
	return GetAsyncKeyState(VK_SPACE) & 0x8000;
}
const bool CHECK_ESCAPE() noexcept {
	return GetAsyncKeyState(VK_ESCAPE) & 0x8000;
}
const bool CHECK_LEFT_ARROW() noexcept {
	return GetAsyncKeyState(VK_LEFT) & 0x8000;
}
const bool CHECK_RIGHT_ARROW() noexcept {
	return GetAsyncKeyState(VK_RIGHT) & 0x8000;
}
const bool CHECK_UP_ARROW() noexcept {
	return GetAsyncKeyState(VK_UP) & 0x8000;
}
const bool CHECK_DOWN_ARROW() noexcept {
	return GetAsyncKeyState(VK_DOWN) & 0x8000;
}
const bool CHECK_ALT() noexcept {
	return GetAsyncKeyState(VK_MENU) & 0x8000;
}
void CHECK_ALT_F4() noexcept {
	if (CHECK_ALT() && GetAsyncKeyState(VK_F4) & 0x8000) exit(0);
}

inline void INPUT_STRING(std::string &str, const unsigned int &limit) {
	bool enter = false;
	while (!enter) {
		while (!enter) {
			CURSOR_MOVE(-int(str.size()), 0);
			sync_cout << str;
			SHOW_CONSOLE_CURSOR(true);
			const char c = _getch();
			SHOW_CONSOLE_CURSOR(false);

			if (c != 0x00) {
				CHECK_ALT_F4();

				if (CHECK_ENTER()) { enter = true; break; }
				else if (CHECK_UP_ARROW()) { continue; }
				else if (CHECK_DOWN_ARROW()) { continue; }
				else if (CHECK_LEFT_ARROW()) { continue; }
				else if (CHECK_RIGHT_ARROW()) { continue; }
				else if (CHECK_ALT()) { continue; }
				else if (c == 0x08) {	//When backspace pressed
					if (str.size() > 0) {
						CURSOR_MOVE(-1, 0);
						sync_cout << ' ';
						CURSOR_MOVE(-1, 0);
						str.pop_back();
					}
				}
				else if (c >= '0' && c <= '9' || c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z') {
					if (str.size() < limit) {
						CURSOR_MOVE(1, 0);
						str += c;
					}
				}
			}
		}
	}
	SHOW_CONSOLE_CURSOR(true);
}
