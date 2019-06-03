#include <WiFiClientSecure.h>

class Remote : public WiFiSTAClass
{
public:
  struct RemoteConfig
  {
    String ssid;
    String password;
    String serverRootUrl;
    String serverScriptUrl;
  };

  Remote(const RemoteConfig& cfg,
         void (*f_logFunPtr_p)(const String&) = nullptr,
         const String f_tag = "[REMOTE]");

  virtual ~Remote() {}

  /* Setup. */
  bool setup(uint8_t cxAttempts = 10);
  /* Send data to remote. */
  bool sendData(const String& f_url, String& f_scriptResponse);
  /* Get script URL without parameters. */
  inline const String& getScriptUrl() const { return m_serverScriptUrl; }

private:
  const String m_ssid;                  /* AP SSID. */
  const String m_password;              /* AP password. */
  const String m_serverRootUrl;         /* Server root URL. */
  const String m_serverScriptUrl;       /* Server page URL. */
  void (*m_logFunPtr_p)(const String&); /* Function pointer for logging. */
  const String m_tag;                   /* Tag used as prefix in logs. */
  WiFiClientSecure m_client;            /* WiFi handle. */

  /* Logging function. */
  void log(const String& msg) const;
};
