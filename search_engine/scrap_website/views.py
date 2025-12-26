from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework import status
from rest_framework.permissions import AllowAny
from django.http import HttpResponse, JsonResponse

from .scrap_website import ScrapWebsite
from .serializers import CrawlRequestSerializer


class CrawlView(APIView):

    permission_classes = [AllowAny]

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
       
        data = {
            'base urls': found_bases
        }
        
        return Response(
            {"message": "Success for crawling",
             "base_urls": found_bases or []
             })


from django.http import HttpResponse

def members(request):
    return HttpResponse('Hello World!')