#include "Utilities.h"

std::string utilities::removeBlankSpacesBeforeAndAfter(std::string& s) {
	if (s == "") {
		return std::string("");
	}
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
		digitSet = std::string("0123456789ABCDEFabcdef");
	}
	else if (number[0] == '\\') {
		begin = 1;
		digitSet = std::string("01234567");
	}
	else {
		if (number[0] == '+' || number[0] == '-') {
			begin++;
		}
		digitSet = std::string("0123456789");
	}

	if (begin < number.length()) {
		for (int i = begin; i < number.length(); i++) {
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

utilities::StringTokenizer::StringTokenizer(std::string& _line, const char *_delimiters) : line(_line), delimiters(_delimiters), last(0), next(0), lastDelimiter(0), nextDelimiter(0) { }

utilities::StringTokenizer::StringTokenizer(std::string&& _line, const char *_delimiters) : line(_line), delimiters(_delimiters), last(0), next(0), lastDelimiter(0), nextDelimiter(0) { }

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

bool utilities::StringTokenizer::hasNextToken() {
	return next < line.length();
}