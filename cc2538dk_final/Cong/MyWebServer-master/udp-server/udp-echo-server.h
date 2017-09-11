#ifndef UDP_ECHO_SERVER_H_
#define UDP_ECHO_SERVER_H_
/*
struct cmd_struct_t {
  uint8_t   sfd;
  uint16_t  seq;
  uint8_t   type;
  uint8_t   len;
  uint8_t   cmd;
  uint8_t   arg[4];
  uint16_t  crc;    
};*/
struct cmd_struct_t{
  uint8_t cmd;
  uint8_t nodeID;
  uint8_t errorCode;  
  uint8_t padding;
  //telosB data:
  uint16_t    temperature;
  uint16_t    lux;
  uint16_t    humidity;
  //Arduino data:
  uint16_t    temp_dht;
  uint16_t    humidity_dht;
  uint16_t    temp;
  uint16_t    pH;
  uint16_t    tur;
  uint16_t    sfr;
};
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
struct sensor_t{
  uint16_t temperature;
  uint16_t humidity;
  uint16_t lux;
};

enum{
  //system state
  IDLE      =0x00,
  WORKING     =0x01
};
#endif
