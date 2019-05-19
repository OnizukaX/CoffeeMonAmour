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
  m_balanceEnquiry_ms(0),
  m_enquiryTimeout_ms(5000) {}

void Hmi::setup()
{
  m_display.init();
  m_display.clear();
  m_display.display();
  //m_display.flipScreenVertically();

  /* LEDs pins setup. */
  pinMode(m_wifiStatusPin, OUTPUT);
  pinMode(m_dataStatusPin, OUTPUT);
  pinMode(m_errorStatusPin, OUTPUT);
}

void Hmi::write(const String f_text, const bool f_clearNdisplay, const int16_t f_x, const int16_t f_y, const uint8_t& f_fontData)
{
  if (f_clearNdisplay) m_display.clear();
  m_display.setFont(&f_fontData);
  m_display.drawString(f_x, f_y, f_text);
  if (f_clearNdisplay) m_display.display();
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
  f_timeLeft_ms = constrain((0 == m_balanceEnquiry_ms) ? 0 : static_cast<long long>(m_enquiryTimeout_ms - abs(millis() - m_balanceEnquiry_ms)),
                            0,
                            m_enquiryTimeout_ms);
  return (0 != m_balanceEnquiry_ms) && (abs(millis() - m_balanceEnquiry_ms) < m_enquiryTimeout_ms);
}

void Hmi::drawProgressBar(uint8_t f_progress, uint8_t f_margin_x, uint8_t f_margin_y)
{
  m_display.drawProgressBar(f_margin_x,
                            f_margin_y,
                            m_display.getWidth() - (2 * f_margin_x),
                            m_display.getHeight() - (2 * f_margin_y),
                            f_progress);
}
