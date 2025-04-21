# SopCalc Pro
 ShopCalc Pro is a further development of its predecessor system SopCalc (see https://github.com/If4x/ShopCalc). 
 While ShopCalc focused on being usable out of the box without the need of additional modules other than the ESP32. This Restriction resulted in preformance issues when the shop size was increased in a way that EEPROM storage will run out (400 byte in size). On the other hand, EEPROM on ESP32 has very limited write cycles which could result in complete failure at an event when used in the long term.
 To solve those errors and opening up possibilities for future features, a simple SD module is being connected to ESP32 via SPI. 

 This is a simple web-based shop calculator running on ESP32. It's intended to serve as a basic calculator for school events to optimize the process of selling products timewise but also to minimize calculation errors occurring quite frequently when performed by head by younger students. It is plattform independent (runs on all mobile devices e.g. iOS, Android, ...). 

# Features
**General**
- Extremely low Poweconsumption (0,7W) enabeling the system to run for days while being powered by a regularly sized powerbank
- Easy and intuitive to use
- Utilizes onboard components and SD Module in order to be as simple and easy to build as possible

**Product Page** (192.168.4.1)
- Let's user select the products ordered by customer
- buttons for +1, +2, +3 to increase speed for inputting into system
- colorcoded buttons for adding/removing items from cart for easy and intuitive interaction
- order submit (saves order to statistic)
- order delete (if bigger mistakes were made and it's just faster to delete the whole order
- displays specific amount of ordered items
- displays total amount
- displays included deposit for glasses and bottles (default 1€)

**Configuration Page** (192.168.4.1:8080)
- Edit Products (name, price, deposit)
- Delete Products (if no longer used/outdated)
- Create new product

**Sales Page** (192.168.4.1/sales)
- Displays total items sold
- Option for export of the statistic for later usage
- Reset statistics (after/before an event to get accurate results)

**Additional 3D-Printed Case (COMMING SOON)**
There are STL files for an additional ESP case that has buttons for EN abd BOOT to be able to restart the ESP32. It also has "air vents" for the Processor due to heat generation caused by using the WIFI module actively. It also holds the SD card module.

**Errorcodes**
The onboard LED of the ESP is used as status LED in first place and blinks shortly every second. However it also functions as visual Output if something went wrong. It blinks a specific amount of times indicating the error.

| # of blinks  | ERROR                              |
|--------------|------------------------------------|
| 1            | SD not found                       |
| 2            | SD found but unable to initialize  |
| 3            | SD unable to mount                 |
| 4            | Unknown Error, check Serial Moitor |


# Setup
The system is intended to run on ESP32-Dev. It uses onboard components and a basic/standard Micro-SD module to minimize the requiered technical sklills to almost zero.

**First Powerup**
1. Plug SD-Card into SD-Module
2. Connect ESP32-Dev to computer
3. Flash main.cpp to ESP32-Dev (recommended using PIO for quick compilation, Arduino IDE works too but slower)
4. Connect your smartphone to wifi (SSID: Kasse | Password: BitteGeld) Can be modified in the main.cpp code at the beginning of the file
5. Go to your browser and ytpe 192.168.4.1:80 into the search bar to access shop page
   Go to your browser and type 192.168.4.1:80/sales to go to the overview page of sold products where you can export this for statistical usage
   Go to your browser and type 192.168.4.1:8080 into the search bar to access config page And you're done! As simple as this!
Now you system is ready to go. It created the configuration files on the SD card so it will store the product list & sold units for use after power was disconnected from ESP

**After First Powerup**
1. Plug SD-Card into SD-Module
2. Connect ESP32-Dev to Power
3. Connect to wifi (if not modified, default connection see First Powerup step 3)
4. Go to your browser and ytpe 192.168.4.1 into the search bar to access shop page
   Go to your browser and type 192.168.4.1:8080 into the search bar to access config page
   Go to your browser and type 192.168.4.1/sales to go to the overview page of sold products where you can export this for statistical usage And you're done! As simple as this!

# Limitations
Since the system now uses a SD-card, there are basically no limits on how many products are in your store (if you manage to fill up a 2GB card just with products, I don't know what you're doing but you might consider switching to something professional instead of using this pice of "garbage".)
**HOWEVER** To increase performance, following limitations were set in the code and can be changed to meet your needs:
- MAX_PRODUCTS was set to 50, can be increased to increase shop size
- name[50] limits the length of product names in the shop for better readability. Not recommended to be increased further since usability of the system would decrease pretty soon.

# Comming soon
- Shortcut on config Page to reset product List to default
- ESP Config (Wifi Password, Name, ...) saved to file on SD. This would enable that somebody just creates config files as their default, saves them to the SD Card and they will be applied without modifying the code enabeling someone unexperienced with microcontrollers to make changes to the ESP config
- 3D Printed Case
- Assemly instructions
