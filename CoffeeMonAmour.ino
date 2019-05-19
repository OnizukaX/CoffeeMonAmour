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
#define DEBUG_ON (false)

#define DEFAULT_SSID "XXX"
#define DEFAULT_PASSWORD "XXX"

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
  hmi.writeSmall(displayLogs);
  /* Switching on all LEDs. */
  hmi.setWifiStatusLight(true);
  hmi.setDataStatusLight(true);
  hmi.setErrorStatusLight(true);
  hmi.writeSmall(displayLogs += "ok\n");
  /* Reader */
  hmi.writeSmall(displayLogs += "RFID reader setup...");
  reader.setup();
  hmi.writeSmall(displayLogs += "ok\n");
  /* Remote */
  hmi.writeSmall(displayLogs += "Connecting to WiFi...");
  bool initRemote = remote.setup();
  hmi.writeSmall(displayLogs += (initRemote) ? "ok" : "error");
  delay(500);
  hmi.clear();

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
    unsigned long enquiryTimeLeft_ms;
    /* Blinking to know if the µC is still running. */
    hmi.setWifiStatusLight((millis() % 1000) > 500);

    if (reader.readUID()) /* Card read. */
    {
      hmi.setDataStatusLight(true);
      hmi.setWifiStatusLight(false);

      String urlBase = remote.getBaseUrl();
      String urlParameters = String("CoffeeMachineID=1&EmployeeCardID=") + reader.getUID();
      if (hmi.isBalanceEnquiryActive(enquiryTimeLeft_ms))
      {
        urlParameters += "&Balance=1";
        hmi.writeBig("Retrieving\nbalance...");
      }
      else
      {
        hmi.writeBig("Ticking...");
      }

      String url = urlBase + urlParameters;
      String scriptResponse;
      if (remote.sendData(url, scriptResponse))
      {
        log("[DATA] data transmitted");
        hmi.writeBig(scriptResponse);
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
        hmi.writeBig("Error!");
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
        hmi.setBalanceEnquiry(millis());
      }
      else if (hmi.isBalanceEnquiryActive(enquiryTimeLeft_ms))
      {
        hmi.write("Balance enquiry...\n" +
                  String((enquiryTimeLeft_ms / 1000)) + "s left for swipping");
      }
      else
      {
        hmi.setBalanceEnquiry(0);
        hmi.writeBig("Swipe card");
      }
    }
  }
  else
  {
    /* Do nothing: not connected to remote. */
    hmi.setErrorStatusLight(true);
    hmi.writeBig("WiFi issue!");
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
