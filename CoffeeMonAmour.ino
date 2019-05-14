#include "Remote.hpp"
#include "Reader.hpp"
#include "Display.hpp"

/*
  Default pins layout.
  +---------+-------+------+-----+--------+
  | NodeMCU | RC522 | OLED | LED | Button |
  +---------+-------+------+-----+--------|
  | GPIO-21 |  SDA  |      |     |        |
  | GPIO-18 |  SCK  |      |     |        |
  | GPIO-23 |  MOSI |      |     |        |
  | GPIO-19 |  MISO |      |     |        |
  | GPIO-22 |  RST  |      |     |        |
  | GPIO-5  |       | SCK  |     |        |
  | GPIO-4  |       | SDA  |     |        |
  | GPIO-0  |       |      |  R  |        |
  | GPIO-15 |       |      |  G  |        |
  | GPIO-2  |       |      |  B  |        |
  | GPIO-27 |       |      |     |  Touch |
  +---------+-------+------+-----+--------+
  The RC522 and the OLED display are powered with 3.3V.
  The LEDs are powered with 5.0V.
*/

/* Print debug data. */
#define DEBUG_ON (true)

#define DEFAULT_SSID "XXX"
#define DEFAULT_PASSWORD "XXX"

/* HMI pins e.g. LEDs. */
#define WIFI_STATUS_PIN (2)
#define DATA_STATUS_PIN (15)
#define ERROR_PIN       (0)

/* Remote configuration. */
struct RemoteConfig
{
  String ssid =       DEFAULT_SSID;
  String password =   DEFAULT_PASSWORD;
  String serverUrl =  "script.google.com";
  String googleKey =  "XXX";
} remoteCfg;

/* Reader configuration. */
struct ReaderConfig
{
  byte chipSelectPin =          21;
  byte resetPowerDownPin =      22;
  unsigned long readPeriod_ms = 2000;
} readerCfg;

/* Display configuration. */
struct DisplayConfig
{
  byte address =  0x3C;
  byte sda =      4;
  byte sdc =      5;
} displayCfg;

/* Remote server to communicate with. */
Remote remote(remoteCfg.ssid, remoteCfg.password, remoteCfg.serverUrl, remoteCfg.googleKey, &log);
/* RFID Card reader. */
Reader reader(readerCfg.chipSelectPin, readerCfg.resetPowerDownPin, readerCfg.readPeriod_ms, &log);
/* Display. */
Display display(displayCfg.address, displayCfg.sda, displayCfg.sdc);

void setup()
{
  /* Init. serial at 115200 bps. */
  Serial.begin(115200);
  while (!Serial);

  /* Pins setup. */
  pinMode(WIFI_STATUS_PIN, OUTPUT);
  pinMode(DATA_STATUS_PIN, OUTPUT);
  pinMode(ERROR_PIN, OUTPUT);

  /* Workers setup. */
  remote.setup();
  reader.setup();
  display.setup();
  display.configureTouchButton();

  log("[SETUP] done");
}

void loop()
{
  if (remote.isConnected())
  {
    /* Flickering to know if the µC is still running. */
    digitalWrite(WIFI_STATUS_PIN, (millis() % 1000) > 500);

    if (reader.readUID())
    {
      digitalWrite(DATA_STATUS_PIN, HIGH);
      digitalWrite(WIFI_STATUS_PIN, HIGH);
      display.write("Card detected, transmitting.");

      String urlBase = remote.getBaseUrl();
      String urlParameters = String("CoffeeMachineID=1&EmployeeCardID=") + reader.getUID();

      String url = urlBase + urlParameters;
      if (remote.sendData(url))
      {
        log("[DATA] ok");
        display.write("Data transmitted.");
        /* Makes the LED flicker to show that data have been transmitted. */
        for (int i = 0; i < 6; ++i)
        {
          digitalWrite(DATA_STATUS_PIN, i%2);
          delay(300);
        }
      }
      else
      {
        log("[DATA] error");
        display.write("Error.");
        digitalWrite(ERROR_PIN, HIGH);
        delay(1000);
      }
    }
    else
    {
      /* No new card detected. */
      digitalWrite(DATA_STATUS_PIN, LOW);

      /* Button status. */
      if (display.isButtonPressed())
      {
        display.write("Button is pressed");
      }
      else
      {
        display.write("Waiting for new card...");
      }
    }
  }
  else
  {
    /* Do nothing: not connected to remote. */
    digitalWrite(WIFI_STATUS_PIN, LOW);
    display.write("Not connected...");
  }

  /* Switch off all HMI outputs. */
  digitalWrite(WIFI_STATUS_PIN, LOW);
  digitalWrite(DATA_STATUS_PIN, LOW);
  digitalWrite(ERROR_PIN, LOW);
}

/* Print out debug info. */
void log(String msg)
{
  if (DEBUG_ON)
  {
    Serial.println(msg);
  }
  else
  {
    /* Do nothing. */
  }
}
