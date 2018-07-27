#include <bluefruit.h>
#include <Adafruit_NeoPixel.h>


#define N_LEDS 10
#define PIN     A0
#define UNCONNECTED 0

// for listing mac addresses
uint seen=0;
const byte MAX_MACS = 100;
uint8_t* seen_macs[MAX_MACS]; 

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// Central uart client
// central can passively see other devices
BLEClientUart clientUart;

// Peripheral uart service
// Peripheral can receive data packets sent to it
BLEUart bleuart;

// Function prototypes for packetparser.cpp
// we need this for the Adafruit app but maybe not...
uint8_t readPacket (BLEUart *ble_uart, uint16_t timeout);
float   parsefloat (uint8_t *buffer);
void    printHex   (const uint8_t * data, const uint32_t numBytes);

// Packet buffer
extern uint8_t packetbuffer[];

void setup(void)
{
  Serial.begin(115200);
  Serial.println(F("Banglet is on"));
  Serial.println(F("-------------------------------------------"));

  // Initialize Bluefruit with max concurrent connections as Peripheral = 1, Central = 1
  // SRAM usage required by SoftDevice will increase with number of connections
  Bluefruit.begin(1,1);
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("Banglet");

    // Callbacks for Peripheral
  Bluefruit.setConnectCallback(prph_connect_callback);
  Bluefruit.setDisconnectCallback(prph_disconnect_callback);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(cent_connect_callback);
  Bluefruit.Central.setDisconnectCallback(cent_disconnect_callback);

  // Configure and Start BLE Uart Service
  bleuart.begin();
  bleuart.setRxCallback(prph_bleuart_rx_callback);

  // Init BLE Central Uart Serivce
  clientUart.begin();
  clientUart.setRxCallback(cent_bleuart_rx_callback);

    /* Start Central Scanning
   * - Enable auto scan if disconnected
   * - Interval = 100 ms, window = 80 ms
   * - Filter only accept bleuart service
   * - Don't use active scan
   * - Start(timeout) with timeout = 0 will scan forever (until connected)
   */

/*
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); // in unit of 0.625 ms
  Bluefruit.Scanner.filterUuid(bleuart.uuid);
  Bluefruit.Scanner.useActiveScan(false);
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds
  */

  // Start Central Scan
  // Dual role connects to another peripheral but we just want to passively listen
  Bluefruit.setConnLedInterval(250);
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.start(0);

  Serial.println("Scanning ...");

  // Set up and start advertising
  startAdv();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in Controller mode"));
  Serial.println(F("Then activate/use the sensors, color picker, game controller, etc!"));
  Serial.println(); 

  stripInit();
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  
  // Include the BLE UART (AKA 'NUS') 128-bit UUID
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

// show that the strip is on
void stripInit(){
  strip.begin();
  for(int i=0;i<strip.numPixels(); i++){
    strip.setPixelColor(i, strip.Color(255, 0, 0));
    strip.show();
    delay(100);
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
  }
}

// turn off all pixels
void turnOff(){
  for(int i=0; i<strip.numPixels(); i++){
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
  }
}

// turn all pixels on
void testAll(uint8_t r, uint8_t g, uint8_t b){
  for(int i=0; i<strip.numPixels(); i++){
    strip.setPixelColor(i, strip.Color(r, g, b));
    strip.show();
  }
}

// turn all pixels on in a color range
void testColorRange(uint8_t r, uint8_t g, uint8_t b){
  
}

// find mac address
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

void loop(void)
{
  // light up the LEDs
  int first=0;
  if(seen>12)first=seen-12;
  
  for (int i = first ; i < seen; ++i)
  {
    //uint32_t color = strip.Color(seen_addrs[i][0],seen_addrs[i][1],seen_addrs[i][2]);
    uint32_t color = strip.Color(seen_macs[i][5],seen_macs[i][4],seen_macs[i][3]);
    strip.setPixelColor(i-first, color);
  }

  strip.show();
}


/*------------------------------------------------------------------*/
/* Peripheral
 *------------------------------------------------------------------*/
void prph_connect_callback(uint16_t conn_handle)
{
  char peer_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, peer_name, sizeof(peer_name));

  Serial.print("[Prph] Connected to ");
  Serial.println(peer_name);

  turnOff();

}

void prph_disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("[Prph] Disconnected");
  turnOff();
}

void prph_bleuart_rx_callback(void)
{
  // This is meant to work with a mobile central
  // in this case, it is Adafruit's BLE app

  // Wait for new data to arrive
  uint8_t len = readPacket(&bleuart, 500);
  if (len == 0) return;

  // Color
  if (packetbuffer[1] == 'C') {
    uint8_t red = packetbuffer[2];
    uint8_t green = packetbuffer[3];
    uint8_t blue = packetbuffer[4];
    Serial.print ("RGB #");
    if (red < 0x10) Serial.print("0");
    Serial.print(red, HEX);
    if (green < 0x10) Serial.print("0");
    Serial.print(green, HEX);
    if (blue < 0x10) Serial.print("0");
    Serial.println(blue, HEX);
    testAll(red, green, blue);
  }

  // This part is to check if the clientUart was discovered

  // Forward data from Mobile to our peripheral
  char str[20+1] = { 0 };
  bleuart.read(str, 20);

  if ( clientUart.discovered() )
  {
    clientUart.print(str);
  }else
  {
    bleuart.println("[Prph] Central role not connected");
  }
}

/*------------------------------------------------------------------*/
/* Central
 *------------------------------------------------------------------*/
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
    Serial.println("BLE UART service detected");
  }
}

void cent_connect_callback(uint16_t conn_handle)
{
  // not sure what this does
  char peer_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, peer_name, sizeof(peer_name));

  Serial.print("[Cent] Connected to ");
  Serial.println(peer_name);;

  if ( clientUart.discover(conn_handle) )
  {
    // Enable TXD's notify
    clientUart.enableTXD();
  }else
  {
    // disconect since we couldn't find bleuart service
    Bluefruit.Central.disconnect(conn_handle);
  }  
}

void cent_disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;
  
  Serial.println("[Cent] Disconnected");
}

/**
 * Callback invoked when uart received data
 * @param cent_uart Reference object to the service where the data 
 * arrived. In this example it is clientUart
 */
void cent_bleuart_rx_callback(BLEClientUart& cent_uart)
{
  char str[20+1] = { 0 };
  cent_uart.read(str, 20);
      
  Serial.print("[Cent] RX: ");
  Serial.println(str);

  if ( bleuart.notifyEnabled() )
  {
    // Forward data from our peripheral to Mobile
    bleuart.print( str );
  }else
  {
    // response with no prph message
    clientUart.println("[Cent] Peripheral role not connected");
  }  
}

