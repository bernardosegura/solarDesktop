import asyncio
import websockets
import sys
from pathlib import Path
import os
import json

async def handler():
    async with websockets.connect('ws://localhost:' + puerto, origin="." + sys.argv[1] + ".rcmSolar") as websocket:
        await websocket.send(sys.argv[2])

def main():
    try:
        if(len(sys.argv) == 3):
            file = open(os.path.join(Path.home(), '.config','Solar_eDEX','settings.json'))
            data = ""
            for line in file:
                data += line
            settings = json.loads(data)
            if "port" in settings:
                puerto = (settings.get('port') - 1)
            asyncio.get_event_loop().run_until_complete(handler())
        else:
            print("rcmSend v1.0")  
            print("")
            print("Help")  
            print("    rcmSend [name] [message]")
            print("")
            print('    rcmSend myApp "{\\"dato\\":\\"valor\\"}"')
            print("")
    except Exception as e:
        print(e)
puerto = '2999'    

if __name__ == "__main__":
    main()
    