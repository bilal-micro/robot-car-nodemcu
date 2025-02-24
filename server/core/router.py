from django.urls import path
from .channels import MainConsumer

websocket_urlpatterns = [
    path('ws/', MainConsumer.as_asgi()),
]