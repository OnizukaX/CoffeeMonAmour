#include <Wire.h>
#include <SSD1306Wire.h>

#include "Hmi.hpp"

Hmi::Hmi(const HmiConfig& cfg) :
  m_display(SSD1306Wire(cfg.address, cfg.sdaPin, cfg.sdcPin)),
  m_wifiStatusPin(cfg.wifiStatusPin),
  m_dataStatusPin(cfg.dataStatusPin),
  m_errorStatusPin(cfg.errorStatusPin),
  m_touchButtonPin(cfg.touchButtonPin),
  m_touchButtonThresh(cfg.touchButtonThresh),
  m_balanceEnquiry_ms(0) {}

void Hmi::setup()
{
  m_display.init();
  //m_display.flipScreenVertically();
  m_display.setFont(ArialMT_Plain_10);

  /* LEDs pins setup. */
  pinMode(m_wifiStatusPin, OUTPUT);
  pinMode(m_dataStatusPin, OUTPUT);
  pinMode(m_errorStatusPin, OUTPUT);
}

void Hmi::write(String text, int16_t x, int16_t y)
{
  m_display.clear();
  m_display.drawString(x, y, text);
  m_display.display();
}

bool Hmi::isButtonPressed() const
{
  /* Debounce noise measurements. */
  const uint8_t NB_MEAS = 3;
  uint32_t sumValues = 0;
  for (uint8_t i = 0; i < NB_MEAS; ++i)
  {
    sumValues += touchRead(m_touchButtonPin);
  }
  return (static_cast<uint16_t>(sumValues / NB_MEAS) <= m_touchButtonThresh);
}

bool Hmi::isBalanceEnquiryActive(unsigned long& f_timeLeft_ms) const
{
  static unsigned long enquiryTimeout_ms = 5000;
  f_timeLeft_ms = constrain((0 == m_balanceEnquiry_ms) ? 0 : enquiryTimeout_ms - abs(millis() - m_balanceEnquiry_ms),
                            0,
                            enquiryTimeout_ms);
  return (0 != m_balanceEnquiry_ms) && (abs(millis() - m_balanceEnquiry_ms) < enquiryTimeout_ms);
}
