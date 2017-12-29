#include "ErrorException.h"
#include<string>

ErrorException::ErrorException(std::string& _errorMessage) : errorMessage(_errorMessage) { }

std::ostream& operator<<(std::ostream& outputStream, ErrorException& error) {
	return outputStream << error.errorMessage;
}