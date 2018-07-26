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
BLEUart bleuart;

// List of commands
const int MAX_COMM_LEN = 10;
const int MAX_COMM = 3;
const char commands[MAX_COMM][MAX_COMM_LEN] = {"list", "rainbow", "patriot"};

void setup()
{
  Serial.begin(115200);
  Serial.println("Welcome to the Banglet!");
  Serial.println("---------------------------\n");

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);

  Bluefruit.begin();
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  Bluefruit.setName("Banglet");
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.setConnectCallback(connect_callback);
  Bluefruit.setDisconnectCallback(disconnect_callback);

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Set up and start advertising
  startAdv();

  Serial.println("Please use the Serial Bluetooth Terminal app to connect to the Banglet");
  Serial.println("Once connected, send LIST for a list of commands to send");
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

void loop()
{
  // Forward data from HW Serial to BLEUART
  while (Serial.available())
  {
    // Delay to wait for enough input, since we have a limited transmission buffer
    delay(2);

    uint8_t buf[64];
    int count = Serial.readBytes(buf, sizeof(buf));
    bleuart.write( buf, count );
  }

  // Forward from BLEUART to HW Serial
  while ( bleuart.available() )
  {
    uint8_t rx_buf[64];
    int rx_size = bleuart.read(rx_buf, sizeof(rx_buf));
    parseCommand(rx_buf, rx_size);
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
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
}

/**
 * Code that banglet uses
 */

void parseCommand(uint8_t *buf, int bufsize)
{
  //for(int i=0; i<bufsize; i++){
  //  Serial.write(buf[i]); 
  //}
  if(buf[0] == '/'){
    int index = getCommand(buf);
    if(index != -1)
    {
      doOption(index);
    }else{
      bleuart.write("Invalid option\n");
    }
  }else{
    bleuart.write("This is not a command\n");
  }
}

int getCommand(uint8_t *buf)
{
  int buf_end = 1;
  while(buf[buf_end] != '\n')
  {
   // Serial.println((char)buf[buf_end]);
    buf_end++;
  }
  // for some reason there are some undetermined characters at the end
  // so shortend the length by this amount which is was found out through
  // experimentation - yes I don't know what is going on here
  buf_end = buf_end - 2;
  //Serial.println(buf_end);
  
  //go through each command
  for(int i=0; i<MAX_COMM; i++)
  {
    //Serial.println(commands[i]);
    //Serial.println((char *)buf);
    //check if the buffer command is the same
    if(strncmp(commands[i], (char *)++buf, buf_end) == 0)
    {
      return i;
    }else{
      --buf;
    }
  }

  return -1;
}

void doOption(int index)
{
  // list
  Serial.write(commands[index]);
}

/**
 * End of code that banglet uses
 */
 
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

