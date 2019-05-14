#include <SSD1306Wire.h>

class Display
{
public:
  Display(byte f_address = 0x3C,
          byte f_sda = 4,
          byte f_sdc = 5);

  virtual ~Display() {}

  /* Setup. */
  void setup();
  /* Output text. */
  void write(String msg);
  /* Capacitive touch button. Thresh=30 took empirically. */
  void configureTouchButton(uint8_t f_pin = T7,
                            uint16_t f_touchButtonThresh = 30);
  bool isButtonPressed() const;

private:
  SSD1306Wire m_display;
  uint8_t m_touchButtonPin;
  /* Threshold from which we consider the capacitive touch button to be active. */
  uint16_t m_touchButtonThresh;
};
