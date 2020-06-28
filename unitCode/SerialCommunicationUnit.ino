/*
 Name:		SerialCommunicationUnit.ino
 Created:	2020/6/20 14:18:39
 Author:	joeyl
*/

// the setup function runs once when you press reset or power the board

#include <WhiteHarbor.h>

#define CMDLENTH (10)

char CMDup[CMDLENTH];
char CMDup_bak[CMDLENTH];
char cmd_up;

int num;

bool isCMDdeal;
bool isSendCMD = false;

char CMDdown[CMDLENTH];

void getCMDFromUp();		//read cmd from up mcu through serial1
void saveCMDtoBAK();		// save cmd to bak 
void clearOrder();         //clear all cmd / up or down
void decodeCMD();			// decode cmd depend on type or first 
void SendCMD();				//send cmd to down mcu through  serial2


void SendCMD(){
	for (size_t i = 0; i < CMDLENTH; i++)
	{
		Serial2.print(CMDdown[i]);
	}
	

	
}

void decodeCMD() {
	if (!isCMDdeal)
	{
		//parse code 
	}
	isCMDdeal = true;
}
void clearOrder() { 
	for (size_t i = 0; i < CMDLENTH; i++)
	{
		CMDup[i] = '0';
		CMDdown[i] = '0';
	}
}
void saveCMDtoBAK() {
	for (size_t i = 0; i < CMDLENTH; i++)
	{
		CMDup_bak[i] = CMDup[i];
	}
}
void getCMDFromUp() {
	if (Serial1.available() > 0)
	{
		if (Serial1.peek() != '\n')
		{
			cmd_up = (char)Serial1.read();
			CMDup[num] = cmd_up;
			num++;
		}
		else {
			Serial2.read();
			num = 0;
			isCMDdeal = false;
		}
	}
}

void setup() {
	Serial.begin(9600);
	Serial1.begin(115200);// up  send
	Serial2.begin(115200);//down receieve
}

// the loop function runs over and over again until power down or reset
void loop() {
	getCMDFromUp();
	decodeCMD();
	if (isSendCMD)
	{
		SendCMD();
	}
}
