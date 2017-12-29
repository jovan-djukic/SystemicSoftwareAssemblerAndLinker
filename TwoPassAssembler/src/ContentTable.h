#ifndef _CONTENT_TABLE_H_
#define _CONTENT_TABLE_H_

#include <string>
#include <vector>

class ContentTable {
protected:
	std::string section;
	int size;
	int position;
public:
	ContentTable(std::string&, int);
	virtual int getSize() const;
	virtual int getCurrentPosition() const;
	virtual void addContent( const char*, int);
	virtual char* getContent();
	virtual ~ContentTable();
};

class StaticContentTable : public ContentTable {
private:
	char *content;
public:
	StaticContentTable(std::string&, int);
	int getSize() const;
	void addContent(const char*, int);
	char* getContent();
	~StaticContentTable();
};

class DynamicContentTable : public ContentTable {
private:
	std::string name;
	std::vector<char> content;
public:
	DynamicContentTable(std::string&, int);
	int getSize() const override;
	int getCurrentPosition() const;
	char* getContent();
	void addContent( const char*, int);
};

#endif