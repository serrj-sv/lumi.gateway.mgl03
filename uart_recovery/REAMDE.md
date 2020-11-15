This instruction will hellp you to flash latest firmware 1.4.7 with telnet enabled with minimal effort.
==No need to decode possword==
==No need to "swap slots"==
==No need to touch TP16/TP17 points==

Requires minimum soldering skills (no soldering manual is coming soon)

1. Download bootloader/rtkboot_115200.bin
2. Download all files from recovery firmware folder of your chioce (for now the latest one is mgl03_1.4.7_0065_mod20201110)
3. Download and intstall Tera Term (https://ttssh2.osdn.jp/index.html.en)
4. Pry open gateway 
<img src=https://user-images.githubusercontent.com/511909/98269111-6da8b980-1f9e-11eb-82ef-d435a900edf1.jpg>
5. connect UART:
<img src=https://user-images.githubusercontent.com/511909/98268507-a8f6b880-1f9d-11eb-80f6-3ae2bee27c5e.png>
Importand notes on UART:
- UART adapter MUST be in 3.3V mode. The Gateway board is 5v intolerant.
- Do not feed VCC from UART to Board. Use external power supply and micro-usb cable
- Do NOT touch any other test points, this is NOT NEEDED. 
6. Run Tera Term
7. Choose "Serial -> COM port", OK
8. Choose "Control -> Macro"
6. Open .ttl file you downloaded in step [2] 
7. Follow on-screen instructions
