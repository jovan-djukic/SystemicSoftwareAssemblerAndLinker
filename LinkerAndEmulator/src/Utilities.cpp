#include "Utilities.h"

std::string utilities::removeBlankSpacesBeforeAndAfter(std::string& s) {
	if (s == "") return std::string("");
	int low = 0, high = s.length() - 1;
	while (s[low] == ' ' || s[low] == '\t') {
		low++;
	}
	while ((s[high] == ' ' || s[high] =='\t') && high > low) {
		high--;
	}
	return s.substr(low, high - low + 1);
}

bool utilities::isNumber(std::string& s) {
	std::string number = utilities::removeBlankSpacesBeforeAndAfter(s);
	int begin = 0;
	std::string digitSet = "";
	if (number.substr(0, 2) == "0x") {
		begin = 2;
		digitSet = std::string("012345679ABCDEFabcdef");
	}
	else if (s[0] == '\\') {
		begin = 1;
		digitSet = std::string("01234567");
	}
	else {
		if (number[0] == '+' || number[0] == '-') {
			begin++;
		}
		digitSet = std::string("0123456789");
	}

	if (begin < s.length()) {
		for (int i = begin; i < s.length(); i++) {
			if (digitSet.find_first_of(number[i]) == std::string::npos) {
				return false;
			}
		}
		return true;
	}
	return false;
}

void utilities::pack(char *buffer, int number, int numOfBytes) {
	int mask = 0x000000ff;
	for (int i = 0; i < numOfBytes; i++) {
		buffer[i] = (number & mask) >> ( i * 8);
		mask <<= 8;
	}
}

int utilities::unpack(char *buffer, int numOfBytes) {
	int result = 0;
	for (int i = 0; i < numOfBytes; i++) {
		result |= ((int)buffer[i]) << (i * 8);
	}
	return result;
}

utilities::StringTokenizer::StringTokenizer(std::string& _line, const char *_delimiters) : line(_line), delimiters(_delimiters), last(0), next(0), lastDelimiter(0), nextDelimiter(0) { }

std::string utilities::StringTokenizer::nextToken() {
	/*if (next >= line.length()) {
		return std::string("");
	}*/

	if (nextDelimiter != 0) {
		next++;
		last = next;
	}

	lastDelimiter = nextDelimiter;
	while (delimiters.find_first_of(line[next]) == std::string::npos && line[next] != '\0') {
		next++;
	}
	nextDelimiter = line[next];

	std::string returnString = line.substr(last, next - last);
	//next++;
	//last = next;
	return returnString;
}

std::string utilities::StringTokenizer::getRestOfTheLine() {
	std::string returnString = line.substr(next + 1);
	next = last = line.length();
	return returnString;
}

char utilities::StringTokenizer::getLastDelimiter() {
	return lastDelimiter;
}

char utilities::StringTokenizer::getNextDelimiter() {
	return nextDelimiter;
}

bool utilities::StringTokenizer::hasNextToken() {
	return next < line.length();
}

utilities::CharInputStream::CharInputStream(char *_data, int _size) : data(_data), position(0), size(_size) { }

int utilities::CharInputStream::read(char *buffer, int num) {
	int numRead = 0;
	for (int i = 0; i < num && position < size; i++, position++) {
		buffer[i] = data[position];
		numRead++;
	}

	return numRead;
}

std::string utilities::CharInputStream::readString(int offset) {
	std::string word = "";
	for (int i = offset; data[i] != '\0' && i < size; i++) {
		word.append(1, data[i]);
	}

	return word;
}

bool utilities::CharInputStream::end() {
	return position == size;
}