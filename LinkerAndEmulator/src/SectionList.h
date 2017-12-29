#ifndef _SECTION_LIST_H_
#define _SECTION_LIST_H_

#include <string>

class SectionList {
public:
	struct Entry {
		std::string name;
		std::string file;
		int offset, size;
		Entry();
		Entry(std::string&, std::string&, int, int);
		Entry(Entry&);
	};
	static const std::string all;
private:
	struct Node {
		Entry entry;
		Node *next;
	};
	Node *head;

	void copy(SectionList&);
	void move(SectionList&);
	void destroy();
public:
	SectionList();
	void addSection(std::string&, std::string&, int, int);
	bool doesExist(std::string&, std::string&);
	Entry remove(std::string&, std::string&);
	bool hasNext();
	Entry removeNext();

	SectionList& operator=(SectionList&);
	SectionList& operator=(SectionList&&);

	~SectionList();
};

#endif