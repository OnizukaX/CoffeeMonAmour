#include <SPI.h>
#include <MFRC522.h>

class Reader
{
public:
  struct ReaderConfig
  {
    byte chipSelectPin;
    byte resetPowerDownPin;
    unsigned long readPeriod_ms;
  };

  Reader(const ReaderConfig& cfg,
         void (*f_logFunPtr_p)(const String&) = nullptr,
         const String f_tag = "[READER]");

  virtual ~Reader() {}

  /* Setup. */
  void setup();
  /* Read card UID. */
  bool readUID();
  /* Returns card UID. */
  inline const String& getUID() const { return m_uid; }

private:
  MFRC522 m_mfrc522;                    /* MFRC522 handle. */
  MFRC522::MIFARE_Key m_mifare_key;     /* MIFARE key object. */
  String m_uid;                         /* MIFARE uuid. */
  const unsigned long m_readPeriod_ms;  /* Period between two reads. */
  unsigned long m_readPrevTstamp_ms;    /* Retain previous read timestamp. */
  void (*m_logFunPtr_p)(const String&); /* Function pointer for logging. */
  const String m_tag;                   /* Tag used as prefix in logs. */

  /* Returns true if the period for the next read is elapsed, false otherwise. */
  inline bool readPeriodElapsed() const { return (abs(millis() - m_readPrevTstamp_ms) >= m_readPeriod_ms); }
  /* Logging function. */
  void log(const String& msg) const;
};
