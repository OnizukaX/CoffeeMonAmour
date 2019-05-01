#include "Remote.hpp"
#include "Reader.hpp"

/* Print debug data. */
#define DEBUG_ON (true)

#define DEFAULT_SSID "XXX"
#define DEFAULT_PASSWORD "XXX"

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

/* Remote server to communicate with. */
Remote remote(remoteCfg.ssid, remoteCfg.password, remoteCfg.serverUrl, remoteCfg.googleKey, &log);
/* RFID Card reader. */
Reader reader(readerCfg.chipSelectPin, readerCfg.resetPowerDownPin, readerCfg.readPeriod_ms, &log);

void setup()
{
  /* Init. serial at 115200 bps. */
  Serial.begin(115200);
  while (!Serial);
  
  /* Workers setup. */
  remote.setup();
  reader.setup();

  log("[SETUP] done");
}

void loop()
{
  if (remote.isConnected())
  {
    if (reader.readUID())
    {
      String urlBase = remote.getBaseUrl();
      String urlParameters = String("CoffeeMachineID=1&EmployeeCardID=") + reader.getUID();
      
      String url = urlBase + urlParameters;
      if (remote.sendData(url))
      {
        log("[DATA] ok");
      }
      else
      {
        log("[DATA] error");
      }
    }
    else
    {
      /* Do nothing: no new card detected. */
    }
    
  }
  else
  {
    /* Do nothing: not connected to remote. */
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
