/*********************************************************************
Use the banglet to monitor nearby bluetooth devices
*********************************************************************/

#include <bluefruit.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN A0

uint seen=0;

const byte MAX_MACS = 100;
uint8_t* seen_macs[MAX_MACS]; 

Adafruit_NeoPixel strip = Adafruit_NeoPixel(13, PIN, NEO_GRB + NEO_KHZ800);


void setup() 
{  
  strip.begin();
  strip.setBrightness(16);
  strip.show(); // Initialize all pixels to 'off'

  
  Serial.begin(115200);

  Serial.println("Bluefruit52 Central Scan Example");
  Serial.println("--------------------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  // SRAM usage required by SoftDevice will increase dramatically with number of connections
  Bluefruit.begin(1, 1);
  
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("503HAX");

  // Start Central Scan
  Bluefruit.setConnLedInterval(250);
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.start(0);

  Serial.println("Scanning ...");
}

int find_mac(uint8_t* mac)
{
  for(int i=0;i<seen;++i)
  {
    if(0==memcmp(mac,seen_macs[i],6))
    {
      return i;
    }
  }
  seen_macs[seen]=mac;
  seen++;
}

void scan_callback(ble_gap_evt_adv_report_t* report)
{
  uint8_t* new_thing = new uint8_t[6]; 
  memcpy(new_thing,report->peer_addr.addr,6);

  int thing = find_mac(new_thing);
  
  Serial.println();
  for ( int i = 0 ; i < seen; ++i)
  {
    Serial.printBufferReverse(seen_macs[i], 6, ':');
    Serial.println();
  }
  
  Serial.printf("%09d ",thing);
  Serial.printf("%09d ",seen);

  // Check if advertising contain BleUart service
  if ( Bluefruit.Scanner.checkReportForUuid(report, BLEUART_UUID_SERVICE) )
  {
    Serial.println("                       BLE UART service detected");
  }

  Serial.println();
}

void loop() 
{
  // Toggle both LEDs every 1 second
  digitalToggle(LED_RED);

  delay(1000);

  uint32_t color_red = strip.Color(128,0,0);
  uint i = 0;

  // light up the LEDs
  int first=0;
  if(seen>12)first=seen-12;
  
  for ( i = first ; i < seen; ++i)
  {
    //uint32_t color = strip.Color(seen_addrs[i][0],seen_addrs[i][1],seen_addrs[i][2]);
    uint32_t color = strip.Color(seen_macs[i][5],seen_macs[i][4],seen_macs[i][3]);
    strip.setPixelColor(i-first, color);
  }

  strip.show();
}
