import os
import board
import time
import terminalio
import displayio
import digitalio
#import usb_hid
#from adafruit_hid.keyboard import Keyboard
#from adafruit_hid.keyboard_layout_us import KeyboardLayoutUS
#from adafruit_hid.keycode import Keycode
import busio
from adafruit_display_text import label
import adafruit_st7789
import adafruit_imageload

version = "V0.1"

print("=====================================================")
print("  _____ _____ _____ ____  _____         _____ _____ ")
print(" |  __ |_   _/ ____/ __ \|  __ \ /\    / ____/ ____|")
print(" | |__) || || |   | |  | | |__) /  \  | (___| (___  ")
print(" |  ___/ | || |   | |  | |  ___/ /\ \  \___ \\___ \ ")
print(" | |    _| || |___| |__| | |  / ____ \ ____) ____) |")
print(" |_|   |_____\_____\____/|_| /_/    \_|_____|_____/ ")
print("=====================================================")
print(version)
print(os.uname())
print("Hello Raspberry Pi Pico/CircuitPython PICOPASS")
print(adafruit_st7789.__name__ + " version: " + adafruit_st7789.__version__)
print()

buttonA = digitalio.DigitalInOut(board.GP12)
buttonA.switch_to_input(pull=digitalio.Pull.UP)
buttonB = digitalio.DigitalInOut(board.GP13)
buttonB.switch_to_input(pull=digitalio.Pull.UP)
buttonX = digitalio.DigitalInOut(board.GP14)
buttonX.switch_to_input(pull=digitalio.Pull.UP)
buttonY = digitalio.DigitalInOut(board.GP15)
buttonY.switch_to_input(pull=digitalio.Pull.UP)


displayio.release_displays()
tft_cs = board.GP17
tft_dc = board.GP16
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

#clear()

#logo = displayio.OnDiskBitmap("/PicoPassLogo.bmp")

#display = board.DISPLAY

bitmap, palette = adafruit_imageload.load("/PicoPassLogo.bmp", bitmap=displayio.Bitmap, palette=displayio.Palette)



# Create a TileGrid to hold the bitmap
tile_grid = displayio.TileGrid(bitmap, pixel_shader=palette)

# Create a Group to hold the TileGrid
group = displayio.Group()

# Add the TileGrid to the Group
group.append(tile_grid)

text_group = displayio.Group(scale=2, x=0, y=10)
text_area = label.Label(terminalio.FONT, text=version, color=0x0000FF)
text_group.append(text_area)
group.append(text_group)

# Add the Group to the Display
splash.append(group)

time.sleep(.1)
splash.pop()

if (not buttonA.value) and (not buttonB.value):
    print("booting into configuration mode!")
    try:
        import config
    except Exception as inst:
        print(inst)
else:
    print("booting into PicoPass!")
    try:
        import picopassOS
        picopassinstance = picopassOS.picopass(splash, buttonA, buttonB, buttonX, buttonY)
        picopassinstance.lockscreen()
        #picopassinstance.menu()
    except Exception as inst:
        print(inst)
