#include "SectionList.h"

const std::string SectionList::all = "__ALL__";

SectionList::Entry::Entry() { }

SectionList::Entry::Entry(std::string& _name, std::string& _file, int _offset, int _size) : name(_name), file(_file), offset(_offset), size(_size) { }

SectionList::Entry::Entry(Entry& entry) : name(entry.name), file(entry.file), offset(entry.offset), size(entry.size) { }

SectionList::SectionList() : head(nullptr) { }

void SectionList::addSection(std::string& file, std::string& name, int offset, int size) {
	Node *node = new Node();
	node->entry = Entry(name, file, offset, size);
	node->next = nullptr;

	Node *current = head, *last = nullptr;
	while (current != nullptr && current->entry.name <= node->entry.name) {
		last = current;
		current = current->next;
	}

	if (last == nullptr) {
		node->next = head;
		head = node;
	}
	else {
		node->next = current;
		last->next = node;
	}
}

bool SectionList::doesExist(std::string& file, std::string& name) {
	Node *current = head;
	
	while (current != nullptr) {
		if (current->entry.file == file && (current->entry.name == name || name == all)) {
			return true;
		}
		else {
			current = current->next;
		}	
	}

	return false;
}

SectionList::Entry SectionList::remove(std::string& file, std::string& name) {
	Node *current = head, *last = nullptr;
	while (current->entry.file != file || (current->entry.name != name && name != all)) {
		last = current;
		current = current->next;
	}
	
	if (last == nullptr) {
		head = head->next;
	}
	else {
		last->next = current->next;
	}

	Entry retEntry = current->entry;
	delete current;
	return retEntry;
}

bool SectionList::hasNext() {
	return head != nullptr;
}

SectionList::Entry SectionList::removeNext() {
	Node *node = head;
	head = head->next;
	Entry retEntry = node->entry;
	delete node;
	return retEntry;
}

void SectionList::destroy() {
	while (head != nullptr) {
		Node *old = head;
		head = head->next;
		delete old;
	}
	head = nullptr;
}

SectionList::~SectionList() {
	destroy();
}

void SectionList::copy(SectionList& source) {
	Node *currentSource = source.head, *currentDestination = nullptr;
	while (currentSource != nullptr) {
		Node *newNode = new Node();
		newNode->entry = Entry(currentSource->entry);
		newNode->next = nullptr;
		if (currentDestination == nullptr) {
			head = currentDestination = newNode;
		}
		else {
			currentDestination->next = newNode;
			currentDestination = newNode;
		}
		currentSource = currentSource->next;
	}
}

void SectionList::move(SectionList& source) {
	head = source.head;
	source.head = nullptr;
}

SectionList& SectionList::operator=(SectionList& source) {
	if (this != &source) {
		destroy();
		copy(source);
	}
	return *this;
}

SectionList& SectionList::operator=(SectionList&& source) {
	if (this != &source) {
		destroy();
		move(source);
	}
	return *this;
}