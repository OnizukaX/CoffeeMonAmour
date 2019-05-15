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
Remote::RemoteConfig remoteCfg = {
  .ssid       = DEFAULT_SSID,
  .password   = DEFAULT_PASSWORD,
  .serverUrl  = "script.google.com",
  .googleKey  = "XXX"
};

/* Reader configuration. */
Reader::ReaderConfig readerCfg = {
  .chipSelectPin      = 21,
  .resetPowerDownPin  = 22,
  .readPeriod_ms      = 2000
};

/* Hmi configuration. */
Hmi::HmiConfig hmiCfg = {
  .address           = 0x3C,
  .sdaPin            = 4,
  .sdcPin            = 5,
  .touchButtonPin    = T7,
  .touchButtonThresh = 30, /* Thresh=30 took empirically. */
  .wifiStatusPin     = 25,
  .dataStatusPin     = 26,
  .errorStatusPin    = 33
};

/* Remote server to communicate with. */
Remote remote(remoteCfg, &log);
/* RFID Card reader. */
Reader reader(readerCfg, &log);
/* Hmi. */
Hmi hmi(hmiCfg);

void setup()
{
  /* Init. serial at 115200 bps. */
  Serial.begin(115200);
  while (!Serial);

  /*
  * Workers setup.
  */
  /* HMI */
  hmi.setup();
  String displayLogs("HMI setup...");
  hmi.write(displayLogs);
  /* Switching on all LEDs. */
  hmi.setWifiStatusLight(true);
  hmi.setDataStatusLight(true);
  hmi.setErrorStatusLight(true);
  hmi.write(displayLogs += "ok\n");
  /* Reader */
  hmi.write(displayLogs += "RFID reader setup...");
  reader.setup();
  hmi.write(displayLogs += "ok\n");
  /* Remote */
  hmi.write(displayLogs += "Connecting to WiFi...");
  bool initRemote = remote.setup();
  hmi.write(displayLogs += (initRemote) ? "ok" : "error");
  delay(500);

  /* Switching off all LEDs. */
  hmi.setWifiStatusLight(false);
  hmi.setDataStatusLight(false);
  hmi.setErrorStatusLight(false);
  log("[SETUP] done");
}

void loop()
{
  if (remote.isConnected())
  {
    /* Blinking to know if the µC is still running. */
    hmi.setWifiStatusLight((millis() % 1000) > 500);

    if (reader.readUID()) /* Card read. */
    {
      hmi.setDataStatusLight(true);
      hmi.setWifiStatusLight(false);
      hmi.write("Card detected, transmitting.");

      String urlBase = remote.getBaseUrl();
      String urlParameters = String("CoffeeMachineID=1&EmployeeCardID=") + reader.getUID();

      String url = urlBase + urlParameters;
      if (remote.sendData(url))
      {
        log("[DATA] ok");
        hmi.write("Data transmitted.");
        /* Makes the LED blink to show that data have been transmitted. */
        for (int i = 0; i < 6; ++i)
        {
          hmi.setDataStatusLight(i% 2);
          delay(300);
        }
      }
      else
      {
        log("[DATA] error");
        hmi.write("Error.");
        hmi.setErrorStatusLight(true);
        delay(1000);
      }
    }
    else
    {
      /* No new card detected. */
      hmi.setDataStatusLight(false);

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
    hmi.setErrorStatusLight(true);
    hmi.write("Not connected...");
  }

  /* Switch off all HMI outputs. */
  hmi.setWifiStatusLight(false);
  hmi.setDataStatusLight(false);
  hmi.setErrorStatusLight(false);
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
