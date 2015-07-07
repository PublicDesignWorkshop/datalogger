
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <RTC_DS3234.h>

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

const int chipSelect = 4;

// Create an RTC instance, using the chip select pin it's connected to
RTC_DS3234 RTC(8);

// Data logging configuration.
#define LOGGING_FREQ_SECONDS   21600       // Seconds to wait before a new sensor reading is logged.

#define MAX_SLEEP_ITERATIONS   LOGGING_FREQ_SECONDS / 8  // Number of times to sleep (for 8 seconds) before
                                                         // a sensor reading is taken and sent to the server.
                                                         // Don't change this unless you also change the 
                                                         // watchdog timer configuration.

int sleepIterations = 0;
uint32_t ip;
volatile bool watchdogActivated = false;

ISR(WDT_vect){
  // Set the watchdog activated flag.
  // Note that you shouldn't do much work inside an interrupt handler.
  watchdogActivated = true;
}

// Put the Arduino to sleep.
void sleep(){
  // Set sleep to full power down.  Only external interrupts or 
  // the watchdog timer can wake the CPU!
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // Turn off the ADC while asleep.
  power_adc_disable();

  // Enable sleep and enter sleep mode.
  sleep_mode();

  // CPU is now asleep and program execution completely halts!
  // Once awake, execution will resume at this point.
  
  // When awake, disable sleep mode and turn on all devices.
  sleep_disable();
  power_all_enable();
}

void logSensorReading() {
    // make a string for assembling the data to log:
  String dataString = "";
  
  // get current date/time and append to string
  const int len = 32;
  static char buf[len];

  DateTime now = RTC.now();
  dataString += now.toString(buf,len);

  // read three sensors and append to the string:
  int analogPin = 0;
  int sensor = analogRead(analogPin);
  dataString += ", "+String(sensor);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
}


void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  SPI.begin();
  RTC.begin();

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  
  Serial.println("SD card initialized.");
 
  // Setup the watchdog timer to run an interrupt which
  // wakes the Arduino from sleep every 8 seconds.
  
  // Note that the default behavior of resetting the Arduino
  // with the watchdog will be disabled.
  
  // This next section of code is timing critical, so interrupts are disabled.
  // See more details of how to change the watchdog in the ATmega328P datasheet
  // around page 50, Watchdog Timer.
  noInterrupts();
  
  // Set the watchdog reset bit in the MCU status register to 0.
  MCUSR &= ~(1<<WDRF);
  
  // Set WDCE and WDE bits in the watchdog control register.
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  // Set watchdog clock prescaler bits to a value of 8 seconds.
  WDTCSR = (1<<WDP0) | (1<<WDP3);
  
  // Enable watchdog as interrupt only (no reset).
  WDTCSR |= (1<<WDIE);
  
  // Enable interrupts again.
  interrupts();
  
  Serial.println(F("Setup complete."));
}

void loop()
{
  // Don't do anything unless the watchdog timer interrupt has fired.
  if (watchdogActivated)
  {
    watchdogActivated = false;
    // Increase the count of sleep iterations and take a sensor
    // reading once the max number of iterations has been hit.
    sleepIterations += 1;
    if (sleepIterations >= MAX_SLEEP_ITERATIONS) {
      // Reset the number of sleep iterations.
      sleepIterations = 0;
      // Log the sensor data
      logSensorReading();
    }
  } 
  // Go to sleep!
  sleep();
}









