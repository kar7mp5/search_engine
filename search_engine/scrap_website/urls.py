from django.urls import path
from . import views

urlpatterns = [
    path('scrap_website/', views.members, name='scrap_website')
]