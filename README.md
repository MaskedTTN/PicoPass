# PicoPass
![image](https://user-images.githubusercontent.com/76824354/186677963-efc9114a-2035-4b4c-993d-25c11beb30ae.jpeg)

A portable password manager using a raspberry pico and a pimoroni display pack
This branch contains the development firmware, it contains features that are still in development
and may not work as expected.

# Instructions
1.) Install CircuitPython on raspberry pico
2.) move files from repository to CIRCUITPY drive
3.) fit pico display pack https://shop.pimoroni.com/products/pico-display-pack?variant=32368664215635
4.) hold buttons A and B when running main.py
5.) replace serial port with the pico's serial port
6.) run talker.py file
7.) enter create-account name username password to add passwords in picopass
8.) your picopass is ready the pin currently is set to ABXY and there will be an option to change your pin in upcoming versions

TO-DO:
  - make picopass logo  [DONE!]
  - add bootscreen [DONE!]
  - store master password using hash [DONE!]
  - store password in seperate file preferable csv for easy migration from chrome [IN PROGRESS!]
  - add encryption ... somehow [IN PROGRESS!]
  - make python desktop app 
  - make app and picopass communicate
  - let app set up and manage picopass

