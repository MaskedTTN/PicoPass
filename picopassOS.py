import time
import usb_hid
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keyboard_layout_us import KeyboardLayoutUS
from adafruit_hid.keycode import Keycode
import displayio
import terminalio
from adafruit_display_text import label
import adafruit_hashlib as hashlib
import json



class picopass:
    """object for picopass"""
    print("made class")
    def __init__(self, display, buttonA, buttonB, buttonX, buttonY):
        """initialises picopass object"""
        self.buttonA = buttonA
        self.buttonB = buttonB
        self.buttonX = buttonX
        self.buttonY = buttonY
        self.display = display
        with open('config.json', 'r')as file:
            self.data = json.load(file)
            file.close()
        print(type(self.data))
        self.pin = 'f77f0ece0aa17656f081c581c06d2b216f5207570c69494f5c879659f03739bc'
        self.progress = ""
        self.proposed_pin = ""

        self.kbd = Keyboard(usb_hid.devices)
        self.layout = KeyboardLayoutUS(self.kbd)
    
    def clear(self, display):
        color_bitmap = displayio.Bitmap(240, 135, 1)
        color_palette = displayio.Palette(1)
        color_palette[0] = 0x0000FF

        bg_sprite = displayio.TileGrid(color_bitmap,
                                    pixel_shader=color_palette, x=0, y=0)
        display.append(bg_sprite)
    
    def lockscreen(self):
        # Draw a smaller inner rectangle
        inner_bitmap = displayio.Bitmap(238, 133, 1)
        inner_palette = displayio.Palette(1)
        inner_palette[0] = 0x000000
        inner_sprite = displayio.TileGrid(inner_bitmap,
                                          pixel_shader=inner_palette, x=1, y=1)
        self.display.append(inner_sprite)

        # Draw a label
        text_group1 = displayio.Group(scale=3, x=20, y=15)
        text1 = "ENTER PIN:"
        text_area1 = label.Label(terminalio.FONT, text=text1, color=0xFF0000)
        text_group1.append(text_area1)
        # Draw a label
        text_group2 = displayio.Group(scale=5, x=60, y=50)
        text_area2 = label.Label(terminalio.FONT, text="____", color=0xFFFFFF)
        text_group2.append(text_area2)

        self.display.append(text_group1)
        self.display.append(text_group2)

        while True:
            if len(self.proposed_pin) <= 3:
                if not self.buttonA.value:
                    self.proposed_pin = self.proposed_pin + 'A'
                    self.progress = self.progress + '*'
                    print("updating")
                    text_group2 = displayio.Group(scale=5, x=60, y=50)
                    text_area2 = label.Label(terminalio.FONT, text=self.progress, color=0xFFFFFF)
                    text_group2.append(text_area2)
                    time.sleep(0.2)
                    self.display.append(text_group2)
                if not self.buttonB.value:
                    self.proposed_pin = self.proposed_pin + 'B'
                    self.progress = self.progress + '*'
                    print("updating")
                    text_group2 = displayio.Group(scale=5, x=60, y=50)
                    text_area2 = label.Label(terminalio.FONT, text=self.progress, color=0xFFFFFF)
                    text_group2.append(text_area2)
                    time.sleep(0.2)
                    self.display.append(text_group2)

                if not self.buttonX.value:
                    self.proposed_pin = self.proposed_pin + 'X'
                    self.progress = self.progress + '*'
                    print("updating")
                    text_group2 = displayio.Group(scale=5, x=60, y=50)
                    text_area2 = label.Label(terminalio.FONT, text=self.progress, color=0xFFFFFF)
                    text_group2.append(text_area2)
                    time.sleep(0.2)
                    self.display.append(text_group2)
                if not self.buttonY.value:
                    self.proposed_pin = self.proposed_pin + 'Y'
                    self.progress = self.progress + '*'
                    print("updating")
                    text_group2 = displayio.Group(scale=5, x=60, y=50)
                    text_area2 = label.Label(terminalio.FONT, text=self.progress, color=0xFFFFFF)
                    text_group2.append(text_area2)
                    time.sleep(0.2)
                    self.display.append(text_group2)

            else:
                m = hashlib.sha256()
                m.update(self.proposed_pin)
                #print('hash: ', m.hexdigest())
                #print('pin given: ', proposed_pin)
                if m.hexdigest() == self.pin:
                    self.clear(self.display)
                    text_group1 = displayio.Group(scale=3, x=20, y=15)
                    text1 = "PIN VERIFIED"
                    print('PIN VERIFIED')
                    text_area1 = label.Label(terminalio.FONT, text=text1, color=0xFF0000)
                    text_group1.append(text_area1)
                    self.display.append(text_group1)
                    time.sleep(1)
                    self.display.pop((len(self.display) - 1))
                    self.menu()
                    break
                else:
                    self.clear(self.display)
                    text_group1 = displayio.Group(scale=3, x=20, y=15)
                    text1 = "PIN DENIED"
                    print('PIN DENIED')
                    text_area1 = label.Label(terminalio.FONT, text=text1, color=0xFF0000)
                    text_group1.append(text_area1)
                    time.sleep(1)
                    self.proposed_pin = ""
                    self.display.append(text_group1)
                    break
            time.sleep(0.1)
    
    def menu(self):
        position = 0
        self.clear(self.display)
        if position > 0:
                text_group3 = displayio.Group(scale=4, x=0, y=20)
                text_area3 = label.Label(terminalio.FONT, text="<", color=0xFFFFFF)
                text_group3.append(text_area3)
                self.display.append(text_group3)

        if  position < (len(self.data['passwords']) - 1):
                text_group4 = displayio.Group(scale=4, x=220, y=20)
                text_area4 = label.Label(terminalio.FONT, text=">", color=0xFFFFFF)
                text_group4.append(text_area4)
                self.display.append(text_group4)

        text_group5 = displayio.Group(scale=3, x=150, y=100)
        text_area5 = label.Label(terminalio.FONT, text="PASSW", color=0xFFFFFF)
        text_group5.append(text_area5)
        self.display.append(text_group5)

        text_group6 = displayio.Group(scale=4, x=50, y=20)
        text_area6 = label.Label(terminalio.FONT, text=self.data['passwords'][position]["Name"], color=0xFFFFFF)
        text_group6.append(text_area6)
        self.display.append(text_group6)
        
        text_group7 = displayio.Group(scale=3, x=0, y=100)
        text_area7 = label.Label(terminalio.FONT, text="USRN", color=0xFFFFFF)
        text_group7.append(text_area7)
        self.display.append(text_group7)

        print(len(self.display))
        changed = False
        while True:
            if changed:
                print(len(self.display))
                changed = False
                self.display.pop((len(self.display) - 1))
                self.display.pop((len(self.display) - 2))
                self.display.pop((len(self.display) - 3))
                if position > 0:
                    print(len(self.display))
                    text_group3 = displayio.Group(scale=4, x=0, y=20)
                    text_area3 = label.Label(terminalio.FONT, text="<", color=0xFFFFFF)
                    text_group3.append(text_area3)
                    self.display.append(text_group3)

                if  position < (len(self.data['passwords']) - 1):
                    print(len(self.display))
                    text_group4 = displayio.Group(scale=4, x=220, y=20)
                    text_area4 = label.Label(terminalio.FONT, text=">", color=0xFFFFFF)
                    text_group4.append(text_area4)
                    self.display.append(text_group4)

                text_group5 = displayio.Group(scale=3, x=150, y=100)
                text_area5 = label.Label(terminalio.FONT, text="PASSW", color=0xFFFFFF)
                text_group5.append(text_area5)
                self.display.append(text_group5)

                text_group6 = displayio.Group(scale=4, x=50, y=20)
                text_area6 = label.Label(terminalio.FONT, text=self.data['passwords'][position]["Name"], color=0xFFFFFF)
                text_group6.append(text_area6)
                self.display.append(text_group6)
                
                text_group7 = displayio.Group(scale=3, x=0, y=100)
                text_area7 = label.Label(terminalio.FONT, text="USRN", color=0xFFFFFF)
                text_group7.append(text_area7)
                self.display.append(text_group7)

            if not self.buttonA.value:
                if position > 0:
                    changed = True
                    print('moving left')
                    position = position - 1
            if not self.buttonX.value:
                if  position < (len(self.data['passwords']) - 1):
                    changed = True
                    print('moving right')
                    position = position + 1
            if not self.buttonY.value:
                text_group5 = displayio.Group(scale=3, x=80, y=55)
                text_area5 = label.Label(terminalio.FONT, text="typing", color=0xFFFFFF)
                text_group5.append(text_area5)
                self.display.append(text_group5)
                time.sleep(.2)
                self.layout.write(self.data['passwords'][position]['password'])
                self.display.pop((len(self.display) - 1))
            if not self.buttonB.value:
                text_group5 = displayio.Group(scale=3, x=80, y=55)
                text_area5 = label.Label(terminalio.FONT, text="typing", color=0xFFFFFF)
                text_group5.append(text_area5)
                self.display.append(text_group5)
                time.sleep(.2)
                self.layout.write(self.data['passwords'][position]['Username'])
                self.display.pop((len(self.display) - 1))
            time.sleep(.1)
        
        