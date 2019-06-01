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
  The LEDs@20Ma are powered with 3.3V and each of them is in series with a 100Ohm resistors.
*/

/* Determine on which Core Arduino is running. */
#if CONFIG_FREERTOS_UNICORE
# define ARDUINO_RUNNING_CORE 0
#else
# define ARDUINO_RUNNING_CORE 1
#endif

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

/* State-machine. */
enum StateMachine
{
  NONE = 0,
  INIT_START,
  INIT_FINISHED,
  WAITING_FOR_CARD,
  TICKING_COFFEE,
  BALANCE_ENQUIRED,
  BALANCE_RETRIEVAL,
  ENQUIRING_BALANCE,
  DATA_SUCCESS,
  DATA_FAILURE,
  CONNECTION_FAILURE
};

struct GlobalParams
{
  StateMachine state;
  unsigned long enquiryTimeLeft_ms;
  String scriptResponse;
};

GlobalParams gParams = {
  .state = StateMachine::NONE,
  .enquiryTimeLeft_ms = 0,
  .scriptResponse = ""
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

  /* Handling Hmi on Arduino Core. Arduino sketch runs on Core1 by default.
    note: Core1 seems to be the best for Serial I/O.
    https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/freertos.html */
  xTaskCreatePinnedToCore(hmiWrapper, "hmiWrapper", 10000, NULL, 1, NULL, ARDUINO_RUNNING_CORE);

  /*
  * Workers setup.
  */
  /* HMI */
  hmi.setup();
  gParams.state = INIT_START;
  String displayLogs("HMI setup...");
  hmi.writeSmall(displayLogs += "ok\n");
  /* Reader */
  hmi.writeSmall(displayLogs += "RFID reader setup...");
  reader.setup();
  hmi.writeSmall(displayLogs += "ok\n");
  /* Remote */
  hmi.writeSmall(displayLogs += "Connecting to WiFi...");
  bool initRemote = remote.setup();
  hmi.writeSmall(displayLogs += (initRemote) ? "ok" : "error");
  gParams.state = INIT_FINISHED;
  delay(500);
  hmi.clear();

  log("[SETUP] done");
}

void loop()
{
  if (remote.isConnected())
  {
    if (reader.readUID()) /* Card read. */
    {
      String urlParameters = String("CoffeeMachineID=1&EmployeeCardID=") + reader.getUID();
      if (hmi.isBalanceEnquiryActive(gParams.enquiryTimeLeft_ms))
      {
        gParams.state = BALANCE_RETRIEVAL;
        urlParameters += "&Balance=1";
      }
      else
      {
        gParams.state = TICKING_COFFEE;
      }

      String url = remote.getBaseUrl() + urlParameters;
      
      if (remote.sendData(url, gParams.scriptResponse))
      {
        if (TICKING_COFFEE == gParams.state)
        {
          // Hard-coding the server reply: sometimes, the reply does not have the expected format.
          gParams.scriptResponse = "OK";
        }

        gParams.state = DATA_SUCCESS;
        log("[DATA] data transmitted");
      }
      else
      {
        gParams.state = DATA_FAILURE;
        log("[DATA] error");
      }
    }
    else
    {
      /* Button status. */
      if (hmi.isButtonPressed())
      {
        hmi.setBalanceEnquiry(millis());
        gParams.state = BALANCE_ENQUIRED;
      }
      else if (hmi.isBalanceEnquiryActive(gParams.enquiryTimeLeft_ms))
      {
        gParams.state = BALANCE_ENQUIRED;
      }
      else
      {
        hmi.setBalanceEnquiry(0);
        /* No new card detected. */
        gParams.state = WAITING_FOR_CARD;
      }
    }
  }
  else
  {
    /* Error: not connected to remote. */
    gParams.state = CONNECTION_FAILURE;
  }
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

/* Handle delegated HMI task. */
void hmiWrapper(void* pvParameters)
{
  while (true)
  {
    /* Required to avoid Task Watchdog Timer (TWDT) being triggered when loop executes nothing (not yielding).
      note: disableCoreXWDT() seems not to be a good solution for this. */
    delay(10);
    switch (gParams.state)
    {
      case NONE:
        /* Do nothing: HMI not even initialized. */
        break;
      case INIT_START: /* Setup method called, HMI must first be initialized. */
        hmi.setWDEStatusLights(true, true, true);
        break;
      case INIT_FINISHED: /* Setup method finished. */
        hmi.setWDEStatusLights(false, false, false);
        break;
      case WAITING_FOR_CARD:
        /* Blinking to know if the µC is still running. */
        hmi.setWDEStatusLights((millis() % 1000) > 500, false, false);
        hmi.writeBig("Swipe card");
        break;
      case TICKING_COFFEE: /* Sending data to remote. */
        hmi.setWDEStatusLights(false, true, false);
        hmi.fillCoffeeCup();
        break;
      case BALANCE_ENQUIRED: /* Button pressed. */
        hmi.clear();
        hmi.writeSmall("Balance enquiry...", false);
        hmi.drawProgressBar((gParams.enquiryTimeLeft_ms * 100) / hmi.getEnquiryTimeout());
        hmi.display();
        break;
      case BALANCE_RETRIEVAL: /* Request being processed. */
        hmi.setWDEStatusLights(false, true, false);
        hmi.clear();
        hmi.writeSmall("Retrieving balance...", false);
        static const uint16_t speed = 8 * 1000;
        hmi.drawProgressBar((100 * (speed - (millis() % speed))) / speed);
        hmi.display();
        break;
      case DATA_SUCCESS: /* Data successfully transmitted. */
        hmi.setWDEStatusLights(false, true, false);
        hmi.writeBig(gParams.scriptResponse);
        /* Makes the LED blink to show that data have been transmitted. */
        for (int i = 0; i < 6; ++i)
        {
          hmi.setDataStatusLight(i% 2);
          delay(300);
        }
        delay(1000);
        break;
      case DATA_FAILURE:
        hmi.setWDEStatusLights(false, false, true);
        hmi.writeBig("Error!");
        delay(1000);
        break;
      case CONNECTION_FAILURE: /* Could either not connect to W-LAN or no internet connection. */
        hmi.setWDEStatusLights(false, false, true);
        hmi.writeBig("WiFi issue!");
        break;
      default:
        /* Unexpected -> error. */
        hmi.setWDEStatusLights(false, false, true);
        hmi.writeBig("???");
        break;
    }
  }
}
