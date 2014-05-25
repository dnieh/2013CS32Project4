#include "provided.h"
#include "MyMap.h"
#include <string>
#include <vector>
#include <fstream>  // for save and load
#include <sstream>  // for istringstream
#include <stdlib.h> // for atoi (string to int in load function)
using namespace std;

// TODO: Consider using a list cointainer instead of a vector for HashedUrlCounts

// Similar to the UrlCount struct in provided.h but this version potentially saves
// much more space by using converted lengthy url strings to smaller ints
struct HashedUrlCount
{
	int hashedUrl;
	int count;
};

class IndexerImpl
{
private:

	// Load factor of 10% assuming 1000 values (hashed url strings) to insert
	// 10,007 is the closest prime number after 10,000 (for more equal distribution)
	static const int HASH_TABLE_SIZE = 10007;

	struct Bucket
	{
		// Default false used value
		Bucket(){ used = false; }
		int hashedId;
		string originalId;
		bool used;
	};

	class ClosedHashTable
	{
	public:
		void insert(int hashedId, string originalId)
		{
			int bucket = hashedId;
			for (int tries = 0; tries < HASH_TABLE_SIZE; tries++)
			{
				if (m_buckets[bucket].used == false)
				{
					m_buckets[bucket].hashedId = hashedId;
					m_buckets[bucket].originalId = originalId;
					m_buckets[bucket].used = true;
					return;
				}
				bucket = (bucket + 1) % HASH_TABLE_SIZE;
			}
		}

		// Passes a string by reference so only 1 data structure is needed to store the
		// url instead of 2.
		bool search(int hashedId, string& originalId)
		{
			int bucket = hashedId;
			for (int tries = 0; tries < HASH_TABLE_SIZE; tries++)
			{
				if (m_buckets[bucket].used == false)
					return false;
				if (m_buckets[bucket].hashedId == hashedId)
				{
					// TODO: fix bug related to here. www.b.com does not print out
					originalId = m_buckets[bucket].originalId;
					return true;
				}

				bucket = (bucket + 1) % HASH_TABLE_SIZE;
			}
			return false;
		}

		// TODO: REMOVE THE FOLLOWING AFTER TESTING
		void printHashTable(int start, int end)
		{
			if (start < 0 || start >= HASH_TABLE_SIZE)
				return;
			if (end < start || end < 0 || end >= HASH_TABLE_SIZE)
				return;

			for (int i = start; i < end; i++)
			{
				std::cout << i << " "
					<< m_buckets[i].hashedId << " "
					<< m_buckets[i].originalId << " "
					<< m_buckets[i].used << std::endl;
			}
		}
	
	private:
		Bucket m_buckets[HASH_TABLE_SIZE];
	};

public:
	IndexerImpl();  // Constructor
	bool incorporate(string url, WordBag& wb);
	vector<UrlCount> getUrlCounts(string word);
	bool save(string filenameBase);
	bool load(string filenameBase);

	// TODO: REMOVE AFTER TESTING
	void testPrintHash(int a, int b)
	{
		m_hashTable.printHashTable(a, b);
	}

private:
	// Private methods
	int urlToId(string url);
	string idToUrl(int id);
	void associateHelperUrlIdTrees(string url, int id);
	int hashString(string& url);

	// Private data members
	ClosedHashTable m_hashTable;
	MyMap<string, int> m_urlToId;
	MyMap<int, string> m_idToUrl;
	MyMap<string, int> m_countHolder;  // Used to hold just one value: m_hashedMapCount to meet spec requirements
	int m_hashedMapCount;

	// The index (table) is a MyMap object of string (word) to a vector of UrlCount objects
	MyMap < string, std::vector<UrlCount> > m_index;

	// More space efficient version of m_index used to saving and loading
	MyMap < string, std::vector<HashedUrlCount> > m_indexHashed;
};

// SAVE AND LOAD OVERLOADED SUPPORT FUNCTION DECLARATIONS
void writeItem(ostream& stream, string s)
{
	stream << s << std::endl;
}

void writeItem(ostream& stream, int i)
{
	stream << i << std::endl;
}

void writeItem(ostream& stream, const HashedUrlCount& h)
{
	stream << h.hashedUrl << std::endl;
	stream << h.count << std::endl;
}

void writeItem(ostream& stream, const std::vector<HashedUrlCount>& vh)
{
	// Write the number of items in the vector so when loading a file we know how many lines
	// of text in the file are designated for the contents of the vector to be loaded.
	// *2 because each instance of a HashedUrlCount object has two values: hashedUrl and count.
	writeItem(stream, vh.size()*2);  // Calls int overload of writeItem

	// Write each item in the vector
	for (int i = 0; i < vh.size(); i++)
		writeItem(stream, vh[i]);
}

bool readItem(istream& stream, string& s)
{
	getline(stream, s);

	std::cerr << "String: " << s << std::endl;

	if (!stream)
	{
		std::cerr << "Error loading string" << std::endl;
		return false;
	}
	else
		return true;

}

bool readItem(istream& stream, int& i)
{
	std::string line;
	if (!getline(stream, line))
	{
		std::cerr << "Error loading int: line" << std::endl;
		return false;
	}

	std::cerr << "int: " << i << std::endl;

	// input stringstream text will be copy of line
	istringstream iss(line);

	iss >> i;
	if (!iss)
	{
		std::cerr << "Error loading int: istringstream" << std::endl;
		return false;
	}
	else 
		return true;
}

bool readItem(istream& stream, std::vector<HashedUrlCount>& vh)
{
	for (int i = 0; i < vh.size(); i++)
		std::cerr << vh[i].hashedUrl << " " << vh[i].count << std::endl;

	return true;
}

// DECLARATION OF METHODS FOR TEMPLATED INDEXER SAVE AND LOAD FUNCTIONS
std::string getFileExtension(std::string filename)
{
	std::string fileExtension;
	int fileSize = filename.size();
	for (int i = fileSize - 1; i >= 0; i--)
	{
		if (filename[i] == '.')
			break;

		// Concatenates the string backwards
		fileExtension += filename[i];
	}

	// Reverses the string extension
	for (int i = 0, k = fileExtension.size() - 1; i < fileExtension.size() / 2;
		i++, k--)
	{
		char tempChar = fileExtension[i];
		fileExtension[i] = fileExtension[k];
		fileExtension[k] = tempChar;
	}

	return fileExtension;
}

int getCountFromACFile(std::string acFile)
{
	ifstream stream(acFile);
	if (!stream)
	{
		std::cerr << "Error: Cannot retrieve count from " << acFile << std::endl;
		return -1;
	}

	int count;
	stream >> count;
	return count;
}

template <class KeyType, class ValueType>
bool saveMyMap(std::string filename, MyMap<KeyType, ValueType>& m)
{
	ofstream stream(filename);
	if (!stream)
	{
		std::cerr << "Error: Cannot create " << filename << std::endl;
		return false;
	}
	
	// The MyMap object methods of getting the first key and iterating next after next relies
	// on a Level-Order traveral of the binary search tree which guarantees that the order 
	// of data extracted into the save file and then later loaded will be the same.
	KeyType tempKey;
	ValueType* val = m.getFirst(tempKey);

	if (val == nullptr)
		return false;

	for (val; val != nullptr; val = m.getNext(tempKey))
	{
		// Get the extension of the file so we can use a different procedure for the .ac file
		// TODO: REFACTOR (Gets called everytime a file is saved)
		if (getFileExtension(filename) == "ac")
		{
			writeItem(stream, *val);
			break;
		}

		// See declarations of Save overload functions above
		writeItem(stream, tempKey);
		writeItem(stream, *val);
	}

	return true;
}

template <class KeyType, class ValueType>
bool loadMyMap(string filename, MyMap<KeyType, ValueType>& m)
{
	// Clear m of any associations it currently holds
	m.clear();

	// Create ifstream object to read from file
	ifstream stream(filename);
	if (!stream)
	{
		std::cerr << "Error: Cannot read from " << filename << std::endl;
		return false;
	}

	// Get the number of associations from the .ac save file if we're currently 
	// in a .uti or .itu file. Since this is a non-class public function, we don't have 
	// this data stored anywhere publicly accessible (we also don't want to use globals)
	int associationCount = 0;
	if (getFileExtension(filename) == "uti" || getFileExtension(filename) == "itu")
	{
		// Write the ac filename by rewriting the current filename
		std::string acFileName;
		for (int i = 0; i < filename.size(); i++)
		{
			if (filename[i] == '.')
			{
				acFileName += ".ac";
				break;
			}
			acFileName += filename[i];
		}
		associationCount = getCountFromACFile(acFileName);

		if (associationCount == -1)  // Error in retrieving count
			return false;
	}
	//std::cerr << associationCount  << std::endl;

	//KeyType tempKey;
	//ValueType tempVal;  
	//int vectorSize;

	std::string tempVal;
	//while (getline(stream, tempVal))
		//std::cerr << tempVal << std::endl;

	//if (!readItem(stream, tempVal))
		//return false;

	return true;
}

// INDEXERIMPL CLASS EXTERNALLY DEFINED METHODS
IndexerImpl::IndexerImpl()
{
	m_hashedMapCount = 0;
}

bool IndexerImpl::incorporate(string url, WordBag& wb)
{
	// First check if url has been previously incorporated and return false if it has
	int convertedId = urlToId(url);
	bool existingBucketCheck = m_hashTable.search(convertedId, url);

	if (existingBucketCheck == true)
		return false;

	// m_hashTable is used for quick access to hashed url values in roughly O(1) time
	m_hashTable.insert(convertedId, url);

	// Create/update urlToId, idToUrl, wordToIdCounts objects for saving and loading functions
	associateHelperUrlIdTrees(url, convertedId);

	// Update the index. If we reach this point, then the url has not previously been incorporated
	std::string tempWord;
	int count;

	// Indexer m_indexHashed object is updated with the contents of the WordBag (wb) object
	bool gotAWord = wb.getFirstWord(tempWord, count);

	HashedUrlCount tempHashedUrlCount;
	// [Deprecated] UrlCount tempUrlCount;

	while (gotAWord)
	{
		tempHashedUrlCount.count = count;
		tempHashedUrlCount.hashedUrl = convertedId;
		// [Deprecated] tempUrlCount.url = url;

		// A vector of type pointer to determine if a vector (UrlCount) of url to int association already exists
		std::vector<HashedUrlCount>* tempVector = m_indexHashed.find(tempWord);
		// [Deprecated] std::vector<UrlCount>* tempVector = m_index.find(tempWord);

		// If no association between url and int exists, simply use a non-pointer HashedUrlCount vector to hold the new
		// value and create the association
		if (tempVector == nullptr)
		{
			std::vector<HashedUrlCount> tempVectorNonPtr;
			tempVectorNonPtr.push_back(tempHashedUrlCount);
			m_indexHashed.associate(tempWord, tempVectorNonPtr);
		}

		// If an association DOES exists, use the pointer from above to update the EXISTING vector-type association
		else
		{
			tempVector->push_back(tempHashedUrlCount);
			m_indexHashed.associate(tempWord, *tempVector);
		}

		gotAWord = wb.getNextWord(tempWord, count);
	}

	return true;
}

vector<UrlCount> IndexerImpl::getUrlCounts(string word)
{
	// Passed in word is NOT case sensitive, and since all previously associated words have been converted to
	// lower case, the passed in word here must also be converted to lower case.
	strToLower(word);
	std::vector<HashedUrlCount>* temp = m_indexHashed.find(word);

	// If the result of find is nullptr, create an empty vector and return
	if (temp == nullptr)
	{
		std::vector<UrlCount> emptyVector;
		return emptyVector;
	}

	// Otherwise convert HashedUrlCount to UrlCount and return the pointer to the found vector
	std::vector<UrlCount> tempVector;
	UrlCount copiedVector;

	// TODO: Understand why I have to do the following. Couldn't get the pointer version to work
	std::vector <HashedUrlCount> copiedHashedVector = *temp;

	for (int i = 0; i < temp->size(); i++)
	{
		copiedVector.count = copiedHashedVector[i].count;
		copiedVector.url = idToUrl(copiedHashedVector[i].hashedUrl);
		tempVector.push_back(copiedVector);
	}

	return tempVector;
}

bool IndexerImpl::save(string filenameBase)
{
	// The following association is simply used to be able to pass the size of m_urlToId and m_itToUrl
	// to the public saveMyMap method which otherwise wouldn't have access to this private data.
	// This is done in order to meet spec requirements. Otherwise we could easily add a public method
	// to IndexderImpl.
	m_countHolder.associate("", m_hashedMapCount);

	return saveMyMap(filenameBase + ".ac", m_countHolder) &&	// .ac		= "association count"
		saveMyMap(filenameBase + ".uti", m_urlToId) &&			// .uti		= "url to id"
		saveMyMap(filenameBase + ".itu", m_idToUrl) &&			// .itu		= "id to url"
		saveMyMap(filenameBase + ".wtic", m_indexHashed);		// .wtic	= "word to id counts"
}

bool IndexerImpl::load(string filenameBase)
{
	return //loadMyMap(filenameBase + ".ac", m_countHolder) &&
		loadMyMap(filenameBase + ".uti", m_urlToId) &&
		loadMyMap(filenameBase + ".itu", m_idToUrl) &&
		loadMyMap(filenameBase + ".wtic", m_indexHashed);
}

int IndexerImpl::urlToId(string url)
{
	int hashedUrl = hashString(url);
	return hashedUrl;
}

string IndexerImpl::idToUrl(int id)
{
	// Hash table search function returns a string by reference.
	std::string originalUrl;
	m_hashTable.search(id, originalUrl);
	return originalUrl;
}

void IndexerImpl::associateHelperUrlIdTrees(string url, int id)
{
	m_urlToId.associate(url, id);
	m_idToUrl.associate(id, url);

	// Update the size count for use in the save and load functions
	m_hashedMapCount++;
}

int IndexerImpl::hashString(string& url)
{
	int total = 0;

	for (int i = 0; i < url.length(); i++)
		total = total + (i + 1) * url[i];

	total = total % HASH_TABLE_SIZE;

	return total;
}

//******************** Indexer functions *******************************

// These functions simply delegate to IndexerImpl's functions.
// You probably don't want to change any of this code.

Indexer::Indexer()
{
	m_impl = new IndexerImpl;
}

Indexer::~Indexer()
{
	delete m_impl;
}

bool Indexer::incorporate(string url, WordBag& wb)
{
	return m_impl->incorporate(url, wb);
}

vector<UrlCount> Indexer::getUrlCounts(string word)
{
	return m_impl->getUrlCounts(word);
}

bool Indexer::save(string filenameBase)
{
	return m_impl->save(filenameBase);
}

bool Indexer::load(string filenameBase)
{
	return m_impl->load(filenameBase);
}
