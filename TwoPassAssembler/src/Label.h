#ifndef _LABEL_H_
#define _LABEL_H_

#include "Token.h"
#include "Tables.h"
#include <string>

class Label : public Token {
private:
	std::string name;
public:
	Label(std::string&, int);
	void resolve(Tables*);
};

#endif