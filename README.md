# CoffeeMonAmour

For aficionados sharing coffee machines e.g. in a company using RFID authentication badges.

A good office day usually starts with a cup of your favorite beverage. You go to the nearest coffee machine, select your favorite beverage and look at it slowly filling up your favorite mug. Then, possibly, you will start looking for your name on a sheet of paper and put a tick under it, you will later have to pay your dues. Then as the day goes, you run from one meeting to another one, and frequently stop by other coffee machines ran by other coffee aficionados. The process repeats itself again and again.
Keeping finances up to date slowly becomes complicated and unpaid paper sheets soon become your new paperwall.

Without any hardware modification, you can add extra devices that track user accross all the coffee machines.

## DataFlow
NodeMCU + RFID reader + MIFARE Classic card --(WiFi)--> Google Script --> Google Spreadsheet --> Google Script --(WiFi)--> NodeMCU

## Hardware
* 1x NodeMCU ESP-WROOM-32 [https://cdn.shopify.com/s/files/1/1509/1638/files/ESP_-_32_NodeMCU_Developmentboard_Datenblatt_a3bf98d8-6a53-4d26-8f1c-c61b1c82af39.pdf?76837, https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf]
* 1x RFID-RC522 [
https://cdn.shopify.com/s/files/1/1509/1638/files/RFID_RC522_Set_mit_Reader_Chip_und_Card_13_56MHz_Datenblatt.pdf?2098853292011892896, https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf]
* 1x MIFARE Classic card [https://www.nxp.com/docs/en/data-sheet/MF1S50YYX_V1.pdf]
* 3x LEDs (red, green and blue)

## Libraries
* SPI [ESP32]
* WiFiClientSecure [ESP32]
* MFRC522 [https://github.com/miguelbalboa/rfid]
* SSD1306 [https://github.com/ThingPulse/esp8266-oled-ssd1306]

## Credits for pulling things together
* "ESP32 and RFID-RC522 module example" [http://www.esp32learning.com/code/esp32-and-rfid-rc522-module-example.php]
* "ESP32 und Google Tabellen (Sheets)" [http://esp32-server.de/google-sheets/]
