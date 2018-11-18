#pragma once

#include "Defines.hpp"
#include <bitset>
#include <string>

inline std::ostream& operator << (std::ostream& os, const tm& time) {
	os << std::setfill('0') << std::setw(2) << time.tm_hour << ':' << std::setfill('0') << std::setw(2) << time.tm_min << ':' << std::setfill('0') << std::setw(2) << time.tm_sec;
	return os;
}

inline const tm GetCurrentTimeTm() {
	time_t tt;
	time(&tt);
	tm time;
	localtime_s(&time, &tt);
	time.tm_year += 1900;
	time.tm_mon += 1;
	return time;
}

class BinProtocol {
private:
	std::bitset<3> operation;		//Pole operacji
	std::bitset<3> answer;			//Pole odpowiedzi
	std::bitset<5> id;				//Pole identyfikatora sesji
	std::bitset<8> data;			//Pole danych
	std::bitset<5> padding{ NULL }; //Pole dope�nienia

public: static const unsigned int length = 3; //D�ugo�� protoko�u w bajtach
private:
	//Desetializacja ca�ego ci�gu bit�w na poszczeg�lne pola
	void deserialize(const std::bitset<length * 8>& wholeData) {
		//Pole operacji
		int startOffset = 1;
		int endOffset = operation.size();
		for (int i = length * 8 - startOffset; i >= int(length * 8 - endOffset); i--) { operation[i - (length * 8 - endOffset)] = wholeData[i]; }

		//Pole odpowiedzi
		startOffset += operation.size();
		endOffset = startOffset + answer.size() - 1;
		for (int i = length * 8 - startOffset; i >= int(length * 8 - endOffset); i--) { answer[i - (length * 8 - endOffset)] = wholeData[i]; }

		//Pole identyfikatora sesji
		startOffset += answer.size();
		endOffset = startOffset + id.size() - 1;
		for (int i = length * 8 - startOffset; i >= int(length * 8 - endOffset); i--) { id[i - (length * 8 - endOffset)] = wholeData[i]; }

		//Pole danych
		startOffset += id.size();
		endOffset = startOffset + data.size() - 1;
		for (int i = length * 8 - startOffset; i >= int(length * 8 - endOffset); i--) { data[i - (length * 8 - endOffset)] = wholeData[i]; }

		//Pole dope�nienia
		startOffset += data.size();
		for (int i = length * 8 - startOffset; i >= 0; i--) { padding[i] = wholeData[i]; }
	}

public:

	//Konstruktory
	BinProtocol() : operation(NULL), answer(NULL), id(NULL), data(NULL) {}
	explicit BinProtocol(const std::string& data) { from_string(data); }
	explicit BinProtocol(const std::string& operation, const std::string& answer, const unsigned int& id, const unsigned int& data) {
		this->set(operation, answer, id, data);
	}


	//Settery
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

	void set(const std::string& operation, const std::string& answer, const unsigned int& id, const unsigned int& data) {
		this->setOperation(operation);
		this->setAnswer(answer);
		this->setId(id);
		this->setData(data);
	}


	//Gettery
	const std::bitset<3>& getOperation() const { return operation; }

	const std::bitset<3>& getAnswer() const { return answer; }

	const std::bitset<5>& getId() const { return id; }
	const unsigned int getId_Int() const { return id.to_ulong(); }

	const std::bitset<8>& getData() const { return data; }
	const unsigned int getData_Int() const { return data.to_ulong(); }

	//Por�wnywanie
	bool compare(const std::string& operation, const std::string& answer, const unsigned int& id) const {
		if (this->operation.to_string() != operation) { return false; }
		if (this->answer.to_string() != answer) { return false; }
		if (this->getId_Int() != id) { return false; }
		return true;
	}

	//Serializacja
	std::string to_string() const { //Zamiana na std::string
		std::bitset<8>tempBitset;
		std::string result;
		std::bitset<length * 8>wholeData(std::string(operation.to_string() + answer.to_string() + id.to_string() + data.to_string() + padding.to_string()));

		for (int i = length * 8 - 1; i >= 0; i--) {
			tempBitset[i % 8] = wholeData[i];
			if (i % 8 == 0) { result.push_back(char(tempBitset.to_ulong())); }
		}
		return result;
	}

	//Deserializacja
	void from_char_a(const char* input) { 
		std::bitset<length * 8>wholeData;
		for (unsigned int i = 0; i < length; i++) {
			std::bitset<8>tempBitset(input[i]);
			for (int j = length * 8 - i * 8 - 1; j >= int(length * 8 - (i + 1) * 8); j--) {
				wholeData[j] = tempBitset[j % 8];
			}
		}
		this->deserialize(wholeData);
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
		this->deserialize(wholeData);
	}

	//Wy�wietlanie
	void display() const {
		std::cerr << operation << ' ' << answer << ' ' << id << ' ' << data << ' ' << padding << '\n';
	}
	friend std::ostream& operator << (std::ostream& out, const BinProtocol& input) {
		out << input.operation << ' ' << input.answer << ' ' << input.id << ' ' << input.data << ' ' << input.padding;
		return out;
	}
};