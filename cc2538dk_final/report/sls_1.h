/*
|-------------------------------------------------------------------|
| HCMC University of Technology                                     |
| Telecommunications Departments                                    |
| Wireless Embedded Firmware for Smart Lighting System (SLS)        |
| Version: 1.0                                                      |
| Author: sonvq@hcmut.edu.vn                                        |
| Date: 01/2017                                                     |
|-------------------------------------------------------------------|
*/

#ifndef SLS_H_
#define SLS_H_

//#define IEEE802154_CONF_PANID		0xABCD
#define SLS_PAN_ID	 IEEE802154_CONF_PANID


enum {
	SLS_NORMAL_PORT			= 3000,
	SLS_EMERGENCY_PORT		= 3001,
};

/*---------------------------------------------------------------------------*/
/* This is the UDP port used to receive data */
/* Response will be echoed back to DST port */
#define UDP_SERVER_LISTEN_PORT   	SLS_NORMAL_PORT
#define UDP_CLIENT_SEND_PORT   		SLS_EMERGENCY_PORT


/*
SLS_CC2538DK_HW = 1 : for compiling to CC2538dk
SLS_CC2538DK_HW = 0 : for compiling to SKY used in Cooja simulation
*/
#define SLS_CC2538DK_HW		0


#if (SLS_CC2538DK_HW)
#define SLS_USING_CC2538DK
#else
#define SLS_USING_SKY
#endif

#define	SFD 	0x7F

//redefine leds
#define BLUE		LEDS_BLUE
#define RED			LEDS_RED
#define GREEN		LEDS_GREEN

#define SLS_START	"start"
#define SLS_STOP	"stop"
#define SLS_ID 		"id"

#define SLS_SFD		"A"
#define SLS_SFR		"sfr"
#define SLS_TUR		"tur"
#define SLS_PH		"pH"
#define SLS_TEMP	"temp"

#define SLS_GET_SENS_STATUS	"get_sens_status"
#define SLS_GET_GW_STATUS	"get_gw_status"
#define SLS_GET_NW_STATUS	"get_nw_status"

#define GW_ID_MASK		0x0000
#define LED_ID_MASK		0x1000
#define METTER_ID_MASK	0x2000
#define ENV_ID_MASK		0x4000


#define MAX_CMD_DATA_LEN	20
#define MAX_CMD_LEN	sizeof(cmd_struct_t)

typedef enum {false=0, true=1} bool;

#define DEFAULT_EMERGENCY_STATUS true

enum{
  //error code
  	ERROR_TIMEOUT=0x00,
};
enum {
  //type of reply message
    //reply
CMD_TO_START= 0x00,
CMD_TO_STOP=  0x01,
CMD_HELLO=    0X03,
REPLY_HELLO=  0X04,
REPLY_START=  0x05,
REPLY_STOP=   0x06,
REPLY_DATA=   0x08,
REPLY_UNKNOW= 0x09,
};


enum{
  //system state
  IDLE      =0x00,
  WORKING     =0x01
};

enum {
	//command id
	CMD_GET_NW_STATUS 		= 0xFE,
	CMD_GET_GW_STATUS 		= 0xFD,
	CMD_GW_HELLO			= 0xFC,
	CMD_GW_SHUTDOWN			= 0xFB,
	CMD_GW_TURN_ON_ALL		= 0xFA,
	CMD_GW_TURN_OFF_ALL		= 0xF9,
	CMD_GW_DIM_ALL			= 0xF8,

	CMD_GET_RF_STATUS 		= 0xFF,
	CMD_RF_LED_OFF			= 0xF7,
	CMD_RF_LED_ON			= 0xF6,
	CMD_RF_LED_DIM			= 0xF5,
	CMD_RF_HELLO 			= 0xF4,
	CMD_RF_TIMER_ON 		= 0xF3,
	CMD_RF_TIMER_OFF 		= 0xF2,
	CMD_SET_APP_KEY			= 0xF1,
	CMD_GET_APP_KEY			= 0xF0,
	CMD_RF_REBOOT			= 0xEF,
	CMD_RF_REPAIR_ROUTE		= 0xEE,

};


/*---------------------------------------------------------------------------*/
//	used by gateway
struct gw_struct_t {
	uint16_t	id;			/*0000xxxx xxxxxxxx */
	uint16_t	panid;		
	uint8_t		status;
	/* data of device */
	uint16_t	voltage;
	uint16_t	current;
	uint16_t	power;
	uint16_t	temperature;
	uint16_t	lux;
};

/*---------------------------------------------------------------------------*/
struct sens_struct_t {
	uint16_t	id;			/*0001xxxx xxxxxxxx */
	uint16_t  	panid;						
	uint8_t		status;
	char		sfd;
	/* data of device */

	uint16_t		temp_dht;
	uint16_t		humidity_dht;
	uint16_t		temp;
	uint16_t		pH;
	uint16_t		tur;
	uint16_t		sfr;
};

/*---------------------------------------------------------------------------*/
/* This data structure is used to store the packet content (payload) */
struct net_struct_t {
	uint8_t			channel;	
	int8_t			rssi;
	int8_t			lqi;
	int8_t			tx_power;
	uint16_t		panid;
	uint16_t		node_addr;
	unsigned char	app_code[16];
};

/*---------------------------------------------------------------------------*/
//	sfd = 0x7F
//	seq: transaction id;
//	type: 	REQUEST/REPLY/HELLO
//	len: 	used for App node_id
//	cmd:	command id
//	err_code: code returned in REPLY, sender check this field to know the REQ status
//	arg[16]: data payload
struct cmd_struct_t {
  uint8_t cmd;
  uint8_t nodeID;
  uint8_t errorCode;  
  uint8_t padding;
  //telosB data:
  uint16_t    temperature;
  uint16_t    humidity;
  uint16_t    lux;
  //Arduino data:
  uint16_t    temp_dht;
  uint16_t    humidity_dht;
  uint16_t    temp;
  uint16_t    pH;
  uint16_t    tur;
  uint16_t    sfr;
};


typedef struct cmd_struct_t		cmd_struct_t;
typedef struct net_struct_t		net_struct_t;
typedef struct gw_struct_t		gw_struct_t;
typedef struct sens_struct_t	sens_struct_t;



#endif /* SLS_H_ */
