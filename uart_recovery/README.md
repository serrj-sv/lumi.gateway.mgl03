## Flash firmware to Xiaomi Gateway v3 via UART.
If you just need to open telnet on stock firmware, use "software" method:
https://gist.github.com/zvldz/1bd6b21539f84339c218f9427e022709

### Hardware
1. Pry open gateway:

   <img src=https://user-images.githubusercontent.com/511909/98269111-6da8b980-1f9e-11eb-82ef-d435a900edf1.jpg>

1. connect UART:  
   UART Tx  <--> TP4 (Gateway Rx)  
   UART Rx  <--> TP11 (Gateway Tx)  
   UART GND <--> TP8  (Gateway GND)

   <img src="https://user-images.githubusercontent.com/511909/98268507-a8f6b880-1f9d-11eb-80f6-3ae2bee27c5e.png" width="640">
   
   If you damaged UART pins there is backup UART on the back side of the board:
   
   <img src="https://raw.githubusercontent.com/serrj-sv/lumi.gateway.mgl03/main/media/mgl03_back_uart_eth.jpg" width="640">

    Important notes on UART:
    * UART adapter MUST be in 3.3V mode. The Gateway board is 5v intolerant.
    * Do not feed VCC from UART to Board. Use external power supply and micro-usb cable
    * Do NOT touch any other test points (like TP16, TP17, etc), this is NOT NEEDED. 
1. If you don't feel comfortable or confident with soldering - buy "pcb pogo clip" (for example: [Aliexpress](https://www.aliexpress.com/item/4001015704531.html), choose option "2.54MM 3P Single")

### Files
1. Download intermediate bootloader from [bootloader](https://github.com/serrj-sv/lumi.gateway.mgl03/tree/main/uart_recovery/bootloader) folder with speed of your choice: 
    * [rtkboot_38400.bin](https://github.com/serrj-sv/lumi.gateway.mgl03/raw/main/uart_recovery/bootloader/rtkboot_38400.bin) is slowest one and MOST reliable (upload firmware will take a bit more that 1 hour).
    * [rtkboot_57600.bin](https://github.com/serrj-sv/lumi.gateway.mgl03/raw/main/uart_recovery/bootloader/rtkboot_57600.bin) is a bit faster then rtkboot_38400.bin
    * [**rtkboot_115200.bin**](https://github.com/serrj-sv/lumi.gateway.mgl03/raw/main/uart_recovery/bootloader/rtkboot_115200.bin) (recommended) is the best compromise between speed and reliability (upload firmware will take around 20 min).
    * [rtkboot_230400.bin](https://github.com/serrj-sv/lumi.gateway.mgl03/raw/main/uart_recovery/bootloader/rtkboot_230400.bin) is a bit faster then rtkboot_115200.bin
    * [rtkboot_460800.bin](https://github.com/serrj-sv/lumi.gateway.mgl03/raw/main/uart_recovery/bootloader/rtkboot_460800.bin) is fastest one (upload firmware will take around 6 min). 
1. Download mgl03_xxxxx.uart file from [firmware folder](https://github.com/zvldz/mgl03_fw/tree/main/firmware) of your choice.

### Windows
1. Download [mgl03_uart_recovery.ttl](https://github.com/serrj-sv/lumi.gateway.mgl03/raw/main/uart_recovery/mgl03_uart_recovery.ttl)
1. Download and install [Tera Term](https://ttssh2.osdn.jp/index.html.en)
1. Run Tera Term
1. Choose "Serial -> COM port", OK
1. Choose "Control -> Macro"
1. Open .mgl03_uart_recovery.ttl file you downloaded in step [1] 
1. Follow on-screen instructions
1. Perform Factory Reset: after Gateway fully booted click on it's button 10 times repeatedly.

### Linux (credit: [@CODeRUS](https://github.com/coderus))
1. Download [mgl03_uart_recovery.expect](https://github.com/serrj-sv/lumi.gateway.mgl03/raw/main/uart_recovery/mgl03_uart_recovery.expect)
1. make sure following programs are installed:
  * expect
  * sx (from package lrzsz)
  * stty
1. make sure that bootloader (rtkboot_xxxx.bin), firmware (mgl03_xxxxxx.uart) and mgl03_uart_recovery.expect are in the same folder
1. make sure you're in "dialout" group
1. run:
   ```
   chmod +x mgl03_uart_recovery.expect
   ./mgl03_uart_recovery.expect
   ```
 1. follow on-screen instructions
 1. Perform Factory Reset: after Gateway fully booted click on it's button 10 times repeatedly.
 
 ### Troubleshooting
 #### If something goes wrong, check following:
1. Always clean sordering area with alcohol. Dirt and flux remains may cause short-circuit (see: [issue 87](https://github.com/AlexxIT/XiaomiGateway3/issues/87#issuecomment-754325553))
1. Make sure you did't mix UART connection, the only proper way is Tx to Rx and vice versa (NOT Tx to Tx and Rx to Rx): (see [issue 18](https://github.com/serrj-sv/lumi.gateway.mgl03/issues/18)):
   ```
   UART Tx  -> MGL03 Rx
   UART Rx  -> MGL03 Tx
   UART GND -> MGL03 GND
   ```
1. Make sure you know how to download files on GitHub (see [issue 1](https://github.com/serrj-sv/lumi.gateway.mgl03/issues/1)):
   1. Do NOT do "right-click and 'save as'" on filename, instead:
   1. First click on file you want to download and then click on "Download" button
