from ctypes import cast, POINTER
from comtypes import CLSCTX_ALL
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume
devices = AudioUtilities.GetSpeakers()
interface = devices.Activate(
IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
volume = cast(interface, POINTER(IAudioEndpointVolume))
import pyautogui
import os
import webbrowser
import requests
import json

def set_volume(vol):
    volume.SetMasterVolumeLevelScalar(vol / 100, None)

def get_volume():
    return round(volume.GetMasterVolumeLevelScalar() * 100)

def click(*args):
    pyautogui.hotkey(*args)

def open_app(a):
    os.startfile(a)

def open_url(url):
    webbrowser.open(url)

def print_text(text):
    print(text)

def wled_toggle():
    try:
        requests.post("http://wled-5d82fd.local/json/state", headers={"Content-Type": "application/json"}, data=json.dumps({"on":"t","v":True}))
    except Exception as e:
        print()

def wled_brightness(value):
    try:
        requests.post("http://wled-5d82fd.local/json/state", headers={"Content-Type": "application/json"}, data=json.dumps({"bri":value}))
    except Exception as e:
        print()