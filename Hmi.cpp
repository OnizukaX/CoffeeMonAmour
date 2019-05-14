#include <Wire.h>
#include <SSD1306Wire.h>

#include "Hmi.hpp"

Hmi::Hmi(byte f_address,
         byte f_sda,
         byte f_sdc) :
  m_display(SSD1306Wire(f_address, f_sda, f_sdc)),
  m_touchButtonPin(0),
  m_touchButtonThresh(0) {}

void Hmi::setup()
{
  m_display.init();
  //m_display.flipScreenVertically();
  m_display.setFont(ArialMT_Plain_10);
}

void Hmi::write(String msg)
{
  m_display.clear();
  m_display.drawString(5, 5, msg);
  m_display.display();
}

void Hmi::configureTouchButton(uint8_t f_pin,
                               uint16_t f_touchButtonThresh)
{
  m_touchButtonPin = f_pin;
  m_touchButtonThresh = f_touchButtonThresh;
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
