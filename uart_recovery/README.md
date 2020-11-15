This instruction will help you to flash latest firmware 1.4.7 with telnet enabled with minimal effort.
* No need to decode password
* No need to "swap slots"
* No need to touch TP16/TP17 points

Requires minimum soldering skills (no soldering manual is coming soon)

1. [Download](https://github.com/serrj-sv/lumi.gateway.mgl03/tree/main/uart_recovery/bootloader) intermediate bootloader with speed of your choice: 
  * 38400 is slowest one and MOST reliable (upload firmware will take a bit more that 1 hour).
  * 115200 is the best compromise between speed and reliability (upload firmware will take a bit more that 20 min).
1. [Download](https://github.com/serrj-sv/lumi.gateway.mgl03/tree/main/uart_recovery) all files from recovery firmware folder of your choice (for now the latest one is [mgl03_1.4.7_0065_mod20201110](https://github.com/serrj-sv/lumi.gateway.mgl03/tree/main/uart_recovery/mgl03_1.4.7_0065_mod20201110))
1. Download and install [Tera Term](https://ttssh2.osdn.jp/index.html.en)
1. Pry open gateway:

   <img src=https://user-images.githubusercontent.com/511909/98269111-6da8b980-1f9e-11eb-82ef-d435a900edf1.jpg>

1. connect UART:

   <img src="https://user-images.githubusercontent.com/511909/98268507-a8f6b880-1f9d-11eb-80f6-3ae2bee27c5e.png" width="640">

    Important notes on UART:
    * UART adapter MUST be in 3.3V mode. The Gateway board is 5v intolerant.
    * Do not feed VCC from UART to Board. Use external power supply and micro-usb cable
    * Do NOT touch any other test points (like TP16, TP17, etc), this is NOT NEEDED. 

1. Run Tera Term
1. Choose "Serial -> COM port", OK
1. Choose "Control -> Macro"
1. Open .ttl file you downloaded in step [2] 
1. Follow on-screen instructions
1. Perform Factory Reset: after Gateway fully booted click on it's button 10 times repeatedly.
