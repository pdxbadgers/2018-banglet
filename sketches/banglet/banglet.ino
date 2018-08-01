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

/*********************************************************************
 * Thank you, Adafruit Industries for the examples!
 *
 * This is the sketch that makes the Banglet work
 * It came from https://github.com/pdxbadgers/2018-banglet/
 * There are other sketches there so if you would like to flash the
 * other ones to see what they do or try to hack on your banglet, feel
 * free to clone the project and follow the instructions
 *
 * This code is released under the MIT license and is provided as is
 * with no warranties or guarantees or assurances, so use at your own
 * risk. That means, don't blame us if your banglet catches fire.
 *
 * This is meant to be used with either the Adafruit Bluefruit LE app
 * or the Serial Bluetooth Terminal. It takes advantage of Nordic's
 * proprietary UART service, allowing you to interact with the banglet
 * using a 'shell'. You can find these apps at the Android play store
 * (I am not sure about iOS but I am guessing you can find an equivalent)
 *
 * Once you connect to the banglet, you can list all the commands by sending
 * '/list' through the terminal.
 *
 * Happy Hacking!
 ************************************************************************/

#include <bluefruit.h>
#include <Adafruit_NeoPixel.h>

/*
 * Global
 */

// BLE Service
BLEUart bleuart;

String mode="scan"; // set the default mode

// BT device scan
uint seen=0;
const byte MAX_MACS = 24;
uint8_t* seen_macs[MAX_MACS];
uint8_t* seen_names[MAX_MACS];


// neopixel
#define N_LEDS 12
#define PIN    A0

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// unique names
const String direc[] = {"NE",
                        "NW",
                        "SE",
                        "SW",
                        "N",
                        "S",
                        "E",
                        "W"};

const String loc[] = {"Alder",
                      "Burnside",
                      "Couch",
                      "Davis",
                      "Everett",
                      "Flanders",
                      "Glisan",
                      "Hoyt",
                      "Irving",
                      "Johnson",
                      "Kirby",
                      "Lovejoy",
                      "Marshall",
                      "Northrup",
                      "Overton",
                      "Pettygrove",
                      "Quimby",
                      "Raleigh",
                      "Savier",
                      "Thurman",
                      "Upshur",
                      "Vaughn",
                      "Wilson",
                      "York"};

const String st[] = {"Street",
                     "Avenue",
                     "Way",
                     "Place"};

/*
 * Initial setup
 */
void setup()
{
  // create banglet's name
  randomSeed(uint32_t(getMcuUniqueID()));
  String leading = "503 ";
  String banglet_name = leading + direc[random(0, 8)] + " " + loc[random(0,24)] + " " + st[random(0, 4)];
  char char_b_name[30];
  banglet_name.toCharArray(char_b_name, 30);

  // setup serial
  Serial.begin(115200);
  Serial.println("Welcome to your Banglet!");
  Serial.println("---------------------------\n");

  // setup strip
  stripInit();

  // allocate some buffers
  for(int i=0;i<MAX_MACS;++i)
  {
    uint8_t* new_thing = new uint8_t[6];
    uint8_t* new_name  = new uint8_t[32];
    memset(new_thing,0,6);
    memset(new_name,0,32);
    seen_macs[i]=new_thing;
    seen_names[i]=new_name;
  }

  // Config the peripheral connection with maximum bandwidth
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName(char_b_name);
  Bluefruit.setConnectCallback(connect_callback);
  Bluefruit.setDisconnectCallback(disconnect_callback);

  // Start Central Scan
  Bluefruit.setConnLedInterval(250);
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.start(0);

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Set up and start advertising
  startAdv();

  Serial.println("Please use the Adafruit Bluefruit app or the Serial Bluetooth Terminal app to connect to the Banglet");
  Serial.println("Once connected, send /list for a list of commands you can send");
}

// advertise peripheral BLEUART service
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

// show that the strip is on
void stripInit(){
  strip.begin();
  strip.setBrightness(16);
  for(int i=0;i<strip.numPixels(); i++){
    strip.setPixelColor(i, strip.Color(255, 0, 0));
    strip.show();
    delay(100);
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip.show();
  }
}


// find mac addresses
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
  memcpy(seen_macs[seen%MAX_MACS],mac,6);
  seen++;

  return seen-1;
}

// scan callback function which prints out mac
// addresses in a readable way
void scan_callback(ble_gap_evt_adv_report_t* report)
{
  uint8_t new_thing[6];
  memcpy(new_thing,report->peer_addr.addr,6);
  //Serial.println("copied MAC buffer");

  // which device is this?
  //Serial.println("finding thing");
  int thing = find_mac(new_thing);
  //Serial.printf("found thing %d\n",thing);

  uint8_t new_name[32];
  memset(new_name,0,32);
  if(Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, new_name, 32))
  {
    if(strlen((char*)new_name)>0)
    {
      memset(seen_names[thing],0,32);
      memcpy(seen_names[thing],new_name,32);
    }
  }

}


/*
 * Loop function
 * Read bytes send from UART app and do stuff with it
 */
void loop()
{
  // Forward from BLEUART to HW Serial
  while ( bleuart.available() )
  {
    char rx_buf[64];
    memset(rx_buf,0,sizeof(rx_buf));
    int rx_size = bleuart.read(rx_buf, sizeof(rx_buf));
    String command = String(rx_buf);
    command.trim();
    parseCommand(command);
  }

  if(mode.equals("scan"))btscan();
  else if(mode.equals("rainbow"))rainbow(10, 1);
  else if(mode.equals("patriot"))patriot();
  else if(mode.equals("off"))turnOff();
  else if(mode.equals("frozen"))blueScaleFade(200);
  else if(mode.equals("fire"))redScaleFade(200);
  else btscan(); // I guess scan should be the default

  // Request CPU to enter low-power mode until an event/interrupt occurs
  waitForEvent();
}

// UART service connect callback
void connect_callback(uint16_t conn_handle)
{
  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
  //listCommands();
}

// UART service disconnect callback
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
}

/*
 * Main Banglet functions that parses the send messages and does things with it
 */
void parseCommand(String command)
{
  if(command.equals("help"))listCommands();
  else if(command.equals("devices"))listDevices();
  else if(command.equals("count"))numDevices();
  else if(command.equals("modes"))listModes();

  // some LED modes
  else if(command.equals("scan"))scan();
  else if(command.equals("off"))off();
  else if(command.equals("rainbow"))rainbow();
  else if(command.equals("patriot"))setPatriot();
  else if(command.equals("frozen"))frozen();
  else if(command.equals("fire"))fire();

  // just for fun
  else if(command.equals("id")){bleuart.write("uid=0(root) gid=0(root)");bleuart.write("groups=0(root)\n");}

  // brightness options
  else if(command.equals("bright"))strip.setBrightness(16);
  else if(command.equals("brighter"))strip.setBrightness(32);
  else if(command.equals("brightest"))strip.setBrightness(64);
  else if(command.equals("blinding"))strip.setBrightness(128);
  else
  {
    bleuart.write("Unknown command, ");
    bleuart.write("type 'help' for help.\n");
  }
  bleuart.write("\n# ");
}

/*
 * All the things Banglet can do
 * You can either hack these or add your function here
 */

// list all commands
void listCommands()
{
  bleuart.write("help: You're looking");
  bleuart.write(" at it.\n");

  bleuart.write("devices: List known");
  bleuart.write(" devices.\n");

  bleuart.write("count: List number of");
  bleuart.write(" devices.\n");

  bleuart.write("modes: List banglet");
  bleuart.write(" display modes.\n");
}

// list all modes
void listModes()
{
  bleuart.write("scan: Show bt");
  bleuart.write(" devices.\n");

  if(seen<5)
  {
    bleuart.write("???: unlock at level 5\n");
    bleuart.write("Collect more BT devices to unlock modes!");
    return;
  }

  bleuart.write("off: No lights\n");

  if(seen<10)
  {
    bleuart.write("???: unlock at level 10\n");
    bleuart.write("Collect more BT devices to unlock modes!");
    return;
  }

  bleuart.write("patriot: OMG USA!\n");

  if(seen<15)
  {
    bleuart.write("???: unlock at level 15\n");
    bleuart.write("Collect more BT devices to unlock modes!");
    return;
  }

  bleuart.write("frozen: Only one way");
  bleuart.write(" to find out.\n");

  if(seen<20)
  {
    bleuart.write("???: unlock at level 20\n");
    bleuart.write("Collect more BT devices to unlock modes!");
    return;
  }

  bleuart.write("fire: Halt and");
  bleuart.write(" catch fire.\n");

  if(seen<30)
  {
    bleuart.write("bright/brighter");
    bleuart.write("/brightest\n");
    return;
  }
  if(seen<50)
  {
    bleuart.write("blinding\n");
  }
}


void setPatriot()
{
  bleuart.write("USA! USA!\n");
  mode="patriot";
}

// red, white and blue lights
int patriot_counter=0;
void patriot()
{
  // light up strip with red, white and blue
  uint32_t red=strip.Color(255,0,0);
  uint32_t white=strip.Color(255,255,255);
  uint32_t blue=strip.Color(0,0,255);

  for (int i = 0; i < strip.numPixels(); i+=3)
  {
    strip.setPixelColor((i+patriot_counter)%15,red);
    strip.setPixelColor((i+patriot_counter+1)%15,white);
    strip.setPixelColor((i+patriot_counter+2)%15,blue);
  }

  patriot_counter = (patriot_counter+1)%3;

  strip.show();
  delay(200);
}

// rainbow lights
void rainbow()
{
  bleuart.write("Ooo! Rainbow!\n");
  bleuart.flush();
  mode="rainbow";
}

void rainbow(uint8_t wait, int rainbowLoops)
{
  uint32_t wheelVal;
  int redVal, greenVal, blueVal;

  for(int k = 0 ; k < rainbowLoops ; k ++){

    for(int j=0; j<256; j++) { // 5 cycles of all colors on wheel

      for(int i=0; i< strip.numPixels(); i++) {

        wheelVal = Wheel(((i * 256 / strip.numPixels()) + j) & 255);

        redVal = red(wheelVal);
        greenVal = green(wheelVal);
        blueVal = blue(wheelVal);

        strip.setPixelColor( i, strip.Color( redVal, greenVal, blueVal ) );

      }

        strip.show();
        delay(wait);
    }

  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3,0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3,0);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0,0);
}

uint8_t red(uint32_t c) {
  return (c >> 16);
}
uint8_t green(uint32_t c) {
  return (c >> 8);
}
uint8_t blue(uint32_t c) {
  return (c);
}

// turn the lights off if they get annoying
void off()
{
  bleuart.write("Turning off the lights\n");
  mode="off";
}

void turnOff()
{
  for(int i=0; i<strip.numPixels(); i++)
  {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
  delay(1000); // sleeping is important
}

// scan close by BT devices
void scan()
{
  turnOff();
  bleuart.write("Showing nearby ");
  bleuart.write("bluetooth devices...\n");
  bleuart.flush();
  mode="scan";
}

void btscan()
{
  delay(1000);
  int first=0;
  if(seen>12)first=seen-12;

  for ( uint i=0 ; i < 12; ++i)
  {
    uint32_t color=0;

    if(i>seen)color=strip.Color(0,0,0);
    else color = strip.Color(seen_macs[(first+i)%MAX_MACS][5],seen_macs[(first+i)%MAX_MACS][4],seen_macs[(first+i)%MAX_MACS][3]);
    strip.setPixelColor(i, color);
  }

  strip.show();
}

//frozen
void frozen()
{
  bleuart.write("Let it go!\n");
  bleuart.flush();
  mode="frozen";
}

void blueScaleFade(uint8_t wait)
{
  //loop through fade values

  // divide the neopixel strip into 3 equal parts
  for(int i=0; i< strip.numPixels(); i=i+3)
  {
    // find a color for this set
     uint32_t color = strip.Color(random(0, 181), random(0, 256), random(100, 256));
     // set the color
     for(int j=i; j<i+3; j++)
     {
        if(j<strip.numPixels())
        {
          strip.setPixelColor(j, color);
        }
     }
  }
  strip.show();
  delay(wait);
}

//frozen
void fire()
{
  bleuart.write("Flame on!\n");
  bleuart.flush();
  mode="fire";
}


void redScaleFade(uint8_t wait)
{
  //loop through fade values

  // divide the neopixel strip into 3 equal parts
  for(int i=0; i< strip.numPixels(); i=i+3)
  {
    // find a color for this set
     uint32_t color = strip.Color(random(192, 256), random(0, 64), random(0, 32));
     // set the color
     for(int j=i; j<i+3; j++)
     {
        if(j<strip.numPixels())
        {
          strip.setPixelColor(j, color);
        }
     }
  }
  strip.show();
  delay(wait);
}

//show device names
void listDevices()
{
  for ( int i = 0 ; i < seen; ++i)
  {
    char macbuf[20];
    char namebuf[32];
    memset(macbuf,0,20);
    memset(namebuf,0,32);

    sprintf(macbuf,"%02X:%02X:%02X:%02X:%02X:%02X ",seen_macs[i][5],seen_macs[i][4],seen_macs[i][3],seen_macs[i][2],seen_macs[i][1],seen_macs[i][0]);
    sprintf(namebuf,"%s\n",seen_names[i]);
    Serial.printBufferReverse(seen_macs[i], 6, ':');

    bleuart.write(macbuf);
    bleuart.write(namebuf);
  }
}

//show number of devices
void numDevices()
{
  char outbuf[20];
  memset(outbuf,0,20);
  sprintf(outbuf,"Devices Seen: %d\n",seen);
  bleuart.write(outbuf);
}
