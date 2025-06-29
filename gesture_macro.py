import asyncio
import pyautogui as p
from bleak import BleakClient

address = "C8:29:C0:F8:CC:12"
characteristic_uuid = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
posi = 378

def notification_handler(sender, data):
    s = data.decode("utf-8")
    print(s)
    global posi
    if s == 'r':
        posi += 15
        if posi == 438: posi = 378
        p.click(posi, 408)
    if s == 'l':
        posi -= 15
        if posi == 363: posi = 423
        p.click(posi, 430)

async def run(address):
    async with BleakClient(address) as client:
        print(f"Connected: {client.is_connected}")
        await client.start_notify(characteristic_uuid, notification_handler)
        #print("Started notify")
        try:
            while True:
                await asyncio.sleep(1)
        except KeyboardInterrupt:
            print("stopping notifications")
        await client.stop_notify(characteristic_uuid)
        #print("stopped notify")

loop = asyncio.get_event_loop()
while True:
    loop.run_until_complete(run(address))