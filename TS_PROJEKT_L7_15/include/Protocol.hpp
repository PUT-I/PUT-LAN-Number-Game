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
	void setOperation(const std::string& input) {
		operation = std::bitset<3>(input);
	}
	void setAnswer(const std::string& input) {
		answer = std::bitset<3>(input);
	}
	void setId(const std::string& input) {
		id = std::bitset<5>(input);
	}

	std::string to_string() const
	{
		std::bitset<8>tempBitset;
		std::string result, wholeDataStr;

		wholeDataStr = operation.to_string() + answer.to_string() + id.to_string() + id.to_string();
		std::bitset<16>wholeData(wholeDataStr);

		for (int i = 15; i >= 0; i--) {
			tempBitset[i % 8] = wholeData[i];
			if (i % 8 == 0) { result.push_back(char(tempBitset.to_ulong())); }
		}

		return result;
	}
	void from_string(std::string data) {
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
		std::cout << "Operation: " << operation << '\n';
		std::cout << "Answer: " << answer << '\n';
		std::cout << "Id: " << id << '\n';
		std::cout << "Padding: " << padding << '\n';
	}
};
