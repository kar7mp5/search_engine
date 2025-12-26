import queue
import threading

import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin, urlparse

import pandas as pd

from .custom_logger import Logger
import logging


class ScrapWebsite:

    def __init__(self):
        # Initialize CustomLogger 
        logging.setLoggerClass(Logger)
        self.logger = logging.getLogger(__name__)

        # A set to keep track of visited URLs to avoid infinite loops
        self.visited_urls = set()
        self.lock = threading.Lock()  # Lock for thread-safe access to visited_urls
        self.discovered_bases = set()


    def crawl(self, start_urls: list[str], max_depth: int=1, num_threads: int=4) -> list[str]:
        """Crawl websites using multi-threading, handling multiple starting URLs

        Args:
            start_urls (list[str]): list of starting web urls
            max_depth (int): recursive max limit. Defaults to 1.
            num_threads (int): number of threads to use. Defaults to 4.
        
        Returns:
            list[str]: crawl url list
        """
        if not start_urls:
            print("No starting URLs provided.")
            return

        allowed_domains: set[str] = {urlparse(url).netloc for url in start_urls}
        
        # Initialize discovered bases with start URLs' bases
        self.discovered_bases = set()
        for url in start_urls:
            parsed = urlparse(url)
            if parsed.scheme in ('http', 'https'):
                base = parsed.scheme + '://' + parsed.netloc + '/'
                self.discovered_bases.add(base)
        
        task_queue = queue.Queue()
        for url in start_urls:
            task_queue.put((url, max_depth))

        def worker():
            while True:
                try:
                    url, depth = task_queue.get(timeout=1)
                except queue.Empty:
                    break

                with self.lock:
                    if url in self.visited_urls or depth == 0:
                        task_queue.task_done()
                        continue
                    self.visited_urls.add(url)

                self.logger.info(f"Crawling: {url} (depth: {depth})")

                try:
                    response = requests.get(url, timeout=5)
                    response.raise_for_status()

                    soup = BeautifulSoup(response.content, 'lxml')  # Changed to lxml for better performance

                    for link in soup.find_all('a'):
                        href = link.get('href')
                        if href:
                            absolute_url = urljoin(url, href)
                            parsed = urlparse(absolute_url)
                            if parsed.scheme in ('http', 'https'):
                                base = parsed.scheme + '://' + parsed.netloc + '/'
                                with self.lock:
                                    self.discovered_bases.add(base)
                            if parsed.netloc in allowed_domains:
                                task_queue.put((absolute_url, depth - 1))

                except requests.exceptions.RequestException as e:
                    print(f"Error fetching {url}: {e}")

                task_queue.task_done()

        threads = []
        for _ in range(num_threads):
            t = threading.Thread(target=worker)
            t.start()
            threads.append(t)

        task_queue.join()

        for t in threads:
            t.join()
        
        return list(self.discovered_bases)
    
    
    def save_to_csv(self, urls: list[str], file_path: str):
        """Save data to csv file

        Args:
            urls (list[str]): urls to save
            file_path (str): file path to save
        """
        self.logger.info(f'Save to {file_path}')
        ser = pd.Series(urls)
        ser.to_csv(file_path, index=False)