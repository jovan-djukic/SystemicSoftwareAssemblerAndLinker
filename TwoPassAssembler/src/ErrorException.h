#ifndef _ERROR_EXCEPTION_H_
#define _ERROR_EXCEPTION_H_

#include <iostream>
#include <string>


class ErrorException {
private:
	std::string errorMessage;
public:
	ErrorException(std::string&);
	friend std::ostream& operator<<(std::ostream&, ErrorException&);
};

#endif