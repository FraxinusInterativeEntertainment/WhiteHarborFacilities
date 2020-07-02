/*
 Name:		blueStopWithoutScreen.ino
 Created:	2020/7/2 3:48:49
 Author:	joeyl
*/

// the setup function runs once when you press reset or power the board


#include <ArduinoJson.h>
#include <Ethernet.h>
#include <WhiteHarbor.h>
#include <DFRobot_PN532.h>
//CARD READ 
#define BLOCK_SIZE       16
#define  PN532_IRQ        2
#define  INTERRUPT        1
#define  POLLING          0
// The block to be read
#define  READ_BLOCK_NO    2


#define PIN_LOCK_BALL 21
#define PIN_GET_BALL 22 
#define PIN_OPRAT_MACHINEA 23
#define PIN_OPRAT_MACHINEB 24
#define PIN_DEGREE_DECIDEA 25
#define PIN_DEGREE_DECIDEB 26
#define PIN_OPEN_LOCK 27     // 使用自保持button
#define PIN_QUIT_LOG 28

bool getBall = HIGH;
bool btnQuit = HIGH;
bool openLock = HIGH;
bool getBall_bak = HIGH;
bool btnQuit_bak = HIGH;

bool isOpratAHigh = false;




 
DFRobot_PN532_IIC  nfc(PN532_IRQ, POLLING);
uint8_t dataRead[16] = { 0 };
// numbers that matters to recognize card
#define NUMBER_CARD_MATTER (4)
uint8_t IDLog[NUMBER_CARD_MATTER] = { 0 };

int GAME_MODE = 0; //0 : 待机等待登录
/*					1 ：查询登录等返回（刷到卡） 
					2 ：管理登录（可无限出药）到5 
					3 : 玩家登录（按照数量跳转）     *** 在 parse CMD里已经处理掉 此gamemode无意义
					4 ：非法登录 转回待机0
					5 ：等待按钮出药 如果还有余额继续提取 无了就等待退出
					  
					7 ：退出账号/返回0
					*/
int GAME_MODE_bak = 0;

					// the setup function runs once when you press reset or power the board

EthernetClient client;
int read_Page_Mode = 0;
bool isMsgDeal = true;
bool isMsgSent = true;
byte mMac[] = MAC_NET_UNIT;
IPAddress mIP IP_NET_UNIT;

unsigned long time_now = 0;
unsigned long heart_time_mark = 0;

int NumberOfBalls = 0;

bool isLedInformed = false;

char * clientConnect();
char * readPage();
void init();
void HeartBeat();
void ParseTheMsg(char argv[]);

void checkGameMode();
void NFCreadCard();
void bak_save();
void sendMsgToServer();
void readFromDevice();
void opratingFunction();
void getOneBall();



int falseConnectNum = 1;
bool isConnected = false;

void getOneBall() {
	if (isOpratAHigh) {
		isOpratAHigh = !isOpratAHigh;
		digitalWrite(PIN_OPRAT_MACHINEA, LOW);
		digitalWrite(PIN_OPRAT_MACHINEB, HIGH);
	}
	else
	{
		isOpratAHigh = !isOpratAHigh;
		digitalWrite(PIN_OPRAT_MACHINEA, HIGH);
		digitalWrite(PIN_OPRAT_MACHINEB, LOW);
	}
}
void readFromDevice() {
	getBall = digitalRead(PIN_GET_BALL);
	openLock = digitalRead(PIN_OPEN_LOCK);
	btnQuit = digitalRead(PIN_QUIT_LOG);
}
void opratingFunction() {
	if (getBall_bak == HIGH && getBall == LOW )
	{
		if (NumberOfBalls < 0)
		{
			getOneBall();
		}
		else if (NumberOfBalls > 0) {
			getOneBall();
			NumberOfBalls -= 1;
		}
	}
	if (btnQuit_bak == HIGH && btnQuit == LOW)
	{
		GAME_MODE = 7;
		isLedInformed = false;

	}
}
void sendMsgToServer() {     // 待商量 api
	switch (GAME_MODE)
	{
	case 0:
		client.print("{\"MsgType\":\"LOGIN\",\"MsgContent\":{\"id\":\"A1\",\"McuStatus\" : 0}}");
		isMsgSent = true;
		break;
	case 7:
		client.print("{\"MsgType\":\"Quit\",\"MsgContent\":{\"Mcu_net_unit\":\"A1\",\"McuStatus\" : 0}}");
		isMsgSent = true;
		break;
	default:
		break;
	}
}
void bak_save() {
	if (isDebugOn)
	{
		if (GAME_MODE_bak!=GAME_MODE)
		{
			Serial.print("game mode now is  : ");
			Serial.println(GAME_MODE);
		}
	}
	GAME_MODE_bak = GAME_MODE;
	getBall_bak = getBall;
	btnQuit_bak = btnQuit;
	
}
void NFCreadCard() {

	if (nfc.scan()) {
		if (nfc.readData(dataRead, READ_BLOCK_NO) != 1) {
			if (isDebugOn)
			{
				Serial.print("Block ");
				Serial.print(READ_BLOCK_NO);
				Serial.println(" read failure!");
			}

		}
		else {
			if (isDebugOn)
			{
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
			for (int i = 0; i < NUMBER_CARD_MATTER; i++)
			{
				IDLog[i] = dataRead[i];
			}
			isMsgSent = false;

		}
		delay(100);
	}
}
void  checkGameMode() {
	switch (GAME_MODE)
	{
		case 0://  nfc read 
			NFCreadCard();
			if (isMsgSent == false)
			{
				sendMsgToServer();
			}
			if (isLedInformed == false)
			{
				Serial1.println("WaitingCard");
				isLedInformed = true;
			}
			GAME_MODE = 1;
			break;
		case 1:	// wait to judge cmd
			if (isDebugOn)
			{
				Serial.println("now waiting cmd and parse it ");
			}
			if (isLedInformed == false)
			{
				Serial1.print("Waiting");
				isLedInformed = true;
			}
			break;
		case 2:
			if (isDebugOn)
			{
				Serial.println("now is authority login ");
			}
			if (isLedInformed == false)
			{
				Serial1.println("AuthorityMode");
				isLedInformed = true;
			}
			break;
		case 3:
			break;
		case 4:  //  wrongcard
			if (isDebugOn)
			{
				Serial.println("wrong card");
			}
			if (isLedInformed == false)
			{
				Serial1.println("WrongCard");
				isLedInformed = true;
				GAME_MODE = 0;
			}
			break;
		case 5:
			if (isDebugOn)
			{
				Serial.println("opration ing");
			}
			if (isLedInformed == false)
			{
				Serial1.println("oprationing");
				isLedInformed = true;
			}
			opratingFunction();
			break;
		case 6:
			break;
		case 7:
			if (isDebugOn)
			{
				Serial.println("quiting");
			}
			if (isLedInformed == false)
			{
				Serial1.println("Quiting");
				isLedInformed = true;
			}
			GAME_MODE = 0;
			isLedInformed = false;	
			break;
	default:
		break;
	}
}
void init() {

	pinMode(PIN_LOCK_BALL,OUTPUT);
	pinMode(PIN_GET_BALL, INPUT_PULLUP);
	pinMode(PIN_OPRAT_MACHINEA, OUTPUT);
	pinMode(PIN_OPRAT_MACHINEB,OUTPUT);
	pinMode(PIN_DEGREE_DECIDEA, OUTPUT);
	pinMode(PIN_DEGREE_DECIDEB, OUTPUT);
	pinMode(PIN_OPEN_LOCK, INPUT_PULLUP);
	pinMode(PIN_QUIT_LOG, INPUT_PULLUP);
	digitalWrite(PIN_LOCK_BALL,HIGH);
	digitalWrite(PIN_OPRAT_MACHINEA,LOW);
	digitalWrite(PIN_OPRAT_MACHINEB,HIGH);
	digitalWrite(PIN_DEGREE_DECIDEA,HIGH);
	digitalWrite(PIN_DEGREE_DECIDEB, LOW);


	if (isDebugOn)
	{
		Serial.println("blueStop  MCU START ");
		Serial.print("Initializing");
		while (!nfc.begin()) {
			Serial.print(".");
			delay(1000);
		}
		Serial.println();
		Serial.println("Waiting for a card......");
	}


}
void HeartBeat() {
	Serial.println("heart beating one time");
	heart_time_mark = time_now;
	client.print("{\"MsgType\":\"Heart\",\"MsgContent\":{\"Mcu_net_unit\":\"A1\",\"McuStatus\" : 0}}");
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
		if (strcmp(MsgType,"LightControl") == 0)
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
		if (strcmp(MsgType,"Authority") == 0)   // 待协商是何种api 
		{
			GAME_MODE = 2;
			NumberOfBalls = -1;
			isLedInformed = false;
		}
		if (strcmp(MsgType,"PlayerNow") == 0) // 玩家身份登录则 返回次数 NumberOfBalls
		{
			if (NumberOfBalls == 0)
			{
		
				GAME_MODE = 5;
			}
		}
		if (strcmp(MsgType,"WrongCard")) {
			GAME_MODE = 4;
			isLedInformed = false;
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
				//Serial.println("read left }");
				inString[stringPos] = c;
				delay(10);
				stringPos++;
				read_Page_Mode = read_Page_Mode + 1;

			}
			else if (startRead) {
				if (read_Page_Mode == 0) // end 
				{
					//delay(10);
					Serial.println("stop read ");
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
	Serial1.begin(115200); // 串口通讯屏幕 未确定型号
	delay(200);
	init();

	Ethernet.begin(mMac, mIP);
}

// the loop function runs over and over again until power down or reset
void loop() {
	readFromDevice();
	time_now = millis();
	char *a = clientConnect();
	checkGameMode();

	HeartBeat();
	bak_save();

}

 