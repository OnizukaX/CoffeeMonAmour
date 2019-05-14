#include <SSD1306Wire.h>

class Hmi
{
public:
  Hmi(byte f_address,
      byte f_sda,
      byte f_sdc,
      uint8_t f_wifiStatusPin,
      uint8_t f_dataStatusPin,
      uint8_t f_errorStatusPin);

  virtual ~Hmi() {}

  /* Setup. */
  void setup();
  /* Output text. */
  void write(String msg);
  /* Capacitive touch button. Thresh=30 took empirically. */
  void configureTouchButton(uint8_t f_pin = T7,
                            uint16_t f_touchButtonThresh = 30);
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
