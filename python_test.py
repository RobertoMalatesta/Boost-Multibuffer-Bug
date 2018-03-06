#!/usr/bin/python3
import sys, os
import websockets
import asyncio
import time


async def main():
    
    increase_t0 = time.time()

    try:
        async with websockets.connect('ws://127.0.0.1:5500') as websocket:
        
            li = []

            for i in range(50):
            
                li.append(i)

            while 1:

                await websocket.send(bytes(li))
                response = await websocket.recv()
                if response != bytes(li):
                    print("Error")
                    exit(-1)
                time.sleep(1)

        increase_t1 = time.time()
        total = increase_t1 - increase_t0
        print("Increase Test Time: " + str(total) + " seconds.")

    except Exception as e:
        print("BREAK! : " + str(e))

asyncio.get_event_loop().run_until_complete(main())