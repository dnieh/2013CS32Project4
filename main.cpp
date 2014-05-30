#include <string>
#include <iostream>
#include <cassert>
#include "MyMap.h"
#include "provided.h"
#include "Indexer.h"

// KNOWN BUGS
// * [FIXED?] Loading from the same indexer object that has just been saved causes a crash.
//   Likely caused by clearing the MyMap<string, vector<HashedUrlCount> > object.
// * [PARTIALLY FIXED] Templated functions in Indexer.cpp may be causing problems. Work around functions 
//   were used. Templated functions should only be in header files
// * [FIXED] What happens if you try to add more Urls to the hash table after loading from
//   a save file? Does the hash table need to be pre-loaded with valid hashed url values?

// TODO
// * Consider using a list cointainer instead of a vector for HashedUrlCounts
// * Understand why template functions in indexer.h do not need to be inline while all other
//   functions need to be.

// Testing Functions
void MyMapTest();
void WordBagTest();
void WordBagTestPrint(WordBag& wb);
bool IndexerTest();
void writeWordInfo(Indexer& indexer, std::string word);

int main()
{
	//MyMapTest();
	//WordBagTest();
	IndexerTest();

	std::cerr << "Passed all tests!" << std::endl;
}

void MyMapTest()
{
	MyMap<std::string, double> nTG;  // name to GPA

	nTG.associate("Daniel", 3.8);
	nTG.associate("Carey", 3.5);
	nTG.associate("Fart", 3.9);

	double* danielsGPA = nTG.find("Daniel");
	if (danielsGPA != nullptr)
		*danielsGPA = 1.5;
	else
		std::cerr << "Key not found" << std::endl;

	std::cerr << "Here are the " << nTG.size() << " associations: "
		<< std::endl;

	std::string name;
	for (double* GPAptr = nTG.getFirst(name); GPAptr != nullptr;
		GPAptr = nTG.getNext(name))
	{
		std::cerr << name << " has a GPA of " << *GPAptr << std::endl;
	}
}

void WordBagTest()
{
	std::string webPageContent;
	webPageContent = "<html>I'm living' 2 love hash tables, I say.</html>";
	WordBag wb(webPageContent);

	WordBagTestPrint(wb);
}

void WordBagTestPrint(WordBag& wb)
{
	std::string word;
	int count;
	bool gotAWord = wb.getFirstWord(word, count);

	while (gotAWord)
	{
		std::cerr << "The word " << word << " occurs " << count
			<< " times" << std::endl;
		gotAWord = wb.getNextWord(word, count);
	}
}

bool IndexerTest()
{
	const std::string INDEX_PREFIX = "C:/Temp/myIndex";

	Indexer indexer;

	WordBag wb1("<html>i like gogiberries and I hate spam</html>");
	assert(indexer.incorporate("www.a.com", wb1));
	//WordBagTestPrint(wb1);

	WordBag wb2("<html>i like gogiberries and I hate spam</html>");
	assert(indexer.incorporate("www.b.com", wb2));


	//writeWordInfo(indexer, "I");
	// writes "I appears 2 times at www.a.com

	// Save the index as file(s) whose names start with prefix
	if (!indexer.save(INDEX_PREFIX))
	{
		std::cerr << "Error saving index" << std::endl;
		return false;
	}
	else
		std::cerr << "Index saved successfully" << std::endl;

	// Load the just-saved index into another indexer
	Indexer indexer2;
	if (!indexer2.load(INDEX_PREFIX))
	{
		std::cerr << "Error loading index" << std::endl;
		return false;  // Error loading the index
	}
	else
		std::cerr << "Index loaded successfully" << std::endl;

	// Add more pages to the second index
	WordBag wb3("<html>engineering is FUN</html>");
	assert(indexer2.incorporate("www.c.com", wb3));
	WordBag wb4("<html>Engineering majors like like like like fun</html>");
	assert(indexer2.incorporate("www.d.com", wb4));

	writeWordInfo(indexer2, "like");
	// writes:
		// "like appears 1 times at www.a.com"
		// "like appears 1 times at www.b.com"

	writeWordInfo(indexer2, "smallberg");
	// writes "smallberg was not found in the index"

	return true;
}

void writeWordInfo(Indexer& indexer, std::string word)
{
	std::vector<UrlCount> urlCounts = indexer.getUrlCounts(word);

	if (urlCounts.empty())
	{
		std::cerr << word << " was not found in the index." << std::endl;
		return;
	}

	for (unsigned int i = 0; i < urlCounts.size(); i++)
		std::cerr << word << " appeears " << urlCounts[i].count
		<< " times at " << urlCounts[i].url << std::endl;
}