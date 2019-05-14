#include "Remote.hpp"
#include "Reader.hpp"
#include "Hmi.hpp"

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
  | GPIO-33 |       |      |  R  |        |
  | GPIO-26 |       |      |  G  |        |
  | GPIO-25 |       |      |  B  |        |
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
#define WIFI_STATUS_PIN (25)
#define DATA_STATUS_PIN (26)
#define ERROR_PIN       (33)

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

/* Hmi configuration. */
struct HmiConfig
{
  byte address =  0x3C;
  byte sda =      4;
  byte sdc =      5;
} hmiCfg;

/* Remote server to communicate with. */
Remote remote(remoteCfg.ssid, remoteCfg.password, remoteCfg.serverUrl, remoteCfg.googleKey, &log);
/* RFID Card reader. */
Reader reader(readerCfg.chipSelectPin, readerCfg.resetPowerDownPin, readerCfg.readPeriod_ms, &log);
/* Hmi. */
Hmi hmi(hmiCfg.address, hmiCfg.sda, hmiCfg.sdc);

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
  hmi.setup();
  hmi.configureTouchButton();
  hmi.write("HMI ok");
  reader.setup();
  hmi.write("Reader ok");
  delay(500);
  hmi.write("Connecting...");
  bool initRemote = remote.setup();
  hmi.write((initRemote) ? "Remote ok" : "Not connected");
  delay(500);

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
      hmi.write("Card detected, transmitting.");

      String urlBase = remote.getBaseUrl();
      String urlParameters = String("CoffeeMachineID=1&EmployeeCardID=") + reader.getUID();

      String url = urlBase + urlParameters;
      if (remote.sendData(url))
      {
        log("[DATA] ok");
        hmi.write("Data transmitted.");
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
        hmi.write("Error.");
        digitalWrite(ERROR_PIN, HIGH);
        delay(1000);
      }
    }
    else
    {
      /* No new card detected. */
      digitalWrite(DATA_STATUS_PIN, LOW);

      /* Button status. */
      if (hmi.isButtonPressed())
      {
        hmi.write("Button is pressed");
      }
      else
      {
        hmi.write("Waiting for new card...");
      }
    }
  }
  else
  {
    /* Do nothing: not connected to remote. */
    digitalWrite(ERROR_PIN, HIGH);
    hmi.write("Not connected...");
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
