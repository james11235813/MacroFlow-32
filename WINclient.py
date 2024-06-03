import actions
import json
import asyncio
import websockets
import yaml

''' example data
{"type":"enc","action":"rotation","value":22,"page":1}
{"type":"enc","action":"clicked","page":1}
{"type":"enc","action":"rotation","value":23,"page":1}
'''

def data_process(data):
    print(data["type"])
    if data["type"] in config["type"]:
        print("type in config")
        if data["type"] == "keypad":
            print("keypad")
            action = config["type"][data["type"]]["page"][data["page"]][data["button"]]
            print(action)
        elif data["type"] == "enc":
            print("enc")
            action = config["type"][data["type"]]["page"][data["page"]]["action"][data["action"]]
            print(action)
        else:
            print("type not recognized")
            action = None
        if action:
            #execute action first element is the function name and the rest are the arguments
            if "value" in data:
                value = data.get("value")
                if "value" in action:
                    getattr(actions, action[0])(value)
                    return None
            else:
                
                getattr(actions, action[0])(*action[1:])
                return None

with open("config.yaml", "r") as file:
    config = yaml.safe_load(file)

prev_volume = None

async def send_updates(websocket):
    global prev_volume
    while True:
        volume = actions.get_volume()
        if volume != prev_volume:
            await websocket.send(json.dumps({"sync": {"media": volume}}))
            print(f"Sent volume update to server: {volume}")
            prev_volume = volume
        await asyncio.sleep(0.3)
        

async def receive_updates(websocket):
    global prev_volume
    async for message in websocket:
        print(f"Received message from server: {message}")
        data = json.loads(message)
        data_process(data)


async def main():
    uri = config["server"]["url"]
    async with websockets.connect(uri) as websocket:
        send_task = asyncio.create_task(send_updates(websocket))
        receive_task = asyncio.create_task(receive_updates(websocket))
        await asyncio.gather(send_task, receive_task)

if __name__ == "__main__":
    asyncio.run(main())