#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <string>

namespace utilities {
	std::string removeBlankSpacesBeforeAndAfter(std::string&);
	
	bool isNumber(std::string&);
	
	void pack(char*, int, int);

	class StringTokenizer {
	private :
		std::string line, delimiters;
		char lastDelimiter, nextDelimiter;
		int last, next;
	public:
		StringTokenizer(std::string&, const char*);
		StringTokenizer(std::string&&, const char*);
		std::string nextToken();
		std::string getRestOfTheLine();
		char getLastDelimiter();
		bool hasNextToken();
	};
}

#endif