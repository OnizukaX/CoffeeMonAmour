#include <WiFiClientSecure.h>

class Remote : public WiFiSTAClass
{
public:
  struct RemoteConfig
  {
    String ssid;
    String password;
    String serverUrl;
    String googleKey;
  };

  Remote(const RemoteConfig& cfg,
         void (*f_logFunPtr_p)(String) = nullptr,
         String f_tag = "[REMOTE]");

  virtual ~Remote() {}

  /* Setup. */
  bool setup(uint8_t cxAttempts = 10);
  /* Send data to remote. */
  bool sendData(String f_url);
  /* Get URL without parameters. */
  String getBaseUrl() const;

private:
  String m_ssid;                  /* AP SSID. */
  String m_password;              /* AP password. */
  String m_serverUrl;             /* Google Server URL. */
  String m_googleKey;             /* Google Script key. */
  void (*m_logFunPtr_p)(String);  /* Function pointer for logging. */
  String m_tag;                   /* Tag used as prefix in logs. */
  WiFiClientSecure m_client;      /* WiFi handle. */

  /* Logging function. */
  void log(String msg) const;
};
