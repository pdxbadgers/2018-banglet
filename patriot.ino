#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include <BLEPeripheral.h>

#define PIN A0

Adafruit_NeoPixel strip = Adafruit_NeoPixel(13, PIN, NEO_GRB + NEO_KHZ800);

// bluetooth
BLEPeripheral blePeriph;// = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);
BLEService bleServ = BLEService("0503");
BLECharacteristic bannerText("0503", BLERead | BLEWrite,20);

String banner="usa";

uint32_t color = 0;
int counter=0;

void setup() {
  strip.begin();
  strip.setBrightness(16);
  strip.show(); // Initialize all pixels to 'off'

  setupBLE();
}


void setupBLE()
{
  // Advertise name and service:
  blePeriph.setDeviceName("503BANGLET");
  blePeriph.setLocalName("503BANGLET");
  blePeriph.setAdvertisedServiceUuid(bleServ.uuid());

  // Add service
  blePeriph.addAttribute(bleServ);

  // Add characteristic
  blePeriph.addAttribute(bannerText);

  // Now that device6, service, characteristic are set up,
  // initialize BLE:
  blePeriph.begin();

  // Set led characteristic to default value:
  char bannerBuff[20];
  banner.toCharArray(bannerBuff,20);
  bannerText.setValue(bannerBuff);  
}

void loop() {

  uint16_t i, j;


  if(banner=="red")
  {
    uint32_t first=strip.Color(128,0,0);
    uint32_t second=strip.Color(172,0,0);
    uint32_t third=strip.Color(255,0,0);
    
    for (i = 0; i < strip.numPixels(); i+=3)
    {
      strip.setPixelColor((i+counter)%15,first);
      strip.setPixelColor((i+counter+1)%15,second);
      strip.setPixelColor((i+counter+2)%15,third);
    }
  }
  else if(banner=="green")
  {
    uint32_t first=strip.Color(0,128,0);
    uint32_t second=strip.Color(0,172,0);
    uint32_t third=strip.Color(0,255,0);
    
    for (i = 0; i < strip.numPixels(); i+=3)
    {
      strip.setPixelColor((i+counter)%15,first);
      strip.setPixelColor((i+counter+1)%15,second);
      strip.setPixelColor((i+counter+2)%15,third);
    }
  }
  else if(banner=="blue")
  {
    uint32_t first=strip.Color(0,0,128);
    uint32_t second=strip.Color(0,0,172);
    uint32_t third=strip.Color(0,0,255);
    
    for (i = 0; i < strip.numPixels(); i+=3)
    {
      strip.setPixelColor((i+counter)%15,first);
      strip.setPixelColor((i+counter+1)%15,second);
      strip.setPixelColor((i+counter+2)%15,third);
    }
  }
  else if(banner=="usa")
  {
    uint32_t red=strip.Color(255,0,0);
    uint32_t white=strip.Color(255,255,255);
    uint32_t blue=strip.Color(0,0,255);
    
    for (i = 0; i < strip.numPixels(); i+=3)
    {
      strip.setPixelColor((i+counter)%15,red);
      strip.setPixelColor((i+counter+1)%15,white);
      strip.setPixelColor((i+counter+2)%15,blue);
    }
  }

  else
  {
    for (i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, 0x66,0x0+i*0x20,i*0x20-0x10);
    }
  }

  counter = (counter+1)%3;
  
  strip.show();
  delay(200);
 
  radioloop();
}


void radioloop()
{
  blePeriph.poll();

  if (bannerText.written())
  {
    char textValue[20];
    memset(textValue,0,20);
    strncpy(textValue,(char*)bannerText.value(),20);
    textValue[bannerText.valueLength()]=0;
    banner=String(textValue);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85)
  {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
