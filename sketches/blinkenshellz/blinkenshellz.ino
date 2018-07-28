/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules
 Pick one up today in the adafruit shop!
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!
 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <bluefruit.h>

// BLE Service
BLEDis  bledis;
BLEUart bleuart;
BLEBas  blebas;

// Software Timer for blinking RED LED
SoftwareTimer blinkTimer;

String mode="btscan"; // set the default mode

// Includes for the pretty lights
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN A0

uint seen=0;
const byte MAX_MACS = 24;
uint8_t seen_macs[6][MAX_MACS]; 
uint8_t seen_names[32][MAX_MACS];

Adafruit_NeoPixel strip = Adafruit_NeoPixel(13, PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  strip.begin();
  strip.setBrightness(16);
  strip.show(); // Initialize all pixels to 'off'

  Serial.begin(115200);

  // Initialize blinkTimer for 1000 ms and start it
  blinkTimer.begin(1000, blink_timer_callback);
  blinkTimer.start();

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behaviour, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName(strcat("POTATO HAX ",getMcuUniqueID()));
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.setConnectCallback(connect_callback);
  Bluefruit.setDisconnectCallback(disconnect_callback);


  // Start Central Scan
  Bluefruit.setConnLedInterval(250);
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.start(0);


  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  startAdv();

  Serial.println("Please use Adafruit's Bluefruit LE app to connect in UART mode");
  Serial.println("Once connected, enter character(s) that you wish to send");
}

// TODO make this work when seen > MAX_MACS
int find_mac(uint8_t* mac)
{
  for(int i=0;i<seen;++i)
  {
    if(0==memcmp(mac,seen_macs[i],6))
    {
      return i;
    }
  }
  //seen_macs[seen]=mac;
  memcpy(seen_macs[seen],mac,6);
  seen++;

  return seen-1;
}

void scan_callback(ble_gap_evt_adv_report_t* report)
{
  Serial.println("got callback");
  uint8_t new_thing[6]; 
  memcpy(new_thing,report->peer_addr.addr,6);
  //Serial.println("copied MAC buffer");
  
  // which device is this?
  //Serial.println("finding thing");
  int thing = find_mac(new_thing);
  //Serial.printf("found thing %d\n",thing);

  memset(seen_names[thing],0,32); 
  if(Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, seen_names[thing], 32))
  {
    Serial.printf("%14s %s\n", "NAME", seen_names[thing]);
    //seen_names[thing]=new_name;
    //memset(buffer, 0, sizeof(buffer));
  }

  Serial.println();
  for ( int i = 0 ; i < seen; ++i)
  {
    Serial.printBufferReverse(seen_macs[i], 6, ':');
    Serial.printf(" : %s",seen_names[i]);
    Serial.println();
  }
  
  Serial.printf("device: %d  ",thing);
  Serial.printf("num seen: %d ",seen);

  // Check if advertising contain BleUart service
  if ( Bluefruit.Scanner.checkReportForUuid(report, BLEUART_UUID_SERVICE) )
  {
    Serial.println("BLE UART service detected");
  }

  Serial.println();
}


void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void btscan()
{
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

int counter = 0; // this is fine

void fire()
{   
    uint32_t first=strip.Color(128,0,0);
    uint32_t second=strip.Color(172,0,0);
    uint32_t third=strip.Color(255,0,0);
    
    for (int i = 0; i < strip.numPixels(); i+=3)
    {
      strip.setPixelColor((i+counter)%15,first);
      strip.setPixelColor((i+counter+1)%15,second);
      strip.setPixelColor((i+counter+2)%15,third);
    }

    counter = (counter+1)%3;
    strip.show();
}

void loop()
{
  Serial.write("loop! ");
  delay(1000);

  if(mode.equals("btscan"))btscan();
  if(mode.equals("fire"))fire();
  
  // Forward from BLEUART to HW Serial
  while ( bleuart.available() )
  {
    char command[80];
    char mode_chars[20];

    memset(command,0,80);
    bleuart.read(command,80);
    
    Serial.write(command);

    if(0==strncmp(command,"help",4))
    {
       bleuart.write("NO HELP FOR YOU!");      
    }
    else if(0==strncmp(command,"mode",4))
    {
       if(strlen(command)==4)bleuart.write("four\n");
       if(strlen(command)==5)bleuart.write("five\n");
       if(strlen(command)==6)bleuart.write("six\n");
       if(strlen(command)==7)bleuart.write("seven\n");

       if(0==strchr(command,' '))
       {
         bleuart.write("MODE : ");
         memset(mode_chars,0,20);
         mode.toCharArray(mode_chars,20);
         bleuart.write(mode_chars);
       }
       else
       {
          mode=String(strchr(command,' ')); //.trim(); 
          mode.trim();

          bleuart.write("MODE CHANGED TO : '");
          memset(mode_chars,0,20);
          mode.toCharArray(mode_chars,20);
          bleuart.write(mode_chars);
          bleuart.write("'\n");
       }
    }
    else
    {
       bleuart.write("UNKNOWN COMMAND,");
       bleuart.write(" TYPE 'HELP' FOR HELP");
    }
    
    bleuart.write("\n#\n");
  }

  // Request CPU to enter low-power mode until an event/interrupt occurs
  waitForEvent();
}

void connect_callback(uint16_t conn_handle)
{
  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);

  //bleuart.write("WELCOME TO BANGLETOS!\n");
  //bleuart.flush();
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
}

/**
 * Software Timer callback is invoked via a built-in FreeRTOS thread with
 * minimal stack size. Therefore it should be as simple as possible. If
 * a periodically heavy task is needed, please use Scheduler.startLoop() to
 * create a dedicated task for it.
 * 
 * More information http://www.freertos.org/RTOS-software-timer.html
 */
void blink_timer_callback(TimerHandle_t xTimerID)
{
  (void) xTimerID;
  digitalToggle(LED_RED);
}

/**
 * RTOS Idle callback is automatically invoked by FreeRTOS
 * when there are no active threads. E.g when loop() calls delay() and
 * there is no bluetooth or hw event. This is the ideal place to handle
 * background data.
 * 
 * NOTE: FreeRTOS is configured as tickless idle mode. After this callback
 * is executed, if there is time, freeRTOS kernel will go into low power mode.
 * Therefore waitForEvent() should not be called in this callback.
 * http://www.freertos.org/low-power-tickless-rtos.html
 * 
 * WARNING: This function MUST NOT call any blocking FreeRTOS API 
 * such as delay(), xSemaphoreTake() etc ... for more information
 * http://www.freertos.org/a00016.html
 */
void rtos_idle_callback(void)
{
  // Don't call any other FreeRTOS blocking API()
  // Perform background task(s) here
}
