# Paper Signals for Raspberry Pi
## Installing
You will need to install.

**premake4 libcurl wiringpi** from apt

    sudo apt-get install libcurl4-openssl-dev premake4 wiringpi

**ServoBlaster**
From github. I used the version at
https://github.com/srcshelton/servoblaster
Build and install as per instructions. I altered the configuration to
user pins 11 and 12, as the default first servo pin is used for 1-wire.

**AdruinoJson**
Clone ArduinoJson from github into the RaspberryPiSignals directory
cd RaspberryPiSignals
git clone https://github.com/bblanchon/ArduinoJson 

## Compiling
Configure Credentials.h as required
Then

    premake4 gmake
    make


## Running

    build/debug/paper-signals


