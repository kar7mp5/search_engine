from search_engine import SearchEngine


def main():
    search_engine = SearchEngine()
    seed_url = ['http://quotes.toscrape.com', 'https://kar7mp5.github.io']
    urls = search_engine.crawl(seed_url, max_depth=2)

    search_engine.save_to_csv(urls, './urls.csv')
    print(urls)


if __name__ == '__main__':
    main()