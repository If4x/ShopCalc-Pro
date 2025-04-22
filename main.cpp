/*
This is a simple web-based cash register system for the ESP32.
Developed by Imanuel Fehse (2025).

It's intended to serve as a basic shop calculator for school events or similar to optimize the process of sellig products timewise.
*/


// Libraries for ESP32
#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>

// Initialize SD Card
// pins
#define SD_CS 5 // Chip select pin for SD card 
#define SD_CLK 18 // SPI clock pin 
#define SD_MISO 19 // SPI MISO pin 
#define SD_MOSI 23 // SPI MOSI pin 


// Initialize Wifi Module
const char* ssid = "Kasse";         // SSID of th WIFI
const char* password = "BitteGeld"; // Password for WIFI

// Port 80 (Kassenseite) und Port 8080 (Konfigurationsseite)
// Standard IP for webserver is 192.168.4.1
WebServer server(80);        // product page
WebServer configServer(8080); // config page

#define LED_PIN 2  // GPIO der Onboard-LED (meist GPIO 2)
#define MAX_PRODUCTS 50 // max number of products in the shop

unsigned long previousMillis = 0;
const long interval = 900; // blinking interval
bool ledOn = false; // state of status led

struct Product {
  char name[50]; // product name max 50 chars 
  float price; // two decimal places
  bool hasDeposit; // true if product has deposit
  int count; // number of products in cart
  int sold; // number of products sold (for sales overview)
};

// colors for serial monitor
struct Colors {
  const char* red = "\033[31m"; // red
  const char* green = "\033[32m"; // green
  const char* yellow = "\033[33m"; // yellow
  const char* blue = "\033[34m"; // blue
  const char* magenta = "\033[35m"; // magenta
  const char* cyan = "\033[36m"; // cyan
  const char* white = "\033[37m"; // white
  const char* reset = "\033[0m"; // reset color
} colors;
Colors color; // create color object

Product products[MAX_PRODUCTS]; // Array for products
int totalSold[MAX_PRODUCTS]; // cumulative number sold per product
int productCount = 0; // max number of products in the shop


// if SD is empty, default products are loaded
Product defaultProducts[] = {
  {"Brezel", 2.50, false, 0},
  {"Fanta", 2.50, true, 0},
  {"Cola", 2.50, true, 0},
  {"Spezi", 3.00, true, 0},
  {"Apfelschorle", 3.00, true, 0},
  {"Ensinger Medium", 2.00, true, 0},
  {"Ensinger Still", 2.00, true, 0},
  {"Bier", 3.00, true, 0},
  {"Sekt", 3.00, true, 0}
};

const int defaultProductCount = 9; // number of default products


void blinkLED(int number) {
  for (int i = 0; i < number; i++) {
    digitalWrite(LED_PIN, HIGH); // LED an
    delay(200);
    digitalWrite(LED_PIN, LOW);  // LED aus
    delay(200);
  }
  delay(500); // wait before next blink
}

void error(int number) {
  // switchcase for error handling
  switch (number) {
    case 1:
      Serial.println(String(color.red) + "SD card not found!" + String(color.reset));
      blinkLED(1);
      break;
    case 2:
      Serial.println(String(color.red) + "SD card not initialized!" + String(color.reset));
      blinkLED(2);
      break;
    case 3:
      Serial.println(String(color.red) + "SD card not mounted!" + String(color.reset));
      blinkLED(3);
      break;
    case 4:
      Serial.println(String(color.red) + "File error!" + String(color.reset));
      blinkLED(4);
      break;
    default:
      Serial.println(String(color.red) + "Unknown error!" + String(color.reset));
      blinkLED(10);
      break;
  }
}

// initialize SD card
void initSD() {
  if (!SD.begin(SD_CS)) {
    error(1); // SD card not found
    return;
  }
  Serial.println(String(color.green) + "SD card initialized successfully." + String(color.reset));
  // create file if it does not exist
  if (!SD.exists("/products.csv")) {
    Serial.println(String(color.blue) + "No product config found, loading default products." + String(color.reset));
    File file = SD.open("/products.csv", FILE_WRITE);
    if (file) {
      file.println("0"); // write 0 to the file
      file.close();
    } else {
      error(2); // SD card not initialized
    }
  }
  if (!SD.exists("/sales.csv")) {
    Serial.println(String(color.blue) + "sales.csv not found, creating new file." + String(color.reset));
    File file = SD.open("/sales.csv", FILE_WRITE);
    if (file) {
      file.println("0"); // write 0 to the file
      file.close();
    } else {
      error(3); // SD card not mounted
    }
  }
}



void saveSalesToSD() {
  File file = SD.open("/sales.csv", FILE_WRITE);
  if (!file) {
    Serial.println("[saveSalesToSD] Failed to open file for writing.");
    error(4); // file error
    return;
  }

  for (int i = 0; i < productCount; i++) {
    file.print(products[i].name); file.print(',');
    file.println(totalSold[i]);
  }
  file.close();
  Serial.println(String(color.reset) + "[saveSalesToSD] Sales data saved to SD card.");
}

void loadSalesFromSD() {
  File file = SD.open("/sales.csv");
  if (!file) {
    Serial.println(String(color.reset) + "[loadSalesFromSD] No sales file found. Initializing empty sales.");
    for (int i = 0; i < productCount; i++) {
      totalSold[i] = 0;
    }
    saveSalesToSD();
    return;
  }

  int index = 0;
  while (file.available() && index < productCount) {
    String line = file.readStringUntil('\n');
    int comma = line.indexOf(',');
    if (comma > 0) {
      totalSold[index] = line.substring(comma + 1).toInt();
      index++;
    }
  }
  file.close();
  Serial.println(String(color.reset) + "[loadSalesFromSD] Sales data loaded from SD card.");
}

void printSDData(char* filename) {
  File file = SD.open(filename);
  if (!file) {
    Serial.print("[printSDData] Failed to open file: ");
    Serial.println(filename);
    error(4); // file error
    return;
  }
  Serial.print(String(color.blue) + "[printSDData] Contents of: ");
  Serial.println(filename + String(color.reset));
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}


// save to SD
void saveProductsToSD() {
  File file = SD.open("/products.csv", FILE_WRITE);
  if (!file) {
    Serial.println("[saveProductsToSD] Failed to open file for writing.");
    error(4); // file error
    return;
  }

  file.println(productCount);
  for (int i = 0; i < productCount; i++) {
    file.print(products[i].name); file.print(',');
    file.print(products[i].price); file.print(',');
    file.print(products[i].hasDeposit); file.print(',');
    file.print(products[i].count); file.print(',');
    file.println(products[i].sold);
  }
  file.close();
  Serial.println(String(color.green) + "[saveProductsToSD] Products saved to SD card." + String(color.reset));
}

void loadProductsFromSD() {
  File file = SD.open("/products.csv");
  if (!file) {
    Serial.println("[loadProductsFromSD] File not found. Using default products.");
    productCount = defaultProductCount;
    for (int i = 0; i < productCount; i++) {
      products[i] = defaultProducts[i];
    }
    saveProductsToSD();
    return;
  }

  productCount = file.readStringUntil('\n').toInt();
  for (int i = 0; i < productCount && file.available(); i++) {
    String line = file.readStringUntil('\n');
    int idx = 0;
    String parts[5];
    for (int j = 0; j < 5 && idx >= 0; j++) {
      int next = line.indexOf(',', idx);
      if (next == -1 && j < 4) break;
      parts[j] = line.substring(idx, (next == -1 ? line.length() : next));
      idx = next + 1;
    }
    parts[0].toCharArray(products[i].name, sizeof(products[i].name));
    products[i].price = parts[1].toFloat();
    products[i].hasDeposit = parts[2].toInt();
    products[i].count = parts[3].toInt();
    products[i].sold = parts[4].toInt();
  }
  file.close();
  Serial.println(String(color.green) + "[loadProductsFromSD] Products loaded from SD card." + String(color.reset));
}

void handleSellProduct(String productName) {
  for (int i = 0; i < productCount; i++) {
    if (String(products[i].name) == productName) {
      products[i].count++; // Increase the sold count
      break;
    }
  }

}

// cacluate total price of all products in cart
float calculateTotal() {
  float total = 0;
  for (int i = 0; i < productCount; i++) {
    total += products[i].count * products[i].price;
    if (products[i].hasDeposit) total += products[i].count * 1.0;
  }
  return total;
}

// calculate total deposit of all products in cart (is gonna be shown as already included in total price)
float calculateDeposit() {
  float deposit = 0;
  for (int i = 0; i < productCount; i++) {
    if (products[i].hasDeposit) deposit += products[i].count * 1.0;
  }
  return deposit;
}


void handleSalesOverview() {
  loadSalesFromSD(); // load sales from SD
  
  // Start the HTML content
  String html = "<h1>Verkäufe</h1>";
  
  // Create a table for the sales
  html += "<table border='1'><tr><th>Produkt</th><th>Anzahl</th></tr>";
  
  // Loop through the products and add them to the table
  for (int i = 0; i < productCount; i++) {
    html += "<tr><td>" + String(products[i].name) + "</td><td>" + String(totalSold[i]) + "</td></tr>";
  }
  Serial.print("productCount: ");
  Serial.println(productCount);
  for (int i = 0; i < productCount; i++) {
    Serial.print("Product ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(products[i].name);
    Serial.print(" - verkauft: ");
    Serial.println(totalSold[i]);
  }
  
  // Close the table tag
  html += "</table>";

  // Add the export CSV button
  html += "<form action='/exportSales' method='post'><button type='submit'>Exportiere Verkäufe als CSV</button></form>";

  // Add the reset sales button
  html += "<form action='/resetSales' method='post'><button type='submit'>Verkäufe zurücksetzen</button></form>";

  // Set the Content-Type header to UTF-8
  server.sendHeader("Content-Type", "text/html; charset=UTF-8");
  
  // Send the HTML response
  server.send(200, "text/html", html);
}


// Endpoint to handle CSV export
void handleExportSales() {
  String csvData = "Produkt,Anzahl\n";
  for (int i = 0; i < productCount; i++) {
    csvData += String(products[i].name) + "," + String(totalSold[i]) + "\n";
  }
  
  server.sendHeader("Content-Disposition", "attachment; filename=sales.csv");
  server.sendHeader("Content-Type", "text/csv");
  server.send(200, "text/csv", csvData);
}

// Endpoint to handle sales reset
void handleResetSales() {
  // Reset the sales data
  for (int i = 0; i < productCount; i++) {
    totalSold[i] = 0;
  }
  saveSalesToSD(); // Save the reset sales data to SD
  Serial.println("[handleResetSales] Sales data reset and saved to SD card.");
  
  // Redirect to the sales overview page after resetting
  server.sendHeader("Location", "/sales"); // Redirect to the sales page
  server.send(303); // Send a redirect response
  delay(1000); // Optional delay before restarting
  ESP.restart();
}



// HTML for product page
// HTML for product page
String generateProductList() {
  String content = "<div class='content-wrapper'>"; // Begin content wrapper

  // repeated for the number of products in the shop
  for (int i = 0; i < productCount; i++) {
    content += "<div class='product'>";
    content += "<p style='margin-top: 0;'><strong>" + String(products[i].name) + "</strong> (" + String(products[i].price, 2) + " €";
    if (products[i].hasDeposit) content += " + 1 € Pfand";
    content += ")</p>";
    content += "<div class='row'><div class='left'>";
    content += "<span>Anzahl: " + String(products[i].count) + "</span>";

    // add product buttons
    content += "<button onclick='sendAction(\"add\", " + String(i) + ", 1)' style='background-color: green; color: white;'>+1</button>"; // +1 Button
    content += "<button onclick='sendAction(\"add\", " + String(i) + ", 2)' style='background-color: green; color: white;'>+2</button>"; // +2 Button
    content += "<button onclick='sendAction(\"add\", " + String(i) + ", 3)' style='background-color: green; color: white;'>+3</button>"; // +3 Button

    content += "</div>";

    // -1 button on the right side of the row
    content += "<button onclick='sendAction(\"remove\", " + String(i) + ")' style='background-color: red; color: white;'>-1</button>";

    content += "</div>"; // line end
    content += "</div>"; // product block end
  }

  content += "</div>"; // End content wrapper

  // Add fixed footer container
  content += "<div class='fixed-footer'>";
  content += "<h3 class='bottom-interface'>" + String(calculateTotal(), 2) + " €<br>";
  content += "<small class='bottom-interface'>(inkl. " + String(calculateDeposit(), 2) + " € Pfand)</small></h3>";
  content += "<button class='bottom-interface' onclick='sendAction(\"clear\", -1)'>Warenkorb löschen</button>";
  content += "<button class='bottom-interface' onclick='sendAction(\"checkout\", -1)'>Bestellung abschließen</button>"; // Add the "Bestellung abschließen" button
  content += "</div>"; // End of footer container

  return content;
}



// configuration page HTML
String generateConfigPage() {
  // HTML template for the configuration page
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Konfiguration</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          padding: 20px;
          max-width: 600px;
          margin: auto;
          padding-bottom: 60px; /* Reserve space for footer */
          min-height: 100vh; /* Ensure the body takes at least the full screen height */
        }

        h1, h2 {
          text-align: center;
        }

        .product-config {
          border: 1px solid #ccc;
          border-radius: 15px;
          padding: 15px;
          margin-bottom: 10px;
          background-color: #f9f9f9;
        }

        label {
          display: block;
          margin-top: 8px;
        }

        input[type='text'], input[type='number'] {
          width: 100%;
          padding: 8px;
          margin-top: 4px;
          border-radius: 5px;
          border: 1px solid #ccc;
        }

        input[type='checkbox'] {
          margin-top: 6px;
        }

        button, input[type='submit'] {
          margin-top: 10px;
          padding: 8px 15px;
          border: none;
          border-radius: 10px;
          background-color: #007BFF;
          color: white;
          cursor: pointer;
        }

        button:hover, input[type='submit']:hover {
          background-color: #0056b3;
        }

        hr {
          margin-top: 20px;
        }

      </style>
    </head>
    <body>
    )rawliteral";    
  html += "<style>.input-field { width: 90%; box-sizing: border-box; }</style>";  // CSS fix for input fields to be 90% of the page width
  html += "<h1>Produktkonfiguration</h1><form method='POST' action='/saveConfig'>";
  // repeated for the number of products in the shop, adding the product name, price and deposit checkbox for each product
  for (int i = 0; i < productCount; i++) {
    html += "<div class='product-config'>";
    html += "<label>Name </label>";
    html += "<input class='input-field' type='text' name='name_" + String(i) + "' value='" + String(products[i].name) + "'><br>";
    html += "<label>Preis </label>";
    html += "<input class='input-field' type='number' step='0.01' name='price_" + String(i) + "' value='" + String(products[i].price, 2) + "'><br>";
    html += "<div style='display: flex; justify-content: space-between; align-items: center;'>";
    html += "<label>Pfand <input type='checkbox' name='deposit_" + String(i) + "'" + (products[i].hasDeposit ? " checked" : "") + "></label>";
    html += "<button type='button' style='background-color: red; color: white;' onclick='deleteProduct(" + String(i) + ")'>Produkt löschen</button>";
    html += "</div>"; // End of flex line
    html += "</div>"; // end of product config block
  }

  
  // Section for new Product at the end of the page
  html += "<h2>Neues Produkt</h2>";
  html += "<label>Name</label><input class='input-field' type='text' name='new_name'><br>";
  html += "<label>Preis</label><input class='input-field' type='number' step='0.01' name='new_price'><br>";
  html += "<label>Pfand<input type='checkbox' name='new_deposit'></label><br>";
  html += "<input type='submit' value='Speichern'></form>";

  html += "<script>function deleteProduct(id){fetch('/deleteProduct?id='+id).then(()=>location.reload());}</script>"; // delete product script for button (references the function in the HTML))

  // footer with copyright 
  html += "<footer style='text-align: center; margin-top: 20px; font-size: 12px; color: #888;'>";
  html += "&copy; 2025 Imanuel Fehse | Alle Rechte vorbehalten.";
  html += "</footer>";
  html += "</body></html>";
  return html;
}


// WEB SERVER HANDLER FUNCTIONS
// Port 80 product page
void handleRoot() {
  // HTML template for the product page
  loadSalesFromSD(); // load sales from SD
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Kasse</title>
    <style>
      body {
        font-family: Arial, sans-serif;
        padding: 20px;
        max-width: 600px;
        margin: auto;
      }
      h1, h3 {
        text-align: center;
      }
      .product {
        border: 1px solid #ccc;
        border-radius: 15px;
        padding: 10px;
        margin-bottom: 7px;
        background-color: #f9f9f9;
        margin-top: 0;
        padding-top: 0;
      }

      .product p {
        // add space between border and text
        margin-top: 10px;
        margin-bottom: 0;
        padding-top: 10px;
      }
      .row {
        display: flex;
        justify-content: space-between;
        align-items: center;
        margin-top: 5px;
        padding: 0;
      }
      .left {
        display: flex;
        align-items: center;
        gap: 10px;
      }
      button {
        font-size: 16px;
        padding: 5px 10px;
        margin-left: 5px;
        border-radius: 10px;
        border: none;
        color: white;
        cursor: pointer;
      }

      .button-green {
        background-color: green;
      }

      .button-red {
        background-color: red;
      }

      button:hover {
        background-color:rgb(116, 116, 116);
      }
      .clear-button {
        width: 100%;
        padding: 10px;
        background-color: red;
        color: white;
        font-size: 18px;
        border-radius: 10px;
        border: none;
        margin-top: 20px;
      }




      /* Fixed footer at the bottom of the screen */
      .fixed-footer {
        position: fixed;
        bottom: 0;
        left: 0;
        display: flex;
        background-color: #f9f9f9;
        border-top: 1px solid #ccc;
        padding: 5px;
        text-align: center;
        box-shadow: 0 -2px 5px rgba(0, 0, 0, 0.1);
      }

      .fixed-footer h3 {
        margin: 0;
        font-size: 16px;
      }

      .fixed-footer button {
        //margin-top: 5px;
        //padding: 5px 5px;
        background-color: red;
        color: white;
        border-radius: 10px;
        border: none;
        //cursor: pointer;
      }

      .fixed-footer button:last-child {
        background-color: #007BFF;
      }

      /* Allow scrolling content, but always show footer */
      .content-wrapper {
        margin-bottom: 70px; /* Make space for the fixed footer */
      }


    </style>
    <script>
      function updateContent(){
        fetch('/content').then(response => response.text()).then(html => {
          document.getElementById('content').innerHTML = html;
        });
      }

      function sendAction(action, id, quantity = 1){
        fetch(`/${action}?id=${id}&quantity=${quantity}`).then(() => updateContent());
      }

      window.onload = function() {
        updateContent();
      }
    </script>
  </head>
  <body>
    <h1>Kassensystem</h1>
    <div id="content">
      Lade Produkte...
    </div>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html); // send HTML to client
}

// add, remove, clear product functions
void handleAdd() {
  int id = server.arg("id").toInt();
  int q = server.arg("quantity").toInt();
  if (id >= 0 && id < productCount) products[id].count += q;
  server.send(200, "text/plain", "OK");
}

// remove product from cart
void handleRemove() {
  int id = server.arg("id").toInt();
  if (id >= 0 && id < productCount && products[id].count > 0) products[id].count--;
  server.send(200, "text/plain", "OK");
}

// clear all products in cart
void handleClear() {
  for (int i = 0; i < productCount; i++) products[i].count = 0;
  server.send(200, "text/plain", "OK");
}

// submit order to server and save to SD
void handleSubmit() {
  for (int i = 0; i < productCount; i++) {
    totalSold[i] += products[i].count;
    products[i].count = 0;
  }
  saveSalesToSD();
  server.send(200, "text/plain", "OK");

  // Verkäufe aktualisieren (already handled above)

  saveSalesToSD(); // Deine Funktion zum Speichern der Verkäufe
}


// update content of product page when action was performed by client (add, remove, clear)
void handleContent() {
  String content = "<div class='content-wrapper'>"; // Begin content wrapper

  // repeated for the number of products in the shop
  for (int i = 0; i < productCount; i++) {
    content += "<div class='product'>";
    content += "<p style='margin-top: 0;'><strong>" + String(products[i].name) + "</strong> (" + String(products[i].price, 2) + " €";
    if (products[i].hasDeposit) content += " + 1 € Pfand";
    content += ")</p>";
    content += "<div class='row'><div class='left'>";
    content += "<span>Anzahl: " + String(products[i].count) + "</span>";

    // add product buttons
    content += "<button onclick='sendAction(\"add\", " + String(i) + ", 1)' style='background-color: green; color: white;'>+1</button>"; // +1 Button
    content += "<button onclick='sendAction(\"add\", " + String(i) + ", 2)' style='background-color: green; color: white;'>+2</button>"; // +2 Button
    content += "<button onclick='sendAction(\"add\", " + String(i) + ", 3)' style='background-color: green; color: white;'>+3</button>"; // +3 Button

    content += "</div>";

    // -1 button on the right side of the row
    content += "<button onclick='sendAction(\"remove\", " + String(i) + ")' style='background-color: red; color: white;'>-1</button>";

    content += "</div>"; // line end
    content += "</div>"; // product block end
  }

  content += "</div>"; // End content wrapper

  // Add fixed footer container
  content += "<div class='fixed-footer'>";
  content += "<h3 class='bottom-interface'>" + String(calculateTotal(), 2) + " €<br>";
  content += "<small class='bottom-interface'>(inkl. " + String(calculateDeposit(), 2) + " € Pfand)</small></h3>";
  content += "<button class='bottom-interface' onclick='sendAction(\"clear\", -1)'>Warenkorb löschen</button>";
  content += "<button class='bottom-interface' onclick='sendAction(\"checkout\", -1)'>Bestellung abschließen</button>"; // Add the "Bestellung abschließen" button
  content += "</div>"; // End of footer container

  server.send(200, "text/html", content);
}





// Port 8080 configuration page
void handleConfig() {
  configServer.send(200, "text/html", generateConfigPage()); // send HTML to client
}

// save configuration page
// save new product to SD and update product list
void handleSaveConfig() {
  for (int i = 0; i < productCount; i++) {
    if (configServer.hasArg("name_" + String(i))) {
      String name = configServer.arg("name_" + String(i));
      name.toCharArray(products[i].name, sizeof(products[i].name));
      products[i].price = configServer.arg("price_" + String(i)).toFloat();
      products[i].hasDeposit = configServer.hasArg("deposit_" + String(i));
    }
  }
  if (configServer.hasArg("new_name") && configServer.arg("new_name").length() > 0 && productCount < MAX_PRODUCTS) {
    String name = configServer.arg("new_name");
    name.toCharArray(products[productCount].name, sizeof(products[productCount].name));
    products[productCount].price = configServer.arg("new_price").toFloat();
    products[productCount].hasDeposit = configServer.hasArg("new_deposit");
    products[productCount].count = 0;
    productCount++;
  }
  saveProductsToSD();
  configServer.sendHeader("Location", "/");
  configServer.send(303);
}

// delete product from SD and update product list
void handleDeleteProduct() {
  int id = configServer.arg("id").toInt();

  // Ensure the ID is within valid range
  if (id >= 0 && id < productCount) {
    // Shift products and sales data
    for (int i = id; i < productCount - 1; i++) {
      products[i] = products[i + 1];
      totalSold[i] = totalSold[i + 1]; // shift sales too
    }

    // Clear the last product for cleanup (optional)
    Product emptyProduct = {};
    products[productCount - 1] = emptyProduct;
    totalSold[productCount - 1] = 0;

    // Decrease the product count
    productCount--;

    // Save the updated products and sales to SD
    saveProductsToSD();
    saveSalesToSD();
  }

  // Send success response to the client
  configServer.send(200, "text/plain", "OK");
}



// SETUP
void setup() {
  // Onboard LED for status
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Serial and Wifi Module
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  Serial.println(String(color.blue) + "AP IP: " + WiFi.softAPIP().toString() + String(color.reset));
  Serial.println(String(color.blue) + "AP SSID: " + ssid + String(color.reset));
  initSD(); // initialize SD card

  loadProductsFromSD();
  if (productCount == 0) {
    Serial.println("No products found on SD, loading default products. productCount: " + String(productCount));
    for (int i = 0; i < defaultProductCount && i < MAX_PRODUCTS; i++) {
      products[i] = defaultProducts[i];
    }
    productCount = defaultProductCount;
    saveProductsToSD();
    saveSalesToSD();  
  }

  loadSalesFromSD(); 


  // Port 80
  server.on("/", handleRoot);
  server.on("/add", handleAdd);
  server.on("/remove", handleRemove);
  server.on("/clear", handleClear);
  server.on("/content", handleContent);
  server.on("/submit", handleSubmit);
  server.on("/sales", handleSalesOverview);
  server.on("/resetSales", HTTP_POST, handleResetSales);
  server.on("/exportSales", HTTP_POST, handleExportSales);
  server.onNotFound([]() {
    server.send(404, "text/plain", "404 Not Found");
  });

  
  


  // Port 8080
  configServer.on("/", handleConfig);
  configServer.on("/saveConfig", HTTP_POST, handleSaveConfig);
  configServer.on("/deleteProduct", handleDeleteProduct);

  server.begin();       // launch product page server so client can request page
  configServer.begin(); // launch config page server so client can request page
  Serial.println(String(color.green) + "servers started successfully" + String(color.reset));

  Serial.println("product page running on port 80");
  Serial.println("config page running on port 8080");

  Serial.println("\n " + String(color.green) + "Setup complete." + String(color.reset));
  Serial.println("Waiting for client requests...\n");

}


// LOOP
void loop() {
  // Status LED, not blocking webservers so client action is not delayed
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    digitalWrite(LED_PIN, HIGH);  // LED an
    ledOn = true;
  }

  // LED blinking
  if (ledOn && millis() - previousMillis >= 100) {
    digitalWrite(LED_PIN, LOW);   // LED aus
    ledOn = false;
  }

  // Webservers looking for client requests
  server.handleClient();        // product page client handler
  configServer.handleClient();  // config page client handler
}
