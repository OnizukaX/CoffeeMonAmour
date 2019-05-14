#include <Wire.h>
#include <SSD1306Wire.h>

#include "Display.hpp"

Display::Display(byte f_address,
                 byte f_sda,
                 byte f_sdc) :
  m_display(SSD1306Wire(f_address, f_sda, f_sdc)) {}

void Display::setup()
{
  m_display.init();
  //m_display.flipScreenVertically();
  m_display.setFont(ArialMT_Plain_10);
}

void Display::write(String msg)
{
  m_display.clear();
  m_display.drawString(5, 5, msg);
  m_display.display();
}
