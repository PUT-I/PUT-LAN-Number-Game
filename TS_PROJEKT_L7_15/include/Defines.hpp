#pragma once

/*
 * Pole operration i answer:
 *	000 - Komunikat:
 *		000 - b³¹d jakiœ tam
 *
 *	001 - Rozgrywka:
 *		000 - pocz¹tek rozgrywki
 *		001 - remis
 *		010 - wygrana
 *		011 - przegrana
 *		111 - koniec rozgrywki
 *
 *	010 - Dane:
 *		000 - przes³anie id
 *		001 - przes³anie liczby
 *		010 - przes³anie czasu
 *
 *	011 - Liczba:
 *		000 - liczba nie odgadniêta
 *		001 - liczba odgagniêta
 *		010 - liczba za ma³a
 *		011 - liczba za du¿a
 */

//Operacje
#define OP_MESSAGE "000"
#define OP_GAME    "001"
#define OP_DATA    "010"
#define OP_NUMBER  "011"


//Komunikaty
#define MESSAGE_ERROR "000"

//Rozgrywka
#define GAME_BEGIN "000"
#define GAME_DRAW  "001"
#define GAME_WON   "010"
#define GAME_LOST  "011"
#define GAME_END   "111"

//Dane
#define DATA_ID     "000"
#define DATA_NUMBER "001"
#define DATA_TIME   "010"

//Liczba
#define NUMBER_NOT_GUESSED "000"
#define NUMBER_GUESSED     "001"
#define NUMBER_TOO_SMALL   "010"
#define NUMBER_TOO_BIG     "011"
