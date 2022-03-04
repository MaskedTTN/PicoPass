import os
import board
import time
import terminalio
import displayio
import digitalio
import usb_hid
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keyboard_layout_us import KeyboardLayoutUS
from adafruit_hid.keycode import Keycode
import busio
from adafruit_display_text import label
import adafruit_st7789

print("=====================================================")
print("  _____ _____ _____ ____  _____         _____ _____ ")
print(" |  __ |_   _/ ____/ __ \|  __ \ /\    / ____/ ____|")
print(" | |__) || || |   | |  | | |__) /  \  | (___| (___  ")
print(" |  ___/ | || |   | |  | |  ___/ /\ \  \___ \\___ \ ")
print(" | |    _| || |___| |__| | |  / ____ \ ____) ____) |")
print(" |_|   |_____\_____\____/|_| /_/    \_|_____|_____/ ")
print("=====================================================")
print(os.uname())
print("Hello Raspberry Pi Pico/CircuitPython PICOPASS")
print(adafruit_st7789.__name__ + " version: " + adafruit_st7789.__version__)
print()

data = [{"Name": "GitHub", "Username" : 'ghuser', "password" : "it works!"}, {"Name": "othacc", "Username" : 'ghuser', "password" : "it works!"}, {"Name": "Discord", "Username" : 'ghuser', "password" : "it works!"}]
pin = "ABXY"
progress = ""
proposed_pin = ""

kbd = Keyboard(usb_hid.devices)
layout = KeyboardLayoutUS(kbd)

buttonA = digitalio.DigitalInOut(board.GP12)
buttonA.switch_to_input(pull=digitalio.Pull.UP)
buttonB = digitalio.DigitalInOut(board.GP13)
buttonB.switch_to_input(pull=digitalio.Pull.UP)
buttonX = digitalio.DigitalInOut(board.GP14)
buttonX.switch_to_input(pull=digitalio.Pull.UP)
buttonY = digitalio.DigitalInOut(board.GP15)
buttonY.switch_to_input(pull=digitalio.Pull.UP)

# Release any resources currently in use for the displays
displayio.release_displays()

tft_cs = board.GP17
tft_dc = board.GP16
#tft_res = board.GP23
spi_mosi = board.GP19
spi_clk = board.GP18


spi = busio.SPI(spi_clk, MOSI=spi_mosi)

display_bus = displayio.FourWire(spi, command=tft_dc, chip_select=tft_cs)
display = adafruit_st7789.ST7789(display_bus,
                    width=135, height=240,
                    rowstart=40, colstart=53)
display.rotation = 270
# Make the display context
#splash = displayio.Group()
splash = displayio.Group()
display.show(splash)

def clear():
    color_bitmap = displayio.Bitmap(240, 135, 1)
    color_palette = displayio.Palette(1)
    color_palette[0] = 0x0000FF

    bg_sprite = displayio.TileGrid(color_bitmap,
                                pixel_shader=color_palette, x=0, y=0)
    splash.append(bg_sprite)

clear()

def menu():
    position = 0
    clear()
    if position > 0:
            text_group3 = displayio.Group(scale=4, x=0, y=20)
            text_area3 = label.Label(terminalio.FONT, text="<", color=0xFFFFFF)
            text_group3.append(text_area3)
            splash.append(text_group3)

    if  position < (len(data) - 1):
            text_group4 = displayio.Group(scale=4, x=220, y=20)
            text_area4 = label.Label(terminalio.FONT, text=">", color=0xFFFFFF)
            text_group4.append(text_area4)
            splash.append(text_group4)

    text_group5 = displayio.Group(scale=3, x=205, y=100)
    text_area5 = label.Label(terminalio.FONT, text="OK", color=0xFFFFFF)
    text_group5.append(text_area5)
    splash.append(text_group5)

    text_group6 = displayio.Group(scale=4, x=50, y=20)
    text_area6 = label.Label(terminalio.FONT, text=data[position]["Name"], color=0xFFFFFF)
    text_group6.append(text_area6)
    splash.append(text_group6)

    print(len(splash))
    changed = False
    while True:
        if changed:
            print(len(splash))
            changed = False
            splash.pop((len(splash) - 1))
            splash.pop((len(splash) - 2))
            splash.pop((len(splash) - 3))
            if position > 0:
                print(len(splash))
                text_group3 = displayio.Group(scale=4, x=0, y=20)
                text_area3 = label.Label(terminalio.FONT, text="<", color=0xFFFFFF)
                text_group3.append(text_area3)
                splash.append(text_group3)

            if  position < (len(data) - 1):
                print(len(splash))
                text_group4 = displayio.Group(scale=4, x=220, y=20)
                text_area4 = label.Label(terminalio.FONT, text=">", color=0xFFFFFF)
                text_group4.append(text_area4)
                splash.append(text_group4)

            text_group5 = displayio.Group(scale=3, x=205, y=100)
            text_area5 = label.Label(terminalio.FONT, text="OK", color=0xFFFFFF)
            text_group5.append(text_area5)
            splash.append(text_group5)

            text_group6 = displayio.Group(scale=4, x=50, y=20)
            text_area6 = label.Label(terminalio.FONT, text=data[position]["Name"], color=0xFFFFFF)
            text_group6.append(text_area6)
            splash.append(text_group6)

        if not buttonA.value:
            if position > 0:
                changed = True
                print('moving left')
                position = position - 1
        if not buttonX.value:
            if  position < (len(data) - 1):
                changed = True
                print('moving right')
                position = position + 1
        if not buttonY.value:
            text_group5 = displayio.Group(scale=3, x=0, y=100)
            text_area5 = label.Label(terminalio.FONT, text="typing", color=0xFFFFFF)
            text_group5.append(text_area5)
            splash.append(text_group5)
            time.sleep(.2)
            layout.write(data[position]['password'])
            splash.pop((len(splash) - 1))
        time.sleep(.1)

# Draw a smaller inner rectangle
inner_bitmap = displayio.Bitmap(238, 133, 1)
inner_palette = displayio.Palette(1)
inner_palette[0] = 0x000000
inner_sprite = displayio.TileGrid(inner_bitmap,
                                  pixel_shader=inner_palette, x=1, y=1)
splash.append(inner_sprite)

# Draw a label
text_group1 = displayio.Group(scale=3, x=20, y=15)
text1 = "ENTER PIN:"
text_area1 = label.Label(terminalio.FONT, text=text1, color=0xFF0000)
text_group1.append(text_area1)
# Draw a label
text_group2 = displayio.Group(scale=5, x=60, y=50)
text_area2 = label.Label(terminalio.FONT, text="____", color=0xFFFFFF)
text_group2.append(text_area2)

splash.append(text_group1)
splash.append(text_group2)

while True:
    if len(proposed_pin) <= 3:
        if not buttonA.value:
            proposed_pin = proposed_pin + 'A'
            progress = progress + '*'
            print("updating")
            text_group2 = displayio.Group(scale=5, x=60, y=50)
            text_area2 = label.Label(terminalio.FONT, text=progress, color=0xFFFFFF)
            text_group2.append(text_area2)
            time.sleep(0.2)
            splash.append(text_group2)
        if not buttonB.value:
            proposed_pin = proposed_pin + 'B'
            progress = progress + '*'
            print("updating")
            text_group2 = displayio.Group(scale=5, x=60, y=50)
            text_area2 = label.Label(terminalio.FONT, text=progress, color=0xFFFFFF)
            text_group2.append(text_area2)
            time.sleep(0.2)
            splash.append(text_group2)

        if not buttonX.value:
            proposed_pin = proposed_pin + 'X'
            progress = progress + '*'
            print("updating")
            text_group2 = displayio.Group(scale=5, x=60, y=50)
            text_area2 = label.Label(terminalio.FONT, text=progress, color=0xFFFFFF)
            text_group2.append(text_area2)
            time.sleep(0.2)
            splash.append(text_group2)
        if not buttonY.value:
            proposed_pin = proposed_pin + 'Y'
            progress = progress + '*'
            print("updating")
            text_group2 = displayio.Group(scale=5, x=60, y=50)
            text_area2 = label.Label(terminalio.FONT, text=progress, color=0xFFFFFF)
            text_group2.append(text_area2)
            time.sleep(0.2)
            splash.append(text_group2)

    else:
        if proposed_pin == pin:
            clear()
            text_group1 = displayio.Group(scale=3, x=20, y=15)
            text1 = "PIN VERIFIED"
            text_area1 = label.Label(terminalio.FONT, text=text1, color=0xFF0000)
            text_group1.append(text_area1)
            splash.append(text_group1)
            time.sleep(1)
            menu()
            break
        else:
            clear()
            text_group1 = displayio.Group(scale=3, x=20, y=15)
            text1 = "PIN DENIED"
            text_area1 = label.Label(terminalio.FONT, text=text1, color=0xFF0000)
            text_group1.append(text_area1)
            time.sleep(1)
            proposed_pin = ""
            splash.append(text_group1)
            break
    time.sleep(0.1)


time.sleep(3.0)

