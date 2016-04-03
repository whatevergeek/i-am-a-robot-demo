#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// If using the breakout with SPI, define the pins for SPI communication.
#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

#include <IRremote.h>
#define button_a 4

IRsend irsend;

long cmd_go  = 0x00FD08F7;
long cmd_stop = 0x00FD8877;
long cmd_random_stop = 0x00FD48B7;

void setup()
{
  Serial.begin(9600);
  pinMode(button_a, INPUT_PULLUP);

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc.SAMConfig();
  
  Serial.println("Waiting for an ISO14443A Card ...");
  
}

int button_a_state = 0;
void loop()
{
  int button_a_value = digitalRead(button_a);

  //Serial.println(button_a_value);

  if((button_a_state == 0)&&(!button_a_value))
  {
    Serial.println("Button Pressed.");
    button_a_state = 1;
    
    irsend.sendNEC(cmd_go, 32); // NEC code
  }
  else if((button_a_state == 1)&&(button_a_value))
  {
    Serial.println("Button Up.");
    button_a_state = 0;
  }

  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    
    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ... 
      uint32_t cardid = uid[0];
      cardid <<= 8;
      cardid |= uid[1];
      cardid <<= 8;
      cardid |= uid[2];  
      cardid <<= 8;
      cardid |= uid[3]; 
      Serial.print("Seems to be a Mifare Classic card #");
      Serial.println(cardid);
      if(cardid == 4078001477)
      {
        Serial.println("Robot Move.");
        irsend.sendNEC(cmd_go, 32); // NEC code
      }
      else if(cardid == 321227731)
      {
        Serial.println("Robot Stop.");
        irsend.sendNEC(cmd_stop, 32); // NEC code
      }
      else if(cardid == 2244008733)
      {
        Serial.println("Robot Random Stop.");
        irsend.sendNEC(cmd_random_stop, 32); // NEC code
      }
    }
    Serial.println("");
  }
}

