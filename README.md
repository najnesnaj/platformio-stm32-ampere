#platformio-stm32-ampere
this is no my project, but something I found on http://atcnetz.blogspot.com/

STM sells these discovery boards with e-ink display for a few bucks.
The STM32L053 contains a chip to measure current.

So what Aaron Christophel did, was write an arduino sketch to turn the demo-board into a very accurate ampere-meter (nano micro milli)

Just flash sketch to board.
On top of E-ink display is connector, connect 2 pins right hand side.
Between ground and free pin on connector (+) feed your device.

Here is a discription on howto setup on raspberry 

get platformio.ini from this repo
platformio init --board nucleo_l053r8
platformio init
platformio lib install U8g2

cp sketch under src to your src/dir

place the variant.h from this repo under
~/.platformio/packages/framework-arduinoststm32/variants/NUCLEO_L053R8#

and type pio run

firmware.bin is somewhere under .pioenvs directory

platformio stm32 discovery amperemeter 



============================
to get data on serial port, connect PA9 AND PA10


on orange pi adapt armbianEnv -- overlays = uart1 uart3


this gets you an extra port /dev/ttyS3

============================
