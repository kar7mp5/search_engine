from django.urls import path
from . import views
from .views import CrawlView

urlpatterns = [
    path('scrap_website/', views.members, name='scrap_website'),
    path('crawl/', CrawlView.as_view(), name='crawl'),
]