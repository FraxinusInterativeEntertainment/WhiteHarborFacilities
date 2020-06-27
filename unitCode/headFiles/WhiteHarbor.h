#pragma once

// mcu self 
#define isDebugOn (true)

#define TIME_HEARTBEAT (3000)

//  MSG with server
#define FirstTitle "MsgType"
#define SecondTitle "MsgContent"

// CMD  with own mcu

//net 
#define SERVER_PORT (55666)

#define IP_1 (192)
#define IP_2 (168)
#define IP_3 (0)
#define IP_SERVER (IP_1 , IP_2 , IP_3 , 100)
#define IP_NET_UNIT (IP_1 , IP_2 , IP_3 , 101)
#define IP_COMMON_STOP (IP_1 , IP_2 , IP_3 , 102);

#define MAC_NET_UNIT {0xEA , 0x00 , 0x00 , 0x00 , 0x00 , 0x01}
#define MAC_COMMOM_STOP {0xEA , 0x00 , 0x00 , 0x00 , 0x00 , 0x02}