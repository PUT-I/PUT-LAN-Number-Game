#pragma once

/*
 * Pole operration i answer:
 *	000 - Komunikat:
 *		000 - nadawca roz��czony
 *		001 - przeciwnik roz��czony
 *		010 - czekanie na przeciwnika
 *
 *	001 - Rozgrywka:
 *		000 - pocz�tek rozgrywki
 *		001 - remis
 *		010 - wygrana
 *		011 - przegrana
 *		111 - koniec rozgrywki
 *
 *	010 - Dane:
 *		000 - przes�anie id
 *		001 - przes�anie liczby
 *
 *	011 - Liczba:
 *		000 - liczba za ma�a
 *		001 - liczba za du�a
 *		
 *	100 - Czas:
 *		000 - czas trwania rozgrywki
 *		001 - czas do rozpocz�cia rozgrywki
 *		010 - czas do ko�ca rozgrywki
 */

//Operacje

#define OP_MESSAGE "000" //Komunitat
#define OP_GAME    "001" //Rozgrywka
#define OP_DATA    "010" //Dane
#define OP_NUMBER  "011" //Liczba
#define OP_TIME	   "100" //Czas


//Komunikaty

#define MESSAGE_SENDER_DISCONNECTED   "000" //Nadawca roz��czony
#define MESSAGE_OPPONENT_DISCONNECTED "001" //Przecwnik roz��czony
#define MESSAGE_WAITING_FOR_OPPONENT  "010" //Czekanie na przeciwnika


//Rozgrywka

#define GAME_BEGIN "000" //Rozpocz�cie rozgrywki
#define GAME_DRAW  "001" //Remis
#define GAME_WON   "010" //Wygrana
#define GAME_LOST  "011" //Przegrana
#define GAME_END   "111" //Koniec rozgrywki

//Dane

#define DATA_ID     "000" //Identyfikator sesji
#define DATA_NUMBER "001" //Liczba


//Liczba

#define NUMBER_TOO_SMALL   "000" //Liczba za ma�a
#define NUMBER_TOO_BIG     "001" //Liczba za du�a


//Czas

#define TIME_DURATION "000" //Czas trwania rozgrywki
#define TIME_TO_START "001" //Czas do rozpocz�cia rozgrywki
#define TIME_LEFT     "010" //Czas pozosta�y do ko�ca rozgrywki
