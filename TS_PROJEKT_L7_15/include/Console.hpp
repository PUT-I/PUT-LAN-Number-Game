#pragma once
#include "ThreadSafe.hpp"
#include <Windows.h>
#include <conio.h>

class CONSOLE_MANIP {
private:
	static void cursor_set_pos(const COORD& c) noexcept {
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
	}

	static void cursor_move(const int& movX, const int& movY) noexcept {
		COORD c = cursor_get_pos();
		c.X += movX;
		c.Y += movY;

		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
	}



	//Funkcje do sprawdzania stanów klawiatury
	static const bool CHECK_ALT() noexcept { return GetAsyncKeyState(VK_MENU) & 0x8000; }
	static void CHECK_ALT_F4() noexcept { if (CHECK_ALT() && GetAsyncKeyState(VK_F4) & 0x8000) exit(0); }
	static const bool CHECK_OTHER_THAN_NUM(const char& c) {
		if (c >= 0x00 && c <= 0x2f) { return true; }
		//Zakres liczb
		else if (c >= 0x30 && c <= 0x39) { return false; }
		else if (c >= 0x3A && c <= 0x40) { return true; }
		//Zakres znaków
		else if (c >= 0x41 && c <= 0x5A) { return true; }
		else if (c >= 0x5b) { return true; }
		return false;
	}

public:
	//Ustawianie widocznoœci kursora
	static void show_console_cursor(const bool &showFlag) noexcept {
		HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

		CONSOLE_CURSOR_INFO     cursorInfo;

		GetConsoleCursorInfo(out, &cursorInfo);
		cursorInfo.bVisible = showFlag; // set the cursor visibility
		SetConsoleCursorInfo(out, &cursorInfo);
	}

	//ZMiana pozycji kursora
	static void cursor_set_pos(const unsigned int& x, const unsigned int& y) noexcept {
		COORD c;
		c.X = x;
		c.Y = y;
		cursor_set_pos(c);
	}

	//Uzyskiwanie aktualnej pozycji kursora
	static const COORD cursor_get_pos() noexcept {
		CONSOLE_SCREEN_BUFFER_INFO cbsi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cbsi);
		return cbsi.dwCursorPosition;
	}

	//Czyszczenie konsoli za pomoc¹ funkcji z windows api
	static void clear_console() noexcept {
		COORD topLeft = { 0, 0 };
		HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO screen;
		DWORD written;

		GetConsoleScreenBufferInfo(console, &screen);
		FillConsoleOutputCharacterA(
			console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
		);
		FillConsoleOutputAttribute(
			console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
			screen.dwSize.X * screen.dwSize.Y, topLeft, &written
		);
		SetConsoleCursorPosition(console, topLeft);
	}

	//Wprowadzanie danych przez u¿ytkownika z ograniczeniem liczby znaków
	static void input_string(std::string &str, const unsigned int &limit) {
		while (true) {
			cursor_move(-int(str.size()), 0);
			sync_cout << str;

			show_console_cursor(true);
			const char c = _getch();
			if (str.size() != limit) { show_console_cursor(false); }

			CHECK_ALT_F4();

			if (c == 0x0d) { break; }
			else if (c == 0x08) {	//Jeœli wprowadzono backspace
				if (str.size() > 0) {
					cursor_move(-1, 0);
					sync_cout << ' ';
					cursor_move(-1, 0);
					str.pop_back();
				}
			}
			else if (CHECK_OTHER_THAN_NUM(c)) { continue; }
			else if (c >= '0' && c <= '9' || c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z') {
				if (str.size() < limit) {
					cursor_move(1, 0);
					str += c;
				}
			}
		}
		show_console_cursor(true);
	}
};
