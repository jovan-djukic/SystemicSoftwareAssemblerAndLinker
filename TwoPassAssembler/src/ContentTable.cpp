#include "ContentTable.h"
#include <string>

ContentTable::ContentTable(std::string& _section, int _size) : section(_section), size(_size), position(0) { }

int ContentTable::getSize() const {
	return size;
}

int ContentTable::getCurrentPosition() const {
	return position;
}

void ContentTable::addContent(const char *buffer, int size) { }

char* ContentTable::getContent() {
	return nullptr;
}

ContentTable::~ContentTable() { }

StaticContentTable::StaticContentTable(std::string& _section,int _size) : ContentTable(_section, _size), content(nullptr) { }

int StaticContentTable::getSize() const {
	return size;
}

void StaticContentTable::addContent(const char *buffer, int num) {
	if (content == nullptr) {
		content = new char[size];
	}
	for (int i = 0; i < num; i++) {
		content[position++] = buffer[i];
	}
}

char* StaticContentTable::getContent() {
	return content;
}

StaticContentTable::~StaticContentTable() {
	if (content != nullptr) {
		delete[] content;
	}
}

DynamicContentTable::DynamicContentTable(std::string& _name, int _size) : ContentTable(_name, _size) { }

int DynamicContentTable::getSize() const {
	return content.size();
}

int DynamicContentTable::getCurrentPosition() const {
	return content.size();
}

void DynamicContentTable::addContent(const char *buffer, int num) {
	for (int i = 0; i < num; i++) {
		content.push_back(buffer[i]);
	}
}

char* DynamicContentTable::getContent() {
	return content.data();
}