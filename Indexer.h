#ifndef INDEXER_INCLUDED
#define INDEXER_INCLUDED

#include "provided.h"
#include "MyMap.h"
#include <string>
#include <fstream>  // for save and load
#include <sstream>  // for istringstream


// Load factor of 10% assuming 1000 values (hashed url strings) to insert
// 10,007 is the closest prime number after 10,000 (for more equal distribution)
static const int HASH_TABLE_SIZE = 10007;

// Similar to the UrlCount struct in provided.h but this version potentially saves
// much more space by using converted lengthy url strings to smaller ints
struct HashedUrlCount
{
	int hashedUrl;
	int count;
};

class ClosedHashTable
{
private:
	struct Bucket
	{
		// Default false used value
		Bucket()
		{ 
			used = false; 
			hashedId = 0;
		}
		int hashedId;
		std::string originalId;
		bool used;
	};

public:
	void insert(int hashedId, std::string originalId)
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

	bool search(int hashedId, std::string& originalId)
	{
		int bucket = hashedId;
		for (int tries = 0; tries < HASH_TABLE_SIZE; tries++)
		{
			if (m_buckets[bucket].used == false)
				return false;
	
			if (m_buckets[bucket].hashedId == hashedId)
			{
				// Returns the original url by reference
				originalId = m_buckets[bucket].originalId;
				return true;
			}

			bucket = (bucket + 1) % HASH_TABLE_SIZE;
		}
		return false;
	}

	int hashString(std::string& url)
	{
		int total = 0;

		for (int i = 0; i < url.length(); i++)
			total = total + (i + 1) * url[i];

		total = total % HASH_TABLE_SIZE;

		return total;
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

class IndexerImpl
{
public:
	IndexerImpl();  // Constructor
	bool incorporate(std::string url, WordBag& wb);
	std::vector<UrlCount> getUrlCounts(std::string word);
	bool save(std::string filenameBase);
	bool load(std::string filenameBase);

	// TODO: REMOVE AFTER TESTING
	void testPrintHash(int a, int b)
	{
		m_hashTable.printHashTable(a, b);
	}

private:
	// Private methods
	int urlToId(std::string url);
	std::string idToUrl(int id);
	void associateHelperUrlIdTrees(std::string url, int id);
	int hashString(std::string& url);

	// Private data members
	ClosedHashTable m_hashTable;
	MyMap<std::string, int> m_urlToId;
	MyMap<int, std::string> m_idToUrl;
	MyMap<std::string, int> m_countHolder;  // Used to hold just one value: m_hashedMapCount to meet spec requirements
	int m_hashedMapCount;

	// The index (table) is a MyMap object of string (word) to a vector of UrlCount objects
	MyMap < std::string, std::vector<UrlCount> > m_index;

	// More space efficient version of m_index used for saving and loading
	MyMap < std::string, std::vector<HashedUrlCount> > m_indexHashed;
};

// TEMPLATE FUNCTIONS 
template <class KeyType, class ValueType>
bool loadHashTable(MyMap<KeyType, ValueType>& m, ClosedHashTable& cht)
{
	// MyMap <key, value> type of <Url to Id> (ex. "www.a.com" -> 4221)
	KeyType tempKey;
	ValueType* tempValue = m.getFirst(tempKey);

	if (tempValue == nullptr)
		return false;

	while (tempValue != nullptr)
	{
		cht.insert(*tempValue, tempKey);
		tempValue = m.getNext(tempKey);
	}

	return true;
}

inline void writeItem(std::ostream& stream, std::string s)
{
	stream << s << std::endl;
}

inline void writeItem(std::ostream& stream, int i)
{
	stream << i << std::endl;
}

inline void writeItem(std::ostream& stream, const HashedUrlCount& h)
{
	stream << h.hashedUrl << std::endl;
	stream << h.count << std::endl;
}

inline void writeItem(std::ostream& stream, const std::vector<HashedUrlCount>& vh)
{
	// Write the number of items in the vector so when loading a file we know how many lines
	// of text in the file are designated for the contents of the vector to be loaded.
	// *2 because each instance of a HashedUrlCount object has two values: hashedUrl and count.
	writeItem(stream, vh.size() * 2);  // Calls int overload of writeItem

	// Write each item in the vector
	for (int i = 0; i < vh.size(); i++)
		writeItem(stream, vh[i]);
}

inline bool readItem(std::istream& stream, std::string& s)
{
	std::getline(stream, s);

	//std::cerr << "String: " << s << std::endl;

	if (!stream)
	{
		/* std::cerr << "LINE " << __LINE__ << ": If index NOT loaded succesfully, then error. Otherwise "
		<< "end of stream reached." << std::endl; */
		return false;
	}
	else
		return true;

}

inline bool readItem(std::istream& stream, int& i)
{
	std::string line;
	if (!getline(stream, line))
	{
		std::cerr << "LINE " << __LINE__ << ": Error loading int/string "
			<< __FILE__ << std::endl;
		return false;
	}

	// input stringstream text will be copy of line
	std::istringstream iss(line);

	iss >> i;
	//std::cerr << "int: " << i << std::endl;
	if (!iss)
	{
		std::cerr << "LINE " << __LINE__ << ": Error loading istringstream "
			<< __FILE__ << std::endl;
		return false;
	}
	else
		return true;
}

template <class KeyType, class ValueType>
bool saveMyMap(std::string filename, MyMap<KeyType, ValueType>& m)
{
	std::ofstream stream(filename);
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

inline std::string getFileExtension(std::string filename)
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

inline int getCountFromACFile(std::string acFile)
{
	std::ifstream stream(acFile);
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
void fillInUtiItuFile(std::istream& stream, int count, MyMap<KeyType, ValueType>& m)
{
	KeyType key;
	ValueType value;

	for (int i = 0; i < count; i++)
	{
		readItem(stream, key);
		readItem(stream, value);
		m.associate(key, value);
	}
}

inline void fillLoadingVector(std::istream& stream, MyMap<std::string, std::vector<HashedUrlCount> >& m, int count,
	std::vector<HashedUrlCount>& vec)
{
	// Clear vector of prior values
	vec.clear();

	// Contents of a HashedUrlCount object
	HashedUrlCount temp;
	int tempHashedUrl;
	int tempCount;

	for (int i = 0; i < count / 2; i++)
	{
		readItem(stream, tempHashedUrl);
		readItem(stream, tempCount);
		//std::cerr << "tempHashedUrl: " << tempHashedUrl << std::endl;
		//std::cerr << "tempCount: " << tempCount << std::endl;
		temp.hashedUrl = tempHashedUrl;
		temp.count = tempCount;
		vec.push_back(temp);
	}


}

inline void fillInWticFile(std::istream& stream, MyMap<std::string, std::vector<HashedUrlCount> >& m)
{
	std::string tempWord;
	std::vector<HashedUrlCount> tempVec;
	int tempVectorCount;
	int counter = 0;

	// TODO: FIX BUG HERE (PROBABLY HAS TO DO WITH TEMPLATED FUNCTIONS BEING IN THIS FILE
	// CURRENTLY WORKS WITH THIS WORK-AROUND FUNCTION AND fillLoadingVector ^^above

	for (;;)
	{
		if (counter % 2 != 0)  // Odd numbers
		{
			if (!readItem(stream, tempVectorCount))
				break;

			fillLoadingVector(stream, m, tempVectorCount, tempVec);
		}
		else
		{
			if (!readItem(stream, tempWord))
				break;
		}
		m.associate(tempWord, tempVec);
		counter++;
	}
}

template <class KeyType, class ValueType>
bool loadMyMap(std::string filename, MyMap<KeyType, ValueType>& m)
{
	// Clear m of any associations it currently holds
	m.clear();

	// Create ifstream object to read from file
	std::ifstream stream(filename);
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

		fillInUtiItuFile(stream, associationCount, m);

		return true;
	}

	//else if (getFileExtension(filename) == "wtic")
	//{

	//fillInWticFile(stream, m);
	//return true;
	//}

	else
	{
		std::cerr << "Error: unrecognized file extension" << std::endl;
		return false;
	}
}

inline bool loadWtic(std::string filename, MyMap<std::string, std::vector<HashedUrlCount> >& m)
{
	std::ifstream stream(filename);
	if (!stream)
	{
		std::cerr << "Error: Cannot read from " << filename << std::endl;
		return false;
	}

	if (getFileExtension(filename) == "wtic")
	{

		fillInWticFile(stream, m);
		return true;
	}
	else
		return false;
}


#endif  // INDEXER_INCLUDED