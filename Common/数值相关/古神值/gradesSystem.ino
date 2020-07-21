/*
 Name:		netUnit.ino
 Created:	2020/6/12 6:22:02
 Author:	joeyl
*/

#include <ArduinoJson.h>
#include <Ethernet.h>
#include <WhiteHarbor.h>

// the setup function runs once when you press reset or power the board


EthernetClient client;
int read_Page_Mode = 0;
bool isMsgDeal = true;

byte mMac[] = MAC_GRADE_SYSTEM;
IPAddress mIP IP_GARDE_SYSTEM;

unsigned long time_now = 0;
unsigned long heart_time_mark = 0;

#define PIN_BTN_OVERCOME 21
bool btn_overcome = false;
bool btn_overcome_bak = false;

char * clientConnect();
char * readPage();
void init();
void HeartBeat();
void ParseTheMsg(char argv[]);
void bak_save();
void readFromDevice();

int falseConnectNum = 1;
bool isConnected = false;

void readFromDevice() {
	btn_overcome = digitalRead(PIN_BTN_OVERCOME);

	if (btn_overcome_bak == HIGH && btn_overcome == LOW)
	{
		client.print("{\"MsgType\":\"CompleteQue\",\"MsgContent\":{\"Mcu_grade_system\":\"A1\",\"Grade\" : 1}}");
	}
}
void bak_save(){
	btn_overcome_bak = btn_overcome;
}
void init() {

	pinMode(PIN_BTN_OVERCOME,INPUT_PULLUP);

	if (isDebugOn)
	{
		Serial.println("grads system  MCU START ");
	}
	


}
void HeartBeat() {
	Serial.println("heart beating one time");
	heart_time_mark = time_now;
	client.print("{\"MsgType\":\"Heart\",\"MsgContent\":{\"Mcu_grade_system\":\"A1\",\"McuStatus\" : 0}}");
}

void ParseTheMsg(char argv[])
{
	Serial.println("get to the parse function");
	for (int i = 0; i < 200; i++) { Serial.print(argv[i]); }Serial.println("");

	if (isMsgDeal == false)
	{
		const size_t capacity = JSON_ARRAY_SIZE(2) + 3 * JSON_OBJECT_SIZE(2) + 100;
		DynamicJsonDocument doc1(capacity);


		deserializeJson(doc1, argv);

		const char* MsgType = doc1["MsgType"]; // "LightControl"

		Serial.println("---------------");
		Serial.print(MsgType);
		Serial.println("");
		// According to the MsgType desides whitch pattern of parsing will be chosen
		if (MsgType == "LightControl")
		{


			const char* MsgContent_0_LightID = doc1["MsgContent"][0]["LightID"]; // "L_1"
			int MsgContent_0_LightStatus = doc1["MsgContent"][0]["LightStatus"]; // 0

			const char* MsgContent_1_LightID_ = doc1["MsgContent"][1]["LightID "]; // "L_1 "
			int MsgContent_1_LightStatus = doc1["MsgContent"][1]["LightStatus"]; // 0
		}
		if (MsgType[0] == 'D')
		{
			int dorStatus = doc1["DoorStatus"];
			// Serial.print("doorstatus  is   :    ");
			// Serial.println(dorStatus);
			delay(100);
			client.print("i get the message ---");
			//client.print(dorStatus);
			client.print("bye");
		}
		isMsgDeal = true;
	}
}
char * readPage() {
	char inString[1024];
	int stringPos = 0;
	int loopcounte = 0;
	bool startRead = false;
	//read the page, and capture & return everything between '<' and '>'
	memset(&inString, 0, sizeof(inString)); //clear inString memory
   //Serial.println("stuck in readPage");
	if (client.available()) {
		while (true) {
			loopcounte++;
			if (loopcounte >= 1024) { memset(&inString, 0, sizeof(inString)); loopcounte = 0; Serial.println("bad msg"); return inString; }
			char c = client.read();
			//Serial.println("stuck in readPage--char c");     
			//delay(10);
  //        Serial.print("³£¹æ×Ö·û  c : ");
  //        Serial.print(c);
  //        Serial.println("");
			if (c == '{') {                   //'{' is our begining character//Ready to start reading the part 
				startRead = true;
				//delay(10);
				//Serial.println("¶Áµ½ÁË×óÀ¨ºÅ");
				inString[stringPos] = c;
				delay(10);
				stringPos++;
				read_Page_Mode = read_Page_Mode + 1;

			}
			else if (startRead) {
				if (read_Page_Mode == 0) // end 
				{
					//delay(10);
					Serial.println("¶Á½áÊøÁË");
					client.flush();
					//  cmdRoot = parseServerCmd(inString, &cmd);    // there is where parse the Msg
					  //Serial.println(inString);
					Serial.println("end of read msg");
					for (size_t i = 0; i <= stringPos; i++)
					{
						Serial.print(inString[i]);
					}
					Serial.println("");
					startRead = false;
					isMsgDeal = false;
					ParseTheMsg(inString);//   ´«²Î char Êý×é ½øÀ´

					return inString;
				}
				else {
					//delay(10);

					if (c == '}')
					{
						// Serial.println("¶Áµ½ÁËÓÒÀ¨ºÅ");
						read_Page_Mode = read_Page_Mode - 1;
					}

					inString[stringPos] = c;
					stringPos++;
				}



			}
		}
	}
}
char* clientConnect() {
	if (!client.connected()) {
		client.stop();
		Serial.println("begin");
		if (client.connect(IP_SERVER, SERVER_PORT)) {
			Serial.println("end");


			Serial.println("success!");
			isConnected = true;
		}
		else {
			isConnected = false;
			falseConnectNum++;
			Serial.println("failed!");
			if (falseConnectNum > 3) {
				Serial.println("false time over 3");
			}
		}
	}
	else {
		char *c = readPage();
		return c;
	}
}
void setup() {
	Serial.begin(9600);
	delay(200);
	init();

	Ethernet.begin(mMac, mIP);
}

// the loop function runs over and over again until power down or reset
void loop() {
	readFromDevice();
	time_now = millis();
	char *a = clientConnect();

	HeartBeat();
	bak_save();
}
