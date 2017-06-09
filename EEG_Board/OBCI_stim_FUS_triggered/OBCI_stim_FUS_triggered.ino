// Main Loop for OBCI system to be triggered from the
// Adapted by Jeffrey Wang (@dancingdarwin) from pushbutton Contributed example by Irene Vigue Guix (@viguix)

#include <DSPI.h>
#include <OBCI32_SD.h>
#include <EEPROM.h>
#include <OpenBCI_32bit_Library.h>
#include <OpenBCI_32bit_Library_Definitions.h>

// Booleans Required for SD_Card_Stuff.ino
boolean addAccelToSD = false; // On writeDataToSDcard() call adds Accel data to SD card write
boolean addAuxToSD = true; // On writeDataToSDCard() call adds Aux data to SD card write
boolean SDfileOpen = false; // Set true by SD_Card_Stuff.ino on successful file open

const uint8_t Rate_Adj = ADS_RATE_1kHz;

// Three External Trigger Code
// --------------------------------------------------------
//  << EXTERNAL TRIGGER FROM FUS and STIMULI >>
//const int fusTrig = 13;          // FUS Trigger (from IGT System)
const int stimTrig = 17;          //  LED Stimulus Trigger (from RPi)
const int fusTrig = 18;         // Stimulus Isolator Trigger (from RPi)
int fusValue;          // used to hold the latest button reading
int stimValue;

// --------------------------------------------------------

void setup() {
  // Bring up the OpenBCI Board with the sample rate
  /* Possible Rates:
   * 250 Hz: ADS_RATE_250Hz
   * 500 Hz: ADS_RATE_500Hz
   * 1 kHz:  ADS_RATE_1kHz
   * 2 kHz:  ADS_RATE_2kHz
   * 4 kHz:  ADS_RATE_4kHz
   * 8 kHz:  ADS_RATE_8kHz
   * 16 kHz: ADS_RATE_16kHz
   */
  board.beginDebug(ADS_RATE_1kHz);

  // Notify the board we don't want to use accel data
  board.useAccel = false;

  // Notify the board we want to use aux data, this affects `::sendChannelData()`
  board.useAux = true;

  // Configure two external triggers
  pinMode(stimTrig, INPUT);    // set the button pin direction
  pinMode(fusTrig, INPUT);
  //digitalWrite(fusTrig, LOW);
  //pinMode(ssepTrig, INPUT);
}

void loop() {
  if (board.streaming) {
    // Read from Stim
    stimValue = digitalRead(stimTrig);
    if (stimValue == HIGH) {  // if it's active
      board.auxData[0] = board.auxData[0] | 0x0001;
      addAuxToSD = true;             // add Aux Data to the SD card if it's there
    }

    // Read from FUS
    fusValue = digitalRead(fusTrig);
    if (fusValue == HIGH) {  // if it's active
      board.auxData[0] = board.auxData[0] | 0x0002;
      addAuxToSD = true;             // add Aux Data to the SD card if it's there
    }
    
    if (board.channelDataAvailable) {
      // Read from the ADS(s), store data, set channelDataAvailable flag to false
      board.updateChannelData();

      // Verify the SD file is open
      if (SDfileOpen) {
        // Write to the SD card, writes aux data
        writeDataToSDcard(board.sampleCounter);
      }

      // Send standard packet with channel data, trigger data, and time stamp
      board.sendChannelDataWithTimeAndRawAux();

      //Reset triggers
      board.auxData[0] = 0x0000;
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

  if (board.hasDataSerial1()) {
    // Read one char from the serial port
    char newChar = board.getCharSerial1();

    // Send to the sd library for processing
    sdProcessChar(newChar);

    // Send to the board library
    board.processChar(newChar);
  }
}
