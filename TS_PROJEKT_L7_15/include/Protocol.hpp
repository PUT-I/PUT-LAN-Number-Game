#pragma once

#include "Defines.hpp"
#include <bitset>
#include <string>

inline std::ostream& operator << (std::ostream& os, const tm& time) {
	os << std::setfill('0') << std::setw(2) << time.tm_hour << ':' << std::setfill('0') << std::setw(2) << time.tm_min << ':' << std::setfill('0') << std::setw(2) << time.tm_sec;
	return os;
}

inline const tm GET_CURRENT_TIME() {
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
	std::bitset<5> padding{ NULL }; //Pole dope³nienia

public: static const unsigned int length = 3; //D³ugoœæ protoko³u w bajtach
private:
	//Deserializacja ca³ego ci¹gu bitów na poszczególne pola
	void deserialize(std::bitset<length * 8> &wholeData) {
		const int start = length * 8 - 1;

		//Pole operacji
		int end = start - operation.size();
		for (int i = start; i > end; i--) { operation[i - end - 1] = wholeData[i]; }

		//Pole odpowiedzi
		wholeData <<= operation.size();
		end = start - answer.size();
		for (int i = start; i > end; i--) { answer[i - end - 1] = wholeData[i]; }

		//Pole identyfikatora sesji
		wholeData <<= answer.size();
		end = start - id.size();
		for (int i = start; i > end; i--) { id[i - end - 1] = wholeData[i]; }

		//Pole danych
		wholeData <<= id.size();
		end = start - data.size();
		for (int i = start; i > end; i--) { data[i - end - 1] = wholeData[i]; }

		//Pole dope³nienia
		wholeData <<= data.size();
		end = start - padding.size();
		for (int i = start; i > end; i--) { padding[i - end - 1] = wholeData[i]; }
	}

public:

	//Konstruktory
	BinProtocol() : operation(NULL), answer(NULL), id(NULL), data(NULL) {}
	explicit BinProtocol(const std::string& data) { from_string(data); }
	explicit BinProtocol(const std::string& operation, const std::string& answer, const unsigned int& id, const unsigned int& data) {
		this->set(operation, answer, id, data);
	}


	//Settery
	void set_operation(std::string input) {
		input.resize(3);
		operation = std::bitset<3>(input);
	}

	void set_answer(std::string input) {
		input.resize(3);
		answer = std::bitset<3>(input);
	}

	void set_id(std::string input) {
		input.resize(5);
		id = std::bitset<5>(input);
	}
	void set_id(const unsigned int& input) { id = std::bitset<5>(input); }

	void set_data(std::string input) {
		input.resize(8);
		data = std::bitset<8>(input);
	}
	void set_data(const unsigned int& input) { data = std::bitset<8>(input); }

	void set(const std::string& operation, const std::string& answer, const unsigned int& id, const unsigned int& data) {
		this->set_operation(operation);
		this->set_answer(answer);
		this->set_id(id);
		this->set_data(data);
	}


	//Gettery
	const std::bitset<3>& get_operation() const { return operation; }

	const std::bitset<3>& get_answer() const { return answer; }

	const std::bitset<5>& get_id() const { return id; }
	const unsigned int get_id_int() const { return id.to_ulong(); }

	const std::bitset<8>& get_data() const { return data; }
	const unsigned int get_data_int() const { return data.to_ulong(); }

	//Porównywanie
	bool compare(const std::string& operation, const std::string& answer, const unsigned int& id) const {
		if (this->operation.to_string() != operation) { return false; }
		if (this->answer.to_string() != answer) { return false; }
		if (this->get_id_int() != id) { return false; }
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

	//Wyœwietlanie
	void display() const {
		std::cerr << operation << ' ' << answer << ' ' << id << ' ' << data << ' ' << padding << '\n';
	}
	friend std::ostream& operator << (std::ostream& out, const BinProtocol& input) {
		out << input.operation << ' ' << input.answer << ' ' << input.id << ' ' << input.data << ' ' << input.padding;
		return out;
	}
};