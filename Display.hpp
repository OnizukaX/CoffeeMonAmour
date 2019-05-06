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

private:
  SSD1306Wire m_display;
};
