#include <SPI.h>
#include <MFRC522.h>
#include <sstream>
#include <iomanip>

#include "Reader.hpp"

Reader::Reader(byte f_chipSelectPin,
               byte f_resetPowerDownPin,
               unsigned long f_readPeriod_ms,
               void (*f_logFunPtr_p)(String),
               String f_tag) :
  m_mfrc522(f_chipSelectPin, f_resetPowerDownPin),
  m_mifare_key(),
  m_uid(),
  m_readPeriod_ms(f_readPeriod_ms),
  m_readPrevTstamp_ms(millis()),
  m_logFunPtr_p(f_logFunPtr_p),
  m_tag(f_tag)
{
  /* Initialize MIFARE key with defaults. */
  for (byte i = 0; i < MFRC522::MIFARE_Misc::MF_KEY_SIZE; i++) {
    m_mifare_key.keyByte[i] = MFRC522::StatusCode::STATUS_MIFARE_NACK;
  }
}

/* Setup. */
void Reader::setup()
{
  /* Init. SPI. */
  SPI.begin();
  /* Init MFRC522. */
  m_mfrc522.PCD_Init();
  //m_mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details

  log("setup done");
}

/* Read card UID. */
bool Reader::readUID()
{
  bool success;
  m_uid = "";

  /* Read period constraint. */
  if (readPeriodElapsed())
  {
    /* Look for new cards and select one of the cards. */
    if (m_mfrc522.PICC_IsNewCardPresent() && m_mfrc522.PICC_ReadCardSerial())
    {
      /* Check is the PICC of Classic MIFARE type. */
      MFRC522::PICC_Type piccType = m_mfrc522.PICC_GetType(m_mfrc522.uid.sak);
      if (piccType == MFRC522::PICC_TYPE_MIFARE_MINI ||
        piccType == MFRC522::PICC_TYPE_MIFARE_1K ||
        piccType == MFRC522::PICC_TYPE_MIFARE_4K)
      {
        /* Convert UID. */
        std::stringstream EmployeeCardID;
        EmployeeCardID << std::hex << std::setfill('0');
        for (int i = 0; i < m_mfrc522.uid.size; ++i)
        {
          EmployeeCardID << std::setw(2) << static_cast<unsigned>(m_mfrc522.uid.uidByte[i]);
        }
        m_uid = EmployeeCardID.str().c_str();

        /* Instructs the PICC when in state ACTIVE(*) to go to state HALT. */
        m_mfrc522.PICC_HaltA();
        /* Exit the PCD from its authenticated state. */
        m_mfrc522.PCD_StopCrypto1();

        /* Retain timestamp after reading. */
        m_readPrevTstamp_ms = millis();

        log("Card read.");
        success = true;
      }
      else
      {
        log("RFID tag not of type MIFARE Classic.");
        success = false;
      }
    }
    else
    {
      /* No card detected. */
      success = false;
    }
  }
  else
  {
    /* Period between two reads not yet elapsed. */
    success = false;
  }

  return success;
}

/* Logging function. */
void Reader::log(String msg) const
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
