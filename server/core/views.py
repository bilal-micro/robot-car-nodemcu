from channels.layers import get_channel_layer
from django.http import HttpResponse
from asgiref.sync import async_to_sync
from django.views.decorators.csrf import csrf_exempt

@csrf_exempt
def update_x_Y(request , data):
    i = str(data).split(',')
    x = float(i[0])
    y = float(i[1])
    channel_layer = get_channel_layer()
    async_to_sync(channel_layer.group_send)(
        f'main',
        {
            "type": "control",
            'state' : 'sendTo',
            "message": f'{x},{y}',
        }
    )
    return HttpResponse()