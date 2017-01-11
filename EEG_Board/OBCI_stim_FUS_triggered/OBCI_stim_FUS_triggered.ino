// Main Loop for OBCI system to be triggered from the 
// Adapted by Jeffrey Wang (gojeff@stanford.edu) from pushbutton Contributed example by Irene Vigue Guix (@viguix)

#include <DSPI.h>
#include <OBCI32_SD.h>
#include <EEPROM.h>
#include <OpenBCI_32bit_Library.h>
#include <OpenBCI_32bit_Library_Definitions.h>

// Booleans Required for SD_Card_Stuff.ino
boolean addAccelToSD = false; // On writeDataToSDcard() call adds Accel data to SD card write
boolean addAuxToSD = true; // On writeDataToSDCard() call adds Aux data to SD card write
boolean SDfileOpen = false; // Set true by SD_Card_Stuff.ino on successful file open

//ADS Sample Rate
#define ADS_RATE_250Hz  0b110
#define ADS_RATE_500Hz  0b101
#define ADS_RATE_1KHz   0b100
#define ADS_RATE_2KHz   0b011
#define ADS_RATE_4KHz   0b010
#define ADS_RATE_8KHz   0b001
#define ADS_RATE_16KHz  0b000

const uint8_t Rate_Adj = ADS_RATE_250Hz;

// Three External Trigger Code
// --------------------------------------------------------
//  << EXTERNAL TRIGGER FROM FUS and STIMULI >>
const int fusTrig = 13;          // FUS Trigger (from IGT System)
const int vepTrig = 17;          // LED Stimulus Trigger (from RPi)
const int ssepTrig = 18;         // Stimulus Isolator Trigger (from RPi)
int fusValue;          // used to hold the latest button reading
int vepValue;
int ssepValue;

// --------------------------------------------------------

void setup() {
  // Bring up the OpenBCI Board with the sample rate
  //board.begin(Rate_Adj);
  board.beginDebug();

  // Notify the board we don't want to use accel data
  board.useAccel = false;

  // Notify the board we want to use aux data, this affects `::sendChannelData()`
  board.useAux = true;

  // Configure two external triggers
  pinMode(fusTrig, INPUT);    // set the button pin direction
  pinMode(vepTrig, INPUT);
  pinMode(ssepTrig, INPUT);
}

void loop() {
  if (board.streaming) {
    if (board.channelDataAvailable) {
      // Read from the ADS(s), store data, set channelDataAvailable flag to false
      board.updateChannelData();

      //Reset triggers
      board.auxData[0] = 0x0000;

      // Read from FUS
      fusValue = digitalRead(fusTrig);
      if (fusValue == HIGH) {  // if it's active
        board.auxData[0] += 0x0001;
        addAuxToSD = true;             // add Aux Data to the SD card if it's there
      }

      // Read from VEP
      vepValue = digitalRead(vepTrig);
      if (vepValue == HIGH) {  // if it's active
        board.auxData[0] += 0x0002;
        addAuxToSD = true;             // add Aux Data to the SD card if it's there
      }

      // Read from SSEP
      ssepValue = digitalRead(ssepTrig);
      if (ssepValue == HIGH) {  // if it's active
        board.auxData[0] += 0x0004;
        addAuxToSD = true;             // add Aux Data to the SD card if it's there
      }      

      // Verify the SD file is open
      if(SDfileOpen) {
        // Write to the SD card, writes aux data
        writeDataToSDcard(board.sampleCounter);
      }

      // Send standard packet with channel data, trigger data, and time stamp
      board.sendChannelDataWithTimeAndRawAux();
    }
  }

  // Check the serial port for new data
  if (board.hasDataSerial0()) {
    // Read one char from the serial port
    char newChar = board.getCharSerial0();

    // Send to the sd library for processing
    sdProcessChar(newChar);

    // Send to the board library
    board.processChar(newChar);
  }
}
