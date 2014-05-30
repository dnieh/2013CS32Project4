#include "Indexer.h"


IndexerImpl::IndexerImpl()
{
	m_hashedMapCount = 0;
}

bool IndexerImpl::incorporate(std::string url, WordBag& wb)
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

	while (gotAWord)
	{
		tempHashedUrlCount.count = count;
		tempHashedUrlCount.hashedUrl = convertedId;

		// A vector of type pointer to determine if a vector (UrlCount) of url to int association already exists
		std::vector<HashedUrlCount>* tempVector = m_indexHashed.find(tempWord);

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

std::vector<UrlCount> IndexerImpl::getUrlCounts(std::string word)
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

bool IndexerImpl::save(std::string filenameBase)
{
	// The following association is simply used to be able to pass the size of m_urlToId and m_itToUrl
	// to the public saveMyMap method which otherwise wouldn't have access to this private data.
	// This is done in order to meet spec requirements. Otherwise we could easily add a public method
	// to IndexderImpl to retrieve this value.
	m_countHolder.associate("", m_hashedMapCount);

	return saveMyMap(filenameBase + ".ac", m_countHolder) &&	// .ac		= "association count"
		saveMyMap(filenameBase + ".uti", m_urlToId) &&			// .uti		= "url to id"
		saveMyMap(filenameBase + ".itu", m_idToUrl) &&			// .itu		= "id to url"
		saveMyMap(filenameBase + ".wtic", m_indexHashed);		// .wtic	= "word to id counts"
}

bool IndexerImpl::load(std::string filenameBase)
{
	// Must refill the hash table
	ClosedHashTable hashTableCopy;

	bool loadCheck = (
		loadMyMap(filenameBase + ".uti", m_urlToId) &&
		loadMyMap(filenameBase + ".itu", m_idToUrl) &&
		loadWtic(filenameBase + ".wtic", m_indexHashed) &&
		loadHashTable(m_urlToId, hashTableCopy) );

	if (!loadCheck)
		return false;

	// Default assignment operator
	m_hashTable = hashTableCopy;

	return true;
}

int IndexerImpl::urlToId(std::string url)
{
	int hashedUrl = m_hashTable.hashString(url);
	return hashedUrl;
}

std::string IndexerImpl::idToUrl(int id)
{
	// Hash table search function returns a string by reference.
	std::string originalUrl;
	m_hashTable.search(id, originalUrl);
	return originalUrl;
}

void IndexerImpl::associateHelperUrlIdTrees(std::string url, int id)
{
	m_urlToId.associate(url, id);
	m_idToUrl.associate(id, url);

	// Update the size count for use in the save and load functions.
	m_hashedMapCount++;
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

bool Indexer::incorporate(std::string url, WordBag& wb)
{
	return m_impl->incorporate(url, wb);
}

std::vector<UrlCount> Indexer::getUrlCounts(std::string word)
{
	return m_impl->getUrlCounts(word);
}

bool Indexer::save(std::string filenameBase)
{
	return m_impl->save(filenameBase);
}

bool Indexer::load(std::string filenameBase)
{
	return m_impl->load(filenameBase);
}
