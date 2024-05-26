# pip install websockets

import asyncio

import websockets


async def test_websocket():
    uri = "ws://192.168.0.113:3000"

    async with websockets.connect(uri) as websocket:
        # Send a message
        await websocket.send("Hello, WebSocket!")
        print("Message sent to the server")

        # Receive a message
        response = await websocket.recv()
        print(f"Message received from the server: {response}")


asyncio.get_event_loop().run_until_complete(test_websocket())
