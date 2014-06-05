#include "provided.h"
#include <string>
#include <list>


class WebCrawlerImpl
{
public:
	WebCrawlerImpl();
	void addUrl(std::string url);
	int getNumberOfUrls() const;
	void crawl(void(*callback)(std::string url, bool success));
	bool save(std::string filenameBase);
	bool load(std::string filenameBase);

private:
	Indexer m_webCrawlerIndex;
	int m_numberOfUrls;
	std::list<std::string> m_storedUrls;

};

WebCrawlerImpl::WebCrawlerImpl()
{
	m_numberOfUrls = 0;
}

void WebCrawlerImpl::addUrl(std::string url)
{
	// Stores URL but does not crawl. See crawl function
	m_storedUrls.push_front(url);  // Push front so same order when crawled
	m_numberOfUrls++;
}

int WebCrawlerImpl::getNumberOfUrls() const
{
	return m_numberOfUrls;
}

void WebCrawlerImpl::crawl(void(*callback)(std::string url, bool success))
{
	// Step 1. Connect to website and download web page at specific URL
	// Step 2. If download successful, place website in WordBag object,
	//		   Incorporate WordBag into WebCrawler's Indexer object
	// Step 3. Call a callback function provided by the user via a function
	//		   pointer, to report the status of the web page download and
	//		   incorporation into the index.
	std::string page, url;
	bool success;

	while (!m_storedUrls.empty())
	{
		url = m_storedUrls.back();
		m_storedUrls.pop_back();

		// Step 1
		if (HTTP().get(url, page))
		{
			// Step 2
			WordBag wb(page);
			m_webCrawlerIndex.incorporate(url, wb);

			// TODO: REMOVE AFTER TESTING
			/*std::string word;
			int count;
			bool gotAWord = wb.getFirstWord(word, count);

			while (gotAWord)
			{
				std::cerr << "The word " << word << " occurs " << count
					<< " times" << std::endl;
				gotAWord = wb.getNextWord(word, count);
			}*/

			success = true;
		}
		else
			success = false;

		// Step 3
		callback(url, success);
	}
}

bool WebCrawlerImpl::save(std::string filenameBase)
{
	return m_webCrawlerIndex.save(filenameBase);
}

bool WebCrawlerImpl::load(std::string filenameBase)
{
	return m_webCrawlerIndex.load(filenameBase);
}

//******************** WebCrawler functions *******************************

// These functions simply delegate to WebCrawlerImpl's functions.
// You probably don't want to change any of this code.

WebCrawler::WebCrawler()
{
	m_impl = new WebCrawlerImpl;
}

WebCrawler::~WebCrawler()
{
	delete m_impl;
}

void WebCrawler::addUrl(std::string url)
{
	m_impl->addUrl(url);
}

int WebCrawler::getNumberOfUrls() const
{
	return m_impl->getNumberOfUrls();
}

void WebCrawler::crawl(void(*callback)(std::string url, bool success))
{
	m_impl->crawl(callback);
}

bool WebCrawler::save(std::string filenameBase)
{
	return m_impl->save(filenameBase);
}

bool WebCrawler::load(std::string filenameBase)
{
	return m_impl->load(filenameBase);
}
