from scrap_website import ScrapWebsite


def main():
    scrapper = ScrapWebsite()
    seed_url = ['http://quotes.toscrape.com', 'https://kar7mp5.github.io']
    urls = scrapper.crawl(seed_url, max_depth=2)

    scrapper.save_to_csv(urls, './urls.csv')
    print(urls)


if __name__ == '__main__':
    main()