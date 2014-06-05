#include "provided.h"
#include <string>
using namespace std;


// Used for sorting through search results
struct urlSearchResults
{
	string url;
	int occurences;
	int score;
};

bool urlSearchSortFunction(const urlSearchResults &target, urlSearchResults &src)
{
	return target.score > src.score;
}

class SearcherImpl
{
public:
	vector<string> search(string terms);
	bool load(string filenameBase);

private:
	
	Indexer m_searcherIndex;
	vector<string> m_searchMatches;
	vector<string> m_searchTerms;
	vector<UrlCount> m_urlCountContainer;
	vector<urlSearchResults> m_unsortedSearchResults;
};

vector<string> SearcherImpl::search(string terms)
{
	// Clear out vectors of anything they may have contained
	m_searchMatches.clear();
	m_searchTerms.clear();
	m_unsortedSearchResults.clear();
	m_urlCountContainer.clear();

	// Search terms are NOT case sensitive and can be more than one word so parse out
	// Also treat word repetition as just a single word
	std::transform(terms.begin(), terms.end(), terms.begin(), ::tolower);
	Tokenizer t(terms);
	std::string tempString;

	while (t.getNextToken(tempString))
	{
		if (m_searchTerms.size() == 0)
			m_searchTerms.push_back(tempString);

		else
		{
			for (unsigned int i = 0; i < m_searchTerms.size(); i++)
			{
				// Checks for duplicates
				if (m_searchTerms[i] == tempString)
					break;

				// If on last iteration and no match, then there are no duplicates
				if (i == m_searchTerms.size() - 1)
					m_searchTerms.push_back(tempString);
			}
		}
	}

	// A given web page matches a search query if at least T of the N distinct items found in that page:
	// N = number of distinct words in ther terms string
	// T = int(0.7*N) or 1, whichever is larger
	int N = m_searchTerms.size();
	int T;

	if (N == 1)
		T = 1;
	else if (N > 1)
		T = N * 0.7;
	else
		return m_searchMatches;
	std::cerr << "N: " << N << std::endl;
	std::cerr << "T: " << T << std::endl;

	// Results must be returned in order of greatest relevance
	// Relevance score = add up occurences of term per page per term
	// Ex. "Edward Snowden security news": 
	//		"Edward" appears on www.a.com 5 times
	//		"Snowden" appears on www.a.com 2 times
	//		"news" appears on www.a.com 1 times
	//		"Snowden" appears on www.b.com 4 times
	//		"security" appears on www.b.com 8 times
	//
	//		www.b.com would have a score of 12 (8 + 4) and have a greater
	//		relevance than www.a.com which has a score of 8 (5 + 2 + 1).

	urlSearchResults tempSearchResults;
	UrlCount tempUrlCountVar;
	vector<UrlCount> tempUrlCountContainer;

	for (unsigned int i = 0; i < N; i++)
	{
		tempUrlCountContainer = m_searcherIndex.getUrlCounts(m_searchTerms[i]);

		for (unsigned int k = 0; k < tempUrlCountContainer.size(); k++)
		{
			// TODO: REMOVE AFTER TESTING
			std::cerr << m_searchTerms[i] << " appears in " << tempUrlCountContainer[k].count
				<< " times at " << tempUrlCountContainer[k].url << std::endl;

			// Transfer over the temp variables over to m_urlCountContainer
			tempUrlCountVar.count = tempUrlCountContainer[k].count;
			tempUrlCountVar.url = tempUrlCountContainer[k].url;
			m_urlCountContainer.push_back(tempUrlCountVar);
		}

		//std::cerr << "TEST SIZE: " << tempUrlCountContainer.size() << std::endl;
	}



	for (int k = 0; k < m_urlCountContainer.size(); k++)
	{

			// We're basically consolidating the urlCounts vector into m_unsortedSearchResults
			// Then afterwards we'll sort it and copy the valid contents into m_searchMatches
			if (k == 0)
			{
				tempSearchResults.url = m_urlCountContainer[k].url;
				tempSearchResults.score = m_urlCountContainer[k].count;
				tempSearchResults.occurences = 1;
				m_unsortedSearchResults.push_back(tempSearchResults);
			}

			else
			{
				for (unsigned int g = 0; g < m_unsortedSearchResults.size(); g++)
				{
					// Check for existing url and update info if existing found
					if (m_unsortedSearchResults[g].url == m_urlCountContainer[k].url)
					{
						m_unsortedSearchResults[g].score += m_urlCountContainer[k].count;
						m_unsortedSearchResults[g].occurences++;
						break;
					}

					// If we're at the end of the loop and previous condition not met, then push the new value
					if (g == m_unsortedSearchResults.size() - 1)
					{
						tempSearchResults.url = m_urlCountContainer[k].url;
						tempSearchResults.score = m_urlCountContainer[k].count;
						tempSearchResults.occurences = 1;
						m_unsortedSearchResults.push_back(tempSearchResults);
						break;  // TODO: Understand why I need this break here. Bug otherwise
					}
				}
			}
		}

	// TODO: REMOVE AFTER TESTING
	/*for (int q = 0; q < m_unsortedSearchResults.size(); q++)
		std::cerr << m_unsortedSearchResults[q].url << " "
			<< m_unsortedSearchResults[q].occurences << " "
			<< m_unsortedSearchResults[q].score << std::endl;*/

	// Sort m_unsortedSearchResults based on score (See urlSearchSortFunction at beginning of file)
	std::cerr << "Sort function initializing" << std::endl;
	std::sort(m_unsortedSearchResults.begin(), m_unsortedSearchResults.end(), urlSearchSortFunction);

	// TODO: REMOVE AFTER TESTING
	for (int q = 0; q < m_unsortedSearchResults.size(); q++)
		std::cerr << m_unsortedSearchResults[q].url << " "
		<< m_unsortedSearchResults[q].occurences << " "
		<< m_unsortedSearchResults[q].score << std::endl;

	// Iterate through now sorted m_unsortedSearchResults and push valid values into m_searchMatches
	string tempUrl;
	for (unsigned int z = 0; z < m_unsortedSearchResults.size(); z++)
	{
		if (m_unsortedSearchResults[z].occurences >= T)
		{
			tempUrl = m_unsortedSearchResults[z].url;
			m_searchMatches.push_back(tempUrl);
		}
	}
	
	return m_searchMatches; 
}

bool SearcherImpl::load(string filenameBase)
{
	return m_searcherIndex.load(filenameBase);
}

//******************** Searcher functions *******************************

// These functions simply delegate to SearcherImpl's functions.
// You probably don't want to change any of this code.

Searcher::Searcher()
{
	m_impl = new SearcherImpl;
}

Searcher::~Searcher()
{
	delete m_impl;
}

vector<string> Searcher::search(string terms)
{
	return m_impl->search(terms);
}

bool Searcher::load(string filenameBase)
{
	return m_impl->load(filenameBase);
}
