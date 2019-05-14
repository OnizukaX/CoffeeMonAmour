#include <WiFiClientSecure.h>

#include "Remote.hpp"

#define TIMEOUT_MS (1500)

Remote::Remote(const RemoteConfig& cfg,
               void (*f_logFunPtr_p)(String),
               String f_tag) :
  m_ssid(cfg.ssid),
  m_password(cfg.password),
  m_serverUrl(cfg.serverUrl),
  m_googleKey(cfg.googleKey),
  m_logFunPtr_p(f_logFunPtr_p),
  m_tag(f_tag),
  m_client(WiFiClientSecure()) {}

bool Remote::setup(uint8_t cxAttempts)
{
  WiFi.begin(m_ssid.c_str(), m_password.c_str());
  WiFi.setAutoReconnect(true);

  log("Waiting for connection");
  while (!WiFi.isConnected() && (0 != cxAttempts))
  {
    delay(500);
    log(".");
    --cxAttempts;
  }

  if (WiFi.isConnected())
  {
    log("IP Addresse: " + WiFi.localIP().toString());
    log("setup done");
  }
  else
  {
    log("Not connected with remote.");
  }

  return WiFi.isConnected();
}

/* Send data to remote. */
bool Remote::sendData(String f_url)
{
  bool success;
  String movedURL;
  String line;
  unsigned long start_ms;

  log("Connecting to " + m_serverUrl);

  if (m_client.connect(m_serverUrl.c_str(), 443))
  {
    log("Connected.");

    /* Craft a HTTPS request for the remote. */
    m_client.println("GET " + f_url);
    m_client.println("Host: " + m_serverUrl);
    m_client.println("Connection: close");
    m_client.println();

    // The remote replies.
    /* Headers received. */
    log("+++ headers +++");
    start_ms = millis();
    while (m_client.connected() && (abs(millis() - start_ms) < TIMEOUT_MS))
    {
      line = m_client.readStringUntil('\n');
      log(line);
      if (line.equals("\r"))
      {
        break; /* End of header transmission. */
      }
      if (line.indexOf("Location") >= 0) /*Header redirection. */
      {
        movedURL = line.substring(line.indexOf(":") + 2 ) ; /* Retain new URL. */
      }
    }
    log("--- headers ---");

    /* Read HTML response. */
    log("+++ response +++");
    start_ms = millis();
    while (m_client.connected() && (abs(millis() - start_ms) < TIMEOUT_MS))
    {
      if (m_client.available())
      {
        line = m_client.readStringUntil('\r');
        log(line);
      }
    }
    log("--- response ---");
    m_client.stop();

    movedURL.trim();
    if (!movedURL.equals(""))
    {
      log("Redirection to URL: \"" + movedURL + "\"");

      if (movedURL.length() < 10)
      {
        success = false; /* Redirection missing. */
      }
      else
      {
        log("Start redirection...");
        if (m_client.connect(m_serverUrl.c_str(), 443))
        {
          log("Connected.");
          /* Craft HTTPS request for the Remote. */
          m_client.println("GET " + movedURL);
          m_client.println("Host: " + m_serverUrl);
          m_client.println("Connection: close");
          m_client.println();

          /* Headers received. */
          log("+++ headers +++");
          start_ms = millis();
          while (m_client.connected() && (abs(millis() - start_ms) < TIMEOUT_MS))
          {
            line = m_client.readStringUntil('\n');
            log(line);
            if (line.equals("\r"))
            {
              break;
            }
          }
          log("--- headers ---");

          /* Read HTML response. */
          log("+++ response +++");
          start_ms = millis();
          while (m_client.connected() && (abs(millis() - start_ms) < TIMEOUT_MS))
          {
            if (m_client.available())
            {
              line = m_client.readStringUntil('\r');
              log(line);
              line.trim();
              line.toLowerCase();
              if (line.endsWith("</html>"))
              {
                log("Reached end of document.");
                break;
              }
            }
          }
          log("--- response ---");
          m_client.stop();
          
          log("Accessed URL: " + f_url);
          success = true;
        }
        else
        {
          log("Redirection impossible.");
          success = false;
        }
      }
    }
    else
    {
      success = true; /* Ok, no redirection. */
    }
  }
  else
  {
    log("Connection impossible.");
    success = false;
  }

  return success;
}

/* Get URL without parameters. */
String Remote::getBaseUrl() const
{
  return String("https://script.google.com/macros/s/") + m_googleKey + String("/exec?");
}

/* Logging function. */
void Remote::log(String msg) const
{
  if (nullptr == m_logFunPtr_p)
  {
    /* Do nothing. */
  }
  else
  {
    (*m_logFunPtr_p)(m_tag + " " + msg);
  }
} 
