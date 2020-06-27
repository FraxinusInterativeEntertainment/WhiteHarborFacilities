/*
 Name:		cardReader.ino
 Created:	2020/6/13 23:52:24
 Author:	joeyl
*/
// the setup function runs once when you press reset or power the board


#include <DFRobot_PN532.h>

#define BLOCK_SIZE       16
#define  PN532_IRQ        2
#define  INTERRUPT        1
#define  POLLING          0
// The block to be read
#define  READ_BLOCK_NO    2

DFRobot_PN532_IIC  nfc(PN532_IRQ, POLLING);
uint8_t dataRead[16] = { 0 };

void setup() {
	Serial.begin(115200);
	Serial.print("Initializing");
	while (!nfc.begin()) {
		Serial.print(".");
		delay(1000);
	}
	Serial.println();
	Serial.println("Waiting for a card......");
}
void loop() {
	if (nfc.scan()) {
		if (nfc.readData(dataRead, READ_BLOCK_NO) != 1) {
			Serial.print("Block ");
			Serial.print(READ_BLOCK_NO);
			Serial.println(" read failure!");
		}
		else {
			Serial.print("Block ");
			Serial.print(READ_BLOCK_NO);
			Serial.println(" read success!");

			Serial.print("Data read(string):");
			Serial.println((char *)dataRead);
			Serial.print("Data read(HEX):");
			for (int i = 0; i < BLOCK_SIZE; i++) {
				Serial.print(dataRead[i], HEX);
				Serial.print(" ");
				dataRead[i] = 0;
			}
			Serial.println();
		}
		delay(500);
	}
}
