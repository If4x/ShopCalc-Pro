# ShopCalc Pro


[![](https://img.shields.io/github/license/If4x/SopCalc-Pro)](LICENSE)

>[!CAUTION]
> This is a DIY project. Although the skills required to replicate this project are considered to be fairly low, people unexperienced with electronics and programming might encounter some difficulties.

>[!Tip]
> Please read these instructions beforehand and make yourself familiar with your equipment. Make sure to check for shortcurcuits after soldering to prevent damage.

1. [Introduction](#introduction)  
2. [Features](#features)  
   - [General](#general)  
   - [Product Page](#product-page-19216841)  
   - [Configuration Page](#configuration-page-192168418080)  
   - [Sales Page](#sales-page-19216841sales)  
3. [Build it Yourself](#build-it-yourself)  
   - [Parts Needed](#parts-needed)  
   - [Skills Needed](#skills-needed)  
   - [Pin Connections / Pin Map](#pin-connections--pin-map)  
   - [Additional 3D-Printed Case](#additional-3d-printed-case)  
   - [Completed Assembly](#completed-assembly)  
4. [Programming the ESP and Startup](#programming-the-esp-and-startup)  
   - [First Power-up](#first-power-up)  
   - [After First Power-up](#after-first-power-up)  
5. [Troubleshooting](#troubleshooting)  
6. [Limitations](#limitations)  
7. [Coming Soon](#coming-soon)  


## Introduction
ShopCalc Pro is a further and more advanced development of its predecessor, **ShopCalc** (see [ShopCalc GitHub by If4x](https://github.com/If4x/ShopCalc)).  
While ShopCalc focused on being usable out of the box without the need for additional modules other than the ESP32, this restriction led to performance issues when the shop size increased to the point where EEPROM storage (400 bytes in size) would run out. On top of that, the ESP32's EEPROM has very limited write cycles, which could result in total failure during an event if used long-term.  
To address these issues and open up possibilities for future features, a simple SD module is now connected to the ESP32 via SPI.

This is a simple, web-based shop calculator running on the ESP32. It's designed to serve as a basic calculator for school events to optimize the selling process, save time, and minimize calculation errors (which happen more often than one might think when the task is done manually by younger students). It's platform-independent, meaning it runs on all mobile devices (e.g., iOS, Android, ...). Additionally, it is about the size of a regular car key and can easily be carried around in a pocket which makes it with it's own WIFI and platform independendece a very versatile tool that has been proven to be very useful at many events.

These instructions are written in a way that everybody (also unexperienced people) are able to replecate this themselves. This  shows in a way that there is very detailed information that is unneccessary for experienced people but good to know for unexperienced people.

## Features
### General
- Extremely low power consumption (0.7W), allowing the system to run for days on a standard-sized power bank.  
- Easy and intuitive to use (seriously, if you can navigate a browser, you can use this).  
- Utilizes onboard components and an SD module to keep things as simple and easy to build as possible.

### Product Page (192.168.4.1)  
<img src="https://github.com/If4x/SopCalc-Pro/blob/main/UI/Shop_page.PNG?raw=true" alt="Image of shop page" height="400">
- Allows the user to select products ordered by customers.  
- Buttons for +1, +2, +3 to speed up input.  
- Color-coded buttons for adding/removing items from the cart for easy and intuitive interaction.  
- Order submit (saves the order to statistics).  
- Order delete (in case of "oops, I made a big mistake," and deleting everything is faster).  
- Displays the specific quantity of ordered items.  
- Displays the total amount.  
- Displays the included deposit for glasses and bottles (default: 1€).

### Configuration Page (192.168.4.1:8080)  
<img src="https://github.com/If4x/SopCalc-Pro/blob/main/UI/Config_page.PNG?raw=true" alt="Image of config page" height="400">

- Edit products (name, price, deposit).  
- Delete products (in case they are no longer used or outdated).  
- Create new products.
- Reset the products to default products.

### Sales Page (192.168.4.1/sales)  
<img src="https://github.com/If4x/SopCalc-Pro/blob/main/UI/Sales_page.PNG?raw=true" alt="Image of sales page" height="400">

- Displays total items sold.  
- Option to export the statistics for later use.  
- Reset statistics (before or after an event to get accurate results).

# Build it yourself

### Parts needed
- ESP32-Dev
- Basic SPI Micro-SD-Module
- Micro-SD-Card (recommended 2-8GB since those are cheap but even 2GB is overkill, tested and works with 2GB-32GB)

### Skills needed
- Being able to type on a computer without breaking your fingers
- Being able to hold a sottering iron in a way that you don't end up cooking your fingers well done
- Being able to read these instructions

The system is intended to run on the **ESP32-Dev**. It uses onboard components and a basic/standard Micro-SD module to minimize the required technical skills to nearly zero.

### Pin connections / Pin Map
Pin connections. (recommended to be soldered)
| Pin on ESP32 | Pin on SD Module      |
|--------------|-----------------------|
| VIN/5V       | VCC                   |
| GND          | GND                   |
| D5           | CS                    |
| D18          | CLK                   |
| D19          | MISO                  |
| D23          | MOSI                  |

Important: Keep in mind that the cables have to be long enogh to be able to "fold" the SD module on the back of the ESP32 but also short enogh so that they can fit into the case.

<img src="https://github.com/If4x/ShopCalc-Pro/blob/main/case/Cable%20length.JPEG" alt="Visualisation of cable length" width="300">


### Additional 3D-Printed Case  
There are STL files for an additional case housing the ESP32, SD-Card module and cables. This case can be fully 3D printed and allows an easy assemply of the components. The case inclused air vents to precent overheating caused by the active use of the oboard WIFI.

The case can be printed from the stl files in the case folder [here](https://github.com/If4x/SopCalc-Pro/tree/main/case). **The folder also includes pictures for assembly and orientation.**
There are two versions of the case:
1. Basic Version. Just the Case that fits the two covers. Works perfectly fine
2. Keychain Version. Has a hole to add a keychain of your choice. Ideal if the System is transported a lot, not as easy to loose.

Recommended Print settings:
- 3 Wall-lines
- 4 Layers bottom and top
- 15% Infil (Pattern of your choice)
- Support (recommended using organic support)
- No brim

### Completed Assembly
<img src="https://github.com/If4x/SopCalc-Pro/blob/main/case/Full%20assembly.JPEG" alt="Image of completed assebly in case" width="450">

## Programming the ESP and Startup
### First Power-up
1. Plug the SD card into the SD module.  
2. Connect the ESP32-Dev to your computer.  
3. Flash `main.cpp` to the ESP32-Dev (we recommend using PlatformIO for quick compilation; Arduino IDE works too, but it's slower).  
4. Connect your smartphone to Wi-Fi (SSID: **Kasse** | Password: **BitteGeld**). This can be modified in the `main.cpp` file at the beginning.  
5. Open your browser and type `192.168.4.1:80` in the search bar to access the shop page.  
   For the sales overview page, type `192.168.4.1/sales`. Here you can export your sales data for statistical use.  
   For the configuration page, type `192.168.4.1:8080`.  
   And you're done! Simple as that!  
   Now, your system is ready to go. It has created the configuration files on the SD card, so it will store the product list and sold units even after power is disconnected from the ESP.

### After First Power-up
1. Plug the SD card into the SD module.  
2. Connect the ESP32-Dev to power.  
3. Connect to Wi-Fi (if not modified, the default connection is as described in the First Power-up steps).  
4. Open your browser and type `192.168.4.1` to access the shop page.  
   For the configuration page, type `192.168.4.1:8080`.  
   For the sales overview page, type `192.168.4.1/sales`.  
   And again, you're done! Super easy!

## Troubleshooting 
The onboard LED of the ESP is primarily used as a status LED, blinking briefly every second. However, it also serves as a visual indicator if something went wrong, blinking a specific number of times to signal the error.

| # of blinks | ERROR                                 |
|-------------|---------------------------------------|
| 1           | SD not found                          |
| 2           | SD found but unable to initialize     |
| 3           | SD unable to mount                    |
| 4           | File error read/write                 |
| 10          | Unknown Error, check Serial Monitor   |

When problem cannot befixed by rebooting (pressing the EN button) or powercycling, please check Serial monitor with the [reader script](https://github.com/If4x/SopCalc-Pro/blob/main/serial_reader/reader.py) for more detaild debuggin information. Adjust the COM Port in the reader script to your needs. Additionally, the serial output is colorcoded for better redability and incsreased efficiency while troubleshooting.

Example CLI readout from COM port after rebooting:
<img src="https://github.com/If4x/SopCalc-Pro/blob/main/UI/Serial_troubleshooting.png?raw=true" alt="Image of serial output while troubleshooting" width="450">

## Limitations
Now that the system uses an SD card, there are basically no limits on how many products you can have in your store (but seriously, if you manage to fill a 2GB card just with products, you might want to reconsider your life choices—or maybe just upgrade to something more professional instead of using this piece of "garbage").  
**However**, to improve performance, the following limitations have been set in the code (and can be changed to meet your needs):  
- **MAX_PRODUCTS** is set to 50 but can be increased for a larger store.  
- **name[50]** limits the length of product names for better readability. It is not recommended to increase this much further, as the usability of the system would decrease significantly.

## Coming Soon
- ESP configuration (Wi-Fi password, name, etc.) saved to a file on the SD card. This would allow someone to create default config files, save them to the SD card, and apply them without modifying the code, enabling even the most inexperienced microcontroller users to make changes to the ESP config.  
- Users log to log who sold what. (Statistical usage)
