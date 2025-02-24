import json
from channels.generic.websocket import AsyncWebsocketConsumer
import requests

class MainConsumer(AsyncWebsocketConsumer):
    async def connect(self):
        self.room_name = f'main'
        await self.channel_layer.group_add(self.room_name , self.channel_name)
        await self.accept()

    async def disconnect(self, close_code):
        await self.channel_layer.group_discard(self.room_name, self.channel_name)
        
    async def receive(self, text_data):
        text_data_json = json.loads(text_data)
        print(text_data)
        await self.send(text_data)
            
   