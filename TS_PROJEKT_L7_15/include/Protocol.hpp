#pragma once
#include <bitset>
#include <iostream>
#include <string>
/*
   Pole operation:
 * 000 - error.
 * 001 - pocz¹tek rozgrywki.

 * 011 - liczba odgadniêta
 * 010 - liczba nie odgadniêta

 * 100 - przes³anie identyfikatora sesji (pocz¹tek sesji).
 * 101 - przes³anie danych
 * 110 - przes³anie informacji o czasie

 * 111 - rozgrywka zakoñczona (koniec sesji).
 */

#define error "000"
#define gameStart "001"

#define numberGuessed "011"
#define gameNotGuessed "010"

#define sendId "100"
#define sendNumber "101"
#define sendTime "110"

#define gameEnd "111"


class BinProtocol {
private:
	std::bitset<3> operation;
	std::bitset<3> answer;
	std::bitset<5> id;
	std::bitset<8> data;
	std::bitset<5> padding{ NULL };

public:
	static const unsigned int length = 3; //D³ugoœæ protoko³u w bajtach


	//Constructors
	BinProtocol() : operation(NULL), answer(NULL), id(NULL), data(NULL) {}
	explicit BinProtocol(const std::string& data) { from_string(data); }
	explicit BinProtocol(const std::string& operation, const std::string& answer, const std::string& id, const unsigned int& data) {
		this->set(operation, answer, id, data);
	}


	//Setters
	void setOperation(std::string input) {
		input.resize(3);
		operation = std::bitset<3>(input);
	}

	void setAnswer(std::string input) {
		input.resize(3);
		answer = std::bitset<3>(input);
	}

	void setId(std::string input) {
		input.resize(5);
		id = std::bitset<5>(input);
	}
	void setId(const unsigned int& input) { id = std::bitset<5>(input); }

	void setData(std::string input) {
		input.resize(8);
		data = std::bitset<8>(input);
	}
	void setData(const unsigned int& input) { data = std::bitset<8>(input); }

	void set(const std::string& operation, const std::string& answer, const std::string& id, const unsigned int& data) {
		this->setOperation(operation);
		this->setAnswer(answer);
		this->setId(id);
		this->setData(data);
	}


	//Getters
	const std::bitset<3>& getOperation() const { return operation; }

	const std::bitset<3>& getAnswer() const { return answer; }

	const std::bitset<5>& getId() const { return id; }
	const unsigned int getId_Int() const { return id.to_ulong(); }

	const std::bitset<8>& getData() const { return data; }
	const unsigned int getData_Int() const { return data.to_ulong(); }


	//Other
	std::string to_string() const {
		std::bitset<8>tempBitset;
		std::string result;
		std::bitset<length * 8>wholeData(std::string(operation.to_string() + answer.to_string() + id.to_string() + data.to_string() + padding.to_string()));

		for (int i = length * 8 - 1; i >= 0; i--) {
			tempBitset[i % 8] = wholeData[i];
			if (i % 8 == 0) { result.push_back(char(tempBitset.to_ulong())); }
		}
		return result;
	}

	void from_string(std::string input) {
		input.resize(length);
		std::bitset<length * 8>wholeData;

		for (unsigned int i = 0; i < length; i++) {
			std::bitset<8> tempBitset(input[i]);
			for (int j = length * 8 - 1 - i * 8; j >= int(length * 8 - (i + 1) * 8); j--) {
				wholeData[j] = tempBitset[j % 8];
			}
		}

		int startOffset = 1;
		int endOffset = operation.size();
		for (int i = length * 8 - startOffset; i >= int(length * 8 - endOffset); i--) { operation[i - (length * 8 - endOffset)] = wholeData[i]; }

		startOffset += operation.size();
		endOffset = startOffset + answer.size() - 1;
		for (int i = length * 8 - startOffset; i >= int(length * 8 - endOffset); i--) { answer[i - (length * 8 - endOffset)] = wholeData[i]; }


		startOffset += answer.size();
		endOffset = startOffset + id.size() - 1;
		for (int i = length * 8 - startOffset; i >= int(length * 8 - endOffset); i--) { id[i - (length * 8 - endOffset)] = wholeData[i]; }


		startOffset += id.size();
		endOffset = startOffset + data.size() - 1;
		for (int i = length * 8 - startOffset; i >= int(length * 8 - endOffset); i--) { data[i - (length * 8 - endOffset)] = wholeData[i]; }


		startOffset += data.size();
		for (int i = length * 8 - startOffset; i >= 0; i--) { padding[i] = wholeData[i]; }
	}

	void display() const {
		std::cout << operation << ' ' << answer << ' ' << id << ' ' << data << ' ' << padding << '\n';
	}
};
