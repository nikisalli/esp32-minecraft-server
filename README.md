# esp32-minecraft-server
this is an open source implementation of the minecraft server to be run on a esp32.

# prerequisites
- esp32
- pc with platformio installed on your favourite IDE

# how-to
1) edit the file include/config_edit_me.h with your WiFi credentials
2) rename the file to config.h
3) you're ready to compile, upload and enjoy!

# gameplay screenshots
![alt text](https://github.com/nikisalli/esp32-minecraft-server/blob/main/images/image1.jpg?raw=true)
![alt text](https://github.com/nikisalli/esp32-minecraft-server/blob/main/images/image2.jpg?raw=true)
![alt text](https://github.com/nikisalli/esp32-minecraft-server/blob/main/images/image3.jpg?raw=true)
![alt text](https://github.com/nikisalli/esp32-minecraft-server/blob/main/images/image4.jpg?raw=true)

# TODO
- better way to buffer data and auto packet length calculation
- general cleanup
- parse more packets sent from the client
- maybe add specific blocks for hardware interaction?
- better way of storing chunks (5 bits per block are used)
- support psram for greater room for more chunks and players
- save game progress
- maybe support more protocols? (takes a lor of effort)
- better client handling

# known issues
- lag because the way of buffering data isn't optimal
- sometimes when a client disconnects it may take several packets for the server to notice
- players with same username can join
- sometimes you can't see other players (happens after some disconnection cycles)
- if you fall in the void you'll keep falling until the client eventually crashes