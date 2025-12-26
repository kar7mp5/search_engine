# crawler/serializers.py
from rest_framework import serializers

class CrawlRequestSerializer(serializers.Serializer):
    start_urls = serializers.ListField(
        child=serializers.URLField(),
        required=True,
        help_text="List of starting URLs to crawl (non-empty)."
    )
    max_depth = serializers.IntegerField(
        default=1,
        min_value=0,
        help_text="Maximum recursion depth for crawling."
    )
    num_threads = serializers.IntegerField(
        default=4,
        min_value=1,
        help_text="Number of threads for multi-threaded crawling."
    )