import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin, urlparse
from concurrent.futures import ThreadPoolExecutor


class SearchEngine:

    def __init__(self):
        # A set to keep track of visited URLs to avoid infinite loops
        self.visited_urls = set()

    def crawl(self, url, max_depth=1):
        """Crawl website

        Args:
            url (str): web url
            max_depth (int): recursive max limit. Defaults to 1.
        """
        # Check depth
        if url in self.visited_urls or max_depth == 0:
            return
        
        print(f"Crawling: {url}")
        self.visited_urls.add(url)

        try:
            # Send a GET request to the URL
            response = requests.get(url, timeout=5)
            response.raise_for_status() # Raise an exception for bad status codes (4xx or 5xx)

            # Parse the HTML content of the page
            # TODO: Change html.parser to lxml
            soup = BeautifulSoup(response.content, 'html.parser')

            # Find all link tags (<a>)
            for link in soup.find_all('a'):
                href = link.get('href')
                if href:
                    # Create an absolute URL by joining the base URL with the relative link
                    absolute_url = urljoin(url, href)
                    # Only crawl links within the same domain
                    if urlparse(absolute_url).netloc == urlparse(url).netloc:
                        self.crawl(absolute_url, max_depth - 1)

        except requests.exceptions.RequestException as e:
            print(f"Error fetching {url}: {e}")
 

    def multi_thread_crawl(self, urls):
        pass              
"""
from concurrent.futures import ThreadPoolExecutor

def worker(task):
    print(f"Task {task} running")

# Create a thread pool with 2 workers
with ThreadPoolExecutor(max_workers=2) as executor:
    # Submit two tasks to run in parallel
    executor.submit(worker, 1)
    executor.submit(worker, 2)
"""

def main():
    search_engine = SearchEngine()
    seed_url = 'http://quotes.toscrape.com'
    search_engine.crawl(seed_url, max_depth=2)


if __name__ == '__main__':
    main()