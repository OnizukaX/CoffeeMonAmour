#include <SSD1306Wire.h>

class Hmi
{
public:
  struct HmiConfig
  {
    byte address;
    byte sdaPin;
    byte sdcPin;
    uint8_t touchButtonPin;
    uint8_t touchButtonThresh;
    uint8_t wifiStatusPin;
    uint8_t dataStatusPin;
    uint8_t errorStatusPin;
  };

  Hmi(const HmiConfig& cfg);

  virtual ~Hmi() {}

  /* Setup. */
  void setup();
  /* Output text. */
  void write(String text, int16_t x = 5, int16_t y = 5);
  /* Capacitive touch button. */
  bool isButtonPressed() const;
  /* Status. */
  void setWifiStatusLight(bool f_on) const { digitalWrite(m_wifiStatusPin, f_on); }
  void setDataStatusLight(bool f_on) const { digitalWrite(m_dataStatusPin, f_on); }
  void setErrorStatusLight(bool f_on) const { digitalWrite(m_errorStatusPin, f_on); }

private:
  SSD1306Wire m_display;
  /* Status LEDs. */
  uint8_t m_wifiStatusPin;
  uint8_t m_dataStatusPin;
  uint8_t m_errorStatusPin;
  /* Button. */
  uint8_t m_touchButtonPin;
  /* Threshold from which we consider the capacitive touch button to be active. */
  uint16_t m_touchButtonThresh;
};
