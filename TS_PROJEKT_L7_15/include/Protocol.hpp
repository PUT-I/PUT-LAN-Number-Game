#pragma once
#include <bitset>
#include <iostream>
#include <string>

class BinProtocol {
private:
	std::bitset<3> operation;
	std::bitset<3> answer;
	std::bitset<5> id;
	std::bitset<5> padding;

public:
	explicit BinProtocol(const std::string& data) { from_string(data); }
	explicit BinProtocol(const std::string& operation, const std::string& answer, const std::string& id) {
		this->set(operation, answer, id);
	}

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
	void set(const std::string& operation, const std::string& answer, const std::string& id) {
		this->setOperation(operation);
		this->setAnswer(answer);
		this->setId(id);
	}

	std::string to_string() const {
		std::bitset<8>tempBitset;
		std::string result;
		std::bitset<16>wholeData(std::string(operation.to_string() + answer.to_string() + id.to_string() + padding.to_string()));

		for (int i = 15; i >= 0; i--) {
			tempBitset[i % 8] = wholeData[i];
			if (i % 8 == 0) { result.push_back(char(tempBitset.to_ulong())); }
		}

		return result;
	}
	void from_string(std::string data) {
		data.resize(2);
		std::bitset<8>tempBitset;
		std::bitset<16>wholeData;

		tempBitset = std::bitset<8>(data[0]);
		for (int i = 15; i >= 8; i--) { wholeData[i] = tempBitset[i % 8]; }
		tempBitset = std::bitset<8>(data[1]);
		for (int i = 7; i >= 0; i--) { wholeData[i] = tempBitset[i % 8]; }


		for (int i = 15; i >= 13; i--) { operation[i - 13] = wholeData[i]; }
		for (int i = 12; i >= 10; i--) { answer[i - 10] = wholeData[i]; }
		for (int i = 9; i >= 5; i--) { id[i - 5] = wholeData[i]; }
		for (int i = 4; i >= 0; i--) { padding[i] = wholeData[i]; }
	}
	void display() const {
		std::cout << operation << ' ' << answer << ' ' << id << ' ' << padding << '\n';
	}
};
