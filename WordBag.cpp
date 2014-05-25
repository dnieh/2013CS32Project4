#include "provided.h"
#include "MyMap.h"
#include <string>
using namespace std;

class WordBagImpl
{
public:
	WordBagImpl(const string& text);
	bool getFirstWord(string& word, int& count);
	bool getNextWord(string& word, int& count);

private:
	MyMap<std::string, int> m_map;
};

WordBagImpl::WordBagImpl(const string& text)
{
	// Must make a copy of text since it's passed as a const parameter
	std::string temp = text;

	// Make text lowercase per spec requirement
	strToLower(temp);

	Tokenizer t(temp);
	std::string w;

	while (t.getNextToken(w))
	{
		int* findInt = m_map.find(w);

		if (findInt == nullptr)
		{
			m_map.associate(w, 1);
			//std::cerr << w << " 1" << std::endl;
		}
		else
		{
			m_map.associate(w, ++*findInt);
			//std::cerr << w << " " << *findInt << std::endl;
		}
	}
}

bool WordBagImpl::getFirstWord(string& word, int& count)
{
	int* getFirst = m_map.getFirst(word);

	if (getFirst == nullptr)
		return false;

	else
	{
		count = *getFirst;
		return true;
	}

}

bool WordBagImpl::getNextWord(string& word, int& count)
{
	int* getNext = m_map.getNext(word);

	if (getNext == nullptr)
		return false;

	else
	{
		count = *getNext;
		return true;
	}
}

//******************** WordBag functions *******************************

// These functions simply delegate to WordBagImpl's functions.
// You probably don't want to change any of this code.

WordBag::WordBag(const std::string& text)
{
	m_impl = new WordBagImpl(text);
}

WordBag::~WordBag()
{
	delete m_impl;
}

bool WordBag::getFirstWord(string& word, int& count)
{
	return m_impl->getFirstWord(word, count);
}

bool WordBag::getNextWord(string& word, int& count)
{
	return m_impl->getNextWord(word, count);
}
