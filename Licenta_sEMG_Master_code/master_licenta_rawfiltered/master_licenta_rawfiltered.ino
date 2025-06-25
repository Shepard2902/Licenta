#include <WiFi.h>
#include <AsyncUDP.h>
#include <SPI.h>

// Access Point WI-FI credentials
const char* ssid = "ESP32-ADC-AP";
const char* password = "12345678";
AsyncUDP udp;

// Multicast connection
IPAddress multicastIP(239, 1, 2, 3);
const int multicastPort = 1234;


// ESP32-ADC Connection Pinouts
const int CS_PIN = 16;
const int CLK_PIN = 17;
const int MISO_PIN = 5;

// Sampling frequency
const int SAMPLE_RATE = 1000;
uint32_t lastSampleTime = 0;

// Filter variables
float bp_x1 = 0, bp_x2 = 0, bp_y1 = 0, bp_y2 = 0;
float notch_x1 = 0, notch_x2 = 0, notch_y1 = 0, notch_y2 = 0;
float bp_b0, bp_b1, bp_b2, bp_a1, bp_a2;
float notch_b0, notch_b1, notch_b2, notch_a1, notch_a2;

void setup() {
  Serial.begin(115200);
  
  // Configure Access Point
  WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
  if(WiFi.softAP(ssid, password)) {
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("AP Failed!");
    while(1);
  }

  // Multicast listener
  if(udp.listenMulticast(multicastIP, multicastPort)) {
    Serial.println("Multicast listener started");
    udp.onPacket([](AsyncUDPPacket packet) {
      if(packet.length() > 4) {
        String data = String((char*)packet.data(), packet.length());
        if(data.startsWith("EMG:")) {
          Serial.print("Received EMG: ");
          Serial.println(data.substring(4));
        }
      }
    });
  } else {
    Serial.println("Multicast listen failed");
    while(1);
  }
// SPI connection with ADC for data transfer
  SPI.begin(CLK_PIN, MISO_PIN, -1, CS_PIN);
  SPI.setFrequency(1000000);
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  initFilters();
}

// ESP32 reads ADC data through SPI
uint16_t readADC() {
  digitalWrite(CS_PIN, LOW);
  delayMicroseconds(1);
  uint16_t result = SPI.transfer16(0x0000);
  digitalWrite(CS_PIN, HIGH);
  return (result >> 1) & 0x0FFF;
}
// Notch filter
float processEMG(uint16_t raw) {
  float centered = (raw - 2048.0) / 2048.0;
  float bp_out = bp_b0 * centered + bp_b1 * bp_x1 + bp_b2 * bp_x2 - bp_a1 * bp_y1 - bp_a2 * bp_y2;
  bp_x2 = bp_x1; bp_x1 = centered;
  bp_y2 = bp_y1; bp_y1 = bp_out;

  float notch_out = notch_b0 * bp_out + notch_b1 * notch_x1 + notch_b2 * notch_x2 - notch_a1 * notch_y1 - notch_a2 * notch_y2;
  notch_x2 = notch_x1; notch_x1 = bp_out;
  notch_y2 = notch_y1; notch_y1 = notch_out;

  return abs(notch_out) * 1000.0;
}
// Bandpass Filter
void initFilters() {
  float f_low = 20.0, f_high = 500.0;
  float w0 = 2 * PI * sqrt(f_low * f_high) / SAMPLE_RATE;
  float BW = f_high - f_low;
  float Q = w0 * SAMPLE_RATE / (2 * PI * BW);
  float alpha = sin(w0) / (2 * Q);

  bp_b0 = alpha;
  bp_b1 = 0;
  bp_b2 = -alpha;
  float bp_a0 = 1 + alpha;
  bp_a1 = -2 * cos(w0);
  bp_a2 = 1 - alpha;

  bp_b0 /= bp_a0; bp_b1 /= bp_a0; bp_b2 /= bp_a0;
  bp_a1 /= bp_a0; bp_a2 /= bp_a0;

  float notch_f0 = 50.0, notch_Q = 30.0;
  float notch_w0 = 2 * PI * notch_f0 / SAMPLE_RATE;
  float notch_alpha = sin(notch_w0) / (2 * notch_Q);
  float notch_a0 = 1 + notch_alpha;

  notch_b0 = 1;
  notch_b1 = -2 * cos(notch_w0);
  notch_b2 = 1;
  notch_a1 = -2 * cos(notch_w0) / notch_a0;
  notch_a2 = (1 - notch_alpha) / notch_a0;

  notch_b0 /= notch_a0; notch_b1 /= notch_a0; notch_b2 /= notch_a0;
}

// Data Readings
void loop() {
  uint32_t now = micros();
  if(now - lastSampleTime >= 1000000UL / SAMPLE_RATE) {
    lastSampleTime = now;
    
    uint16_t raw = readADC();
    float filtered = processEMG(raw);
    
    // Filtered sEMG signal 
    AsyncUDPMessage filteredMsg;
    filteredMsg.print("EMG:" + String(filtered, 2));
    udp.sendTo(filteredMsg, multicastIP, multicastPort);
    
    // Raw sEMG signal
    AsyncUDPMessage rawMsg;
    rawMsg.print("RAW:" + String(raw));
    udp.sendTo(rawMsg, multicastIP, multicastPort);
  }

  //Delay created so we can see data slower to be able to read/analyze it
  delay(10); 
}