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
  /* Display. */
  inline void clear() { m_display.clear(); }
  inline void display() { m_display.display(); }
  /* Output text. */
  void write(const String& f_text, const bool f_clearNdisplay, const int16_t f_x, const int16_t f_y, const uint8_t& f_fontData);
  inline void writeSmall(const String& f_text, const bool f_clearNdisplay = true, const int16_t f_x = 5, const int16_t f_y = 5)
  { write(f_text, f_clearNdisplay, f_x, f_y, *ArialMT_Plain_10); }
  inline void writeMedium(const String& f_text, const bool f_clearNdisplay = true, const int16_t f_x = 5, const int16_t f_y = 8)
  { write(f_text, f_clearNdisplay, f_x, f_y, *ArialMT_Plain_16); }
  inline void writeBig(const String& f_text, const bool f_clearNdisplay = true, const int16_t f_x = 5, const int16_t f_y = 12)
  { write(f_text, f_clearNdisplay, f_x, f_y, *ArialMT_Plain_24); }
  /* Capacitive touch button. */
  bool isButtonPressed() const;
  /* Balance enquiry. */
  inline unsigned long getEnquiryTimeout() const { return m_enquiryTimeout_ms; }
  inline void setBalanceEnquiry(const unsigned long f_time_ms) { m_balanceEnquiry_ms = f_time_ms; }
  bool isBalanceEnquiryActive(unsigned long& f_timeLeft_ms) const;
  void drawProgressBar(const uint8_t f_progress, const uint8_t f_margin_x = 5, const uint8_t f_margin_y = 25);
  /* Ticking. */
  void fillCoffeeCup();

  /* Status. */
  inline void setWifiStatusLight(const bool f_on) const { digitalWrite(m_wifiStatusPin, f_on); }
  inline void setDataStatusLight(const bool f_on) const { digitalWrite(m_dataStatusPin, f_on); }
  inline void setErrorStatusLight(const bool f_on) const { digitalWrite(m_errorStatusPin, f_on); }
  inline void setWDEStatusLights(const bool f_wifi, const bool f_data, const bool f_error) const
  { setWifiStatusLight(f_wifi); setDataStatusLight(f_data); setErrorStatusLight(f_error); }

private:
  SSD1306Wire m_display;
  /* Status LEDs. */
  const uint8_t m_wifiStatusPin;
  const uint8_t m_dataStatusPin;
  const uint8_t m_errorStatusPin;
  /* Button. */
  const uint8_t m_touchButtonPin;
  /* Threshold from which we consider the capacitive touch button to be active. */
  const uint16_t m_touchButtonThresh;
  /* Last time the balance enquiry was requested. */
  unsigned long m_balanceEnquiry_ms;
  unsigned long m_enquiryTimeout_ms;
};
