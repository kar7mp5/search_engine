# crawler/views.py (full updated version to fix schema 500 error)
from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework import status
from rest_framework.renderers import BaseRenderer
from django.http import HttpResponse

from drf_spectacular.utils import extend_schema, OpenApiTypes, OpenApiExample
from .scrap_website import ScrapWebsite
from .serializers import CrawlRequestSerializer

class CSVRenderer(BaseRenderer):
    media_type = 'text/csv'
    format = 'csv'
    charset = 'utf-8'

    def render(self, data, accepted_media_type=None, renderer_context=None):
        # This won't be used since we return HttpResponse, but good to have
        return '\n'.join(data) + '\n'

class CrawlView(APIView):
    renderer_classes = [CSVRenderer]

    @extend_schema(
        summary="Crawl URLs and return discovered base URLs as CSV",
        description="Provide starting URLs, depth, and threads. Returns a CSV file download on success.",
        request=CrawlRequestSerializer,
        responses={
            (200, 'text/csv'): OpenApiTypes.STR,  # For text content like CSV
            400: OpenApiTypes.OBJECT,
            500: OpenApiTypes.OBJECT,
        },
        examples=[
            OpenApiExample(
                name="CSV Download Example",
                description="Sample CSV content",
                value="https://example.com/\nhttps://another.com/\n",
                media_type="text/csv"
            )
        ]
    )
    def post(self, request):
        serializer = CrawlRequestSerializer(data=request.data)
        if not serializer.is_valid():
            return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)
        
        start_urls = serializer.validated_data['start_urls']
        max_depth = serializer.validated_data['max_depth']
        num_threads = serializer.validated_data['num_threads']

        engine = ScrapWebsite()
        try:
            found_bases = engine.crawl(start_urls, max_depth, num_threads)
        except Exception as e:
            return Response(
                {"error": f"Crawling failed: {str(e)}"},
                status=status.HTTP_500_INTERNAL_SERVER_ERROR
            )

        if not found_bases:
            return Response(
                {"message": "No base URLs discovered", "urls": []},
                status=status.HTTP_200_OK
            )

        # Generate CSV content in-memory
        csv_content = '\n'.join(found_bases) + '\n'

        return HttpResponse(
            csv_content,
            content_type='text/csv',
            headers={'Content-Disposition': 'attachment; filename="discovered_base_urls.csv"'}
        )


# Optional simple view
from django.http import HttpResponse

def members(request):
    return HttpResponse('Hello World!')