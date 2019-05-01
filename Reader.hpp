#include <SPI.h>
#include <MFRC522.h>

class Reader
{
public:
  Reader(byte f_chipSelectPin = 21,
         byte f_resetPowerDownPin = 22,
         unsigned long f_readPeriod_ms = 2000,
         void (*f_logFunPtr_p)(String) = nullptr,
         String f_tag = "[READER]");

  virtual ~Reader() {}

  /* Setup. */
  void setup();
  /* Read card UID. */
  bool readUID();
  /* Returns card UID. */
  String getUID() const
  { return m_uid; }

private:
  MFRC522 m_mfrc522;                  /* MFRC522 handle. */
  MFRC522::MIFARE_Key m_mifare_key;   /* MIFARE key object. */
  String m_uid;                       /* MIFARE uuid. */
  unsigned long m_readPeriod_ms;      /* Period between two reads. */
  unsigned long m_readPrevTstamp_ms;  /* Retain previous read timestamp. */
  void (*m_logFunPtr_p)(String);      /* Function pointer for logging. */
  String m_tag;                       /* Tag used as prefix in logs. */

  /* Returns true if the period for the next read is elapsed, false otherwise. */
  bool readPeriodElapsed() const
  { return (abs(millis() - m_readPrevTstamp_ms) >= m_readPeriod_ms); }
  /* Logging function. */
  void log(String msg) const;
};
