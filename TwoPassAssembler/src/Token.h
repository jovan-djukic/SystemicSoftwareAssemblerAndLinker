#ifndef _TOKEN_H_
#define _TOKEN_H_

#include "Tables.h"

#define INVALID_CHARACTERS "() \t,*/!?"

class Token {
protected:
	int line;
	int getClassificationIndex(Tables*, std::string);
	int getExpressionValue(Tables*, std::string);
public:
	Token(int);
	virtual void resolve(Tables*);
	virtual ~Token();
};

#endif