/*
|-------------------------------------------------------------------|
| HCMC University of Technology                                     |
| Telecommunications Departments                                    |
| Wireless Embedded Firmware for Smart Lighting System (SLS)        |
| Version: 1.0                                                      |
| Author: sonvq@hcmut.edu.vn                                        |
| Date: 01/2017                                                     |
|-------------------------------------------------------------------|*/
/* Description:
- Running on HW platform: TelosB, CC2538
*/


#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <stdlib.h>
#include <string.h>
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "dev/leds.h"
#include "net/rpl/rpl.h"
#include "dev/watchdog.h"
#include "dev/uart1.h" 
//#include "dev/button-sensor.h"

//#include "lib/ringbuf.h"

//#ifdef SLS_USING_CC2538DK
//#include "dev/uart1.h"
//#endif


#include "sls.h"	

/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#define MAX_PAYLOAD_LEN 120

/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static uint16_t len;

/* SLS define */
static 	led_struct_t led_db;
//static struct led_struct_t *led_db_ptr = &led_db;

static 	gw_struct_t gw_db;
static 	net_struct_t net_db;
//static struct led_struct_t *gw_db_ptr = &gw_db;

static 	cmd_struct_t cmd, reply, emer_reply;
//static 	cmd_struct_t *cmdPtr = &cmd;

static char str_reply[50]; //khai bao mang str_reply co 50 ky tu=50 bytes
static char str_cmd[10]; //khai bao mang str_cmd co 10 ky tu=10 bytes
static char str_arg[10]; //khai bao mang str_arg co 10 ky tu=10 bytes
static char str_rx[MAX_PAYLOAD_LEN]; //khai bao mang str_rx co 120 ky tu=120 bytes

static 	radio_value_t aux;
static	int	state;

//#ifdef SLS_USING_CC2538DK
static  char rxbuf[MAX_PAYLOAD_LEN];		/* used for uart1 interface */
static 	int cmd_cnt;
//#endif

/*define timers */
static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;
static	struct	etimer	et;
//static	struct	rtimer	rt;
static bool	emergency_status;


/* define prototype of fucntion call */
//static 	void set_connection_address(uip_ipaddr_t *ipaddr);
static 	void get_radio_parameter(void);
static 	void init_default_parameters(void);
static 	void reset_parameters(void);

static char *p; //dinh nghia con tro p co kieu char

//#ifdef 	SLS_USING_CC2538DK
static 	unsigned int uart1_send_bytes(const	unsigned  char *s, unsigned int len);
static 	int uart1_input_byte(unsigned char c);
//static 	unsigned int uart1_send_bytes(const	unsigned  char *s, unsigned int len);
//static 	int uart1_input_byte(unsigned char c);
//#endif 

static 	void send_cmd_to_led_driver();
static	void process_hello_cmd(cmd_struct_t command);
static	void print_cmd_data(cmd_struct_t command);
static 	void send_reply (cmd_struct_t res);
static	void blink_led (unsigned char led);
static 	bool is_cmd_of_nw (cmd_struct_t cmd);
static 	bool is_cmd_of_led(cmd_struct_t cmd);

/*---------------------------------------------------------------------------*/
PROCESS(udp_echo_server_process, "UDP echo server process");
AUTOSTART_PROCESSES(&udp_echo_server_process);

/*---------------------------------------------------------------------------*/
static void process_req_cmd(cmd_struct_t cmd){
	//uint8_t i;
	reply = cmd;
	reply.type =  MSG_TYPE_REP;
	reply.err_code = ERR_NORMAL;

	if (state==STATE_NORMAL) {
		switch (cmd.cmd) {
#ifdef	SLS_USING_SKY			
			case CMD_RF_LED_ON:
				//leds_on(RED);
				led_db.status = STATUS_LED_ON;
				//PRINTF ("Execute CMD = %s\n",SLS_LED_ON);
				//send_cmd_to_led_driver();
				break;
			case CMD_RF_LED_OFF:
				//leds_off(RED);
				led_db.status = STATUS_LED_OFF;
				//PRINTF ("Execute CMD = %d\n",CMD_LED_OFF);
				//send_cmd_to_led_driver();
				break;
			case CMD_RF_LED_DIM:
				//leds_toggle(GREEN);
				led_db.status = STATUS_LED_DIM;
				led_db.dim = cmd.arg[0];			
				//PRINTF ("Execute CMD = %d; value %d\n",CMD_LED_DIM, led_db.dim);
				//send_cmd_to_led_driver();
				break;
			case CMD_GET_RF_STATUS:
				reply.arg[0] = led_db.id;
				reply.arg[1] = led_db.power;
				reply.arg[2] = led_db.temperature;
				reply.arg[3] = led_db.dim; 
				reply.arg[4] = led_db.status;
				//send_cmd_to_led_driver();
				break;
#endif			
			/* network commands */				
			case CMD_RF_REBOOT:
				send_reply(reply);
				clock_delay(5000000);
				watchdog_reboot();
				break;
			case CMD_GET_NW_STATUS:
				reply.arg[0] = net_db.channel;
				reply.arg[1] = net_db.rssi;
				reply.arg[2] = net_db.lqi;
				reply.arg[3] = net_db.tx_power; 
				reply.arg[4] = (net_db.panid >> 8);
				reply.arg[5] = (net_db.panid) & 0xFF;		
				break;
			case CMD_GET_GW_STATUS:
				break;
			case CMD_GET_APP_KEY:
				memcpy(&reply.arg,&net_db.app_code,16);
				break;
			case CMD_RF_REPAIR_ROUTE:
				rpl_repair_root(RPL_DEFAULT_INSTANCE);
				break;
			default:
				reply.err_code = ERR_UNKNOWN_CMD;			
		}
	}
	else if (state==STATE_HELLO) {
		//PRINTF("in HELLO state: no process REQ cmd\n");	
		reply = cmd;	
		reply.err_code = ERR_IN_HELLO_STATE;
	}
	
}

/*---------------------------------------------------------------------------*/
static void process_hello_cmd(cmd_struct_t command){
	reply = command;
	reply.type =  MSG_TYPE_HELLO;
	reply.err_code = ERR_NORMAL;

	if (state==STATE_HELLO) {
		switch (command.cmd) {
			case CMD_RF_HELLO:
				state = STATE_HELLO;
				leds_off(LEDS_RED);
				break;
			case CMD_SET_APP_KEY:
				state = STATE_NORMAL;
				leds_on(LEDS_RED);
				memcpy(&net_db.app_code,&cmd.arg,16);
				break;
			default:
				reply.err_code = ERR_IN_HELLO_STATE;
				break;
		}	
	}
}

/*---------------------------------------------------------------------------*/
static void print_cmd_data(cmd_struct_t command) {
	uint8_t i;	
  	PRINTF("data = [");
	for (i=0;i<MAX_CMD_DATA_LEN;i++) 
    	PRINTF("0x%02X,",command.arg[i]);
  	PRINTF("]\n");
}

/*---------------------------------------------------------------------------*/
static void send_reply (cmd_struct_t res) {
	/* echo back to sender */	
	PRINTF("Reply to [");
	PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
	PRINTF("]:%u %u bytes\n", UIP_HTONS(UIP_UDP_BUF->srcport), sizeof(res));
	uip_udp_packet_send(server_conn, &res, sizeof(res));

	/* Restore server connection to allow data from any node */
	uip_create_unspecified(&server_conn->ripaddr);
	memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
	server_conn->rport = 0;
//#ifdef SLS_USING_CC2538DK
	blink_led(LEDS_GREEN);
//#else
	//blink_led(RED);	
//#endif	
}

static bool is_cmd_of_nw (cmd_struct_t cmd) {
	return (cmd.cmd==CMD_GET_NW_STATUS) ||
			(cmd.cmd==CMD_GET_GW_STATUS) ||
			(cmd.cmd==CMD_RF_HELLO) ||
			(cmd.cmd==CMD_RF_REPAIR_ROUTE) ||
			(cmd.cmd==CMD_GW_HELLO) ||		
			(cmd.cmd==CMD_SET_APP_KEY) ||		
			(cmd.cmd==CMD_GET_APP_KEY);	
}

static bool is_cmd_of_led (cmd_struct_t cmd) {
	return !is_cmd_of_nw(cmd);
	/*
	return (cmd.cmd==CMD_LED_ON) ||
			(cmd.cmd==CMD_LED_OFF) ||
			(cmd.cmd==CMD_LED_DIM);
	*/
}

/*---------------------------------------------------------------------------*/
static void tcpip_handler(void)	{
	//char *search = " ";
	//uart1_active();
	//uart1_input_byte();
	memset(buf, 0, MAX_PAYLOAD_LEN); //set bo dem ve 0
  	if(uip_newdata()) { //Neu ham nay bang 1, chop tat led xanh
  		blink_led(GREEN);
    	len = uip_datalen(); //tra ket qua uip_datalen() vao bien len
    	memcpy(buf, uip_appdata, len); //sao chep len bytes tu vi tri uip_appdata tro toi den vi tri buf tro toi
    	PRINTF("Received from ["); //in ra dong: Received from [
    	PRINT6ADDR(&UIP_IP_BUF->srcipaddr); //in dia chi ipv6 voi bien la dia chi cua &UIP_IP_BUF->srcipaddr
    	PRINTF("]:%u ", UIP_HTONS(UIP_UDP_BUF->srcport)); //in ra ket qua cua ham UIP_HTONS(UIP_UDP_BUF->srcport)
		PRINTF("%u bytes DATA\n",len,buf); //in ra do dai byte va du lieu trong bo dem buf
		
    	uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr); //goi ham uip_ipaddr_copy(dia chi cua bien server_conn->ripaddr, dia chi cua bien UIP_IP_BUF->srcipaddr)
    	server_conn->rport = UIP_UDP_BUF->srcport; //gan gia tri cua bien UIP_UDP_BUF->srcport vao bien server_conn->rport

		get_radio_parameter(); //goi ham lay thong so radio: get_radio_parameter
		reset_parameters(); //goi ham reset lai cac thong so: reset_parameters;
		
		//chep source:
		strcpy(str_rx,buf); //copy chuoi tu buf vao str_rx
		if (SLS_CC2538DK_HW) //neu SLS_CC2538DK_HW=1 thi thuc hien ham duoi, neu SLS_CC2538DK_HW=0 thi thuc hien dong duoi
			sscanf(str_rx,"%s %s",str_cmd, str_arg); //ham sscanf la gi vay nhi?
		else {
			/* used for SKY */
    		//PRINTF("str_rx = %s", str_rx);
  			p = strtok (str_rx," ");  //ham strtok la gi? ->cat chuoi(chuoi can cat, dau tach chuoi)=>Lay token dau tien. Cat tai vi tri " "
			if (p != NULL) { //neu p khac null thi tiep tuc cat
				strcpy(str_cmd,p); //copy chuoi tu p vao str_cmd
    			p = strtok (NULL, " ,");
				if (p != NULL) {
					strcpy(str_arg,p); //copy chuoi tu p vao str_arg
				}			
			}
		}
				//PRINTF("CMD = %s ARG = %s\n",str_cmd, str_arg);
		/*if (strstr(str_cmd,SLS_UART_ON)!=NULL){
			PRINTF("Execute CMD = %s\n",SLS_UART_ON);
			uart1_input_byte(cmd.sfd);
			uart1_send_bytes((const unsigned  char *)(&cmd), sizeof(cmd));
			//blink_led(BLUE);
			sprintf(str_reply,"Replied=%s",str_rx);
		}*/	
		if (strstr(str_cmd,SLS_LED_ON)!=NULL) { //ham strstr(chuoi de dc quet=str_cmd,chuoi nho de dc tim kiem trong chuoi dc quet str_cmd=SLS_LED_ON)
			//tach phan tu SLS_LED_ON tu trong str_cmd. Neu khac NULL thi thuc hien lenh duoi 
			PRINTF ("Execute CMD = %s\n",SLS_LED_ON); //in ra dong thi hanh lenh : sls_led_on
			leds_on(RED);
			sprintf(str_reply, "Replied = %s", str_rx); //tra ve so luong ky tu duoc in ra trong chuoi str_reply, 
			//tuc la dinh dang str_rx ve kieu string va luu trong str_reply roi in ra man hinh
			led_db.status = STATUS_LED_ON;
		}
		else if (strstr(str_cmd, SLS_LED_OFF)!=NULL) {
			PRINTF ("Execute CMD = %s\n",SLS_LED_OFF);
			leds_off(RED);
			sprintf(str_reply, "Replied = %s", str_rx);
			led_db.status = STATUS_LED_OFF;
		}
		else if (strstr(str_cmd, SLS_LED_DIM)!=NULL) {
			PRINTF ("Execute CMD = %s to value %s",SLS_LED_DIM, str_arg);
			leds_toggle(LEDS_BLUE); //xo vao led(led xanh)
			sprintf(str_reply, "Replied = %s\n", str_rx);
			led_db.status = STATUS_LED_DIM;
			led_db.dim = atoi(str_arg); //chuyen doi gia tri cua str_arg thanh so nguyen
		}
		else if (strstr(str_cmd, SLS_GET_LED_STATUS)!=NULL) {
			sprintf(str_reply, "Replied: id=%u;power=%u;temp=%d;dim=%u;status=0x%02X; \n", led_db.id,
					led_db.power,	led_db.temperature, led_db.dim, led_db.status); //u=so nguyen thap phan ko dau , d=so nguyen thap phan co dau
		}		
		else if (strstr(str_cmd, SLS_GET_NW_STATUS)!=NULL) {
			sprintf(str_reply, "Replied: channel=%u;rssi=%ddBm;lqi=%u;tx_power=%ddBm;panid=0x%02X;\n", 
					net_db.channel, net_db.rssi, net_db.lqi, net_db.tx_power, net_db.panid);
		}		
		else {
			reset_parameters();
			sprintf(str_reply,"unknown cmd\n"); //in ra unknow cmd
		}
		PRINTF("str_reply=%s\n",str_reply); //in ra chuoi str_reply
		
		//send_reply(str_reply);
		//cai nay tuong duong voi goi ham send_reply(str_reply)
		
		/* echo back to sender */	//doi lai cho ben phat
    	PRINTF("Echo back to ["); //in ra dong Echo back to [
    	PRINT6ADDR(&UIP_IP_BUF->srcipaddr); //
    	PRINTF("]:%u %u bytes\n", UIP_HTONS(UIP_UDP_BUF->srcport), sizeof(str_reply)); //in ra ket qua cua ham uip_htons(UIP_UDP_BUF->srcport) va kich thuoc bo nho khung str_reply
    	//uip_udp_packet_send(server_conn, "Server-reply\n", sizeof("Server-reply"));
    	uip_udp_packet_send(server_conn, str_reply, sizeof(str_reply)); //goi ham uip_udp_packet_send(server_conn, str_reply, sizeof(str_reply))
    	uip_create_unspecified(&server_conn->ripaddr); //goi ham uip_create_unspecified(&server_conn->ripaddr)
    	server_conn->rport = 0; //gan bien server->rpot=0
		
		//leds_off(RED); //tat led
		
		//ket thuc phan chep source
		
		//p = &buf;	cmdPtr = (cmd_struct_t *)(&buf);
		cmd = *(cmd_struct_t *)(&buf); //ep kieu cho du lieu con tro dang tro den bo dem buf
		PRINTF("Rx Cmd-Struct: sfd=0x%02X; len=%d; seq=%d; type=0x%02X; cmd=0x%02X; err_code=0x%02X\n",cmd.sfd, cmd.len, 
										cmd.seq, cmd.type, cmd.cmd, cmd.err_code);
		print_cmd_data(cmd);
		
		reply = cmd;		
		/* get a REQ */
		if (is_cmd_of_nw(cmd)){
			if (cmd.type==MSG_TYPE_REQ) {
				process_req_cmd(cmd);
				reply.type = MSG_TYPE_REP;
			}
			/* get a HELLO */
			else if (cmd.type==MSG_TYPE_HELLO) {
				process_hello_cmd(cmd);	
				reply.type = MSG_TYPE_HELLO;
				//send_reply(reply);	
			}
			else if (cmd.type==MSG_TYPE_EMERGENCY) {
			}
			send_reply(reply);
		}	

		/* LED command */
//#ifdef SLS_USING_CC2538DK		
		/* send command to LED-driver */
		//send_cmd_to_led_driver();
		if (is_cmd_of_led(cmd)){
			if (state==STATE_NORMAL) {
				send_cmd_to_led_driver();
			}	
		}	
//#else 	/* used for Cooja */
		//send_reply(reply);
//#endif
  	}

	return;
}

/*---------------------------------------------------------------------------*/
static void blink_led(unsigned char led) {
#ifdef SLS_USING_CC2538DK
	leds_on(led);
	clock_delay_usec((uint16_t)3000000);
	leds_off(led);
#endif	
}

/*---------------------------------------------------------------------------*/
//#ifdef SLS_USING_CC2538DK
static int uart1_input_byte(unsigned char c) {
	if (c==SFD) {
		cmd_cnt=1;
		rxbuf[cmd_cnt-1]=c;
	}
	else {
		cmd_cnt++;
		rxbuf[cmd_cnt-1]=c;
		if (cmd_cnt==sizeof(cmd_struct_t)) {		/* got the full reply */
			cmd_cnt=0;
			emer_reply = *((cmd_struct_t *)(&rxbuf));
			PRINTF("Get cmd from LED-driver %s \n",rxbuf);
			/* processing emergency reply */
			if (emer_reply.err_code == ERR_EMERGENCY) {
				emergency_status = true;
			}
			else {	/*update local db */
			}

			reply = emer_reply;
			send_reply(reply);		/* got a Reply from LED-driver, send to orginal node */
			blink_led(RED); /*goi ham blink_led de chop tat led xanh*/
		}
	}
	return 1;
}

/*---------------------------------------------------------------------------*/
static unsigned int uart1_send_bytes(const	unsigned  char *s, unsigned int len) {
	unsigned int i;
	for (i = 0; i<len; i++) {
		uart1_writeb((uint8_t) (*(s+i)));
   	}   
   return 1;
}
//#endif


/*---------------------------------------------------------------------------*/
static void send_cmd_to_led_driver() {
//#ifdef SLS_USING_CC2538DK
	uart1_send_bytes((const unsigned  char *)(&cmd), sizeof(cmd));	
//#endif
}

/*---------------------------------------------------------------------------*/
static void reset_parameters(void) {
	memset(&reply, 0, sizeof(reply));
}

/*---------------------------------------------------------------------------*/
static void get_radio_parameter(void) {
	NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &aux);
	net_db.channel = (unsigned int) aux;
	//printf("CH: %u ", (unsigned int) aux);	

 	aux = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	net_db.rssi = (int8_t)aux;
 	//printf("RSSI: %ddBm ", (int8_t)aux);

	aux = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
	net_db.lqi = aux;
 	//printf("LQI: %u\n", aux);

	NETSTACK_RADIO.get_value(RADIO_PARAM_TXPOWER, &aux);
	net_db.tx_power = aux;
 	//printf("   Tx Power %3d dBm", aux);
}

/*---------------------------------------------------------------------------*/
static void init_default_parameters(void) {

	state = STATE_HELLO;

	led_db.id		= LED_ID_MASK;				
	led_db.panid 	= SLS_PAN_ID;
	led_db.power	= 120;
	led_db.dim		= 80;
	led_db.status	= STATUS_LED_ON; 
	led_db.temperature = 37;

	gw_db.id		= GW_ID_MASK;				
	gw_db.panid 	= SLS_PAN_ID;
	gw_db.power		= 150;
	gw_db.status	= GW_CONNECTED; 

	cmd.sfd  = SFD;
	cmd.seq	 = 0;
	cmd.type = MSG_TYPE_REP;
	cmd.len  = sizeof(cmd_struct_t);

	net_db.panid 	= SLS_PAN_ID;

	emergency_status = DEFAULT_EMERGENCY_STATUS;

	// init uart1-1
//#ifdef SLS_USING_CC2538DK
	uart1_init(0); 		
 	uart1_set_input(uart1_input_byte);
//#endif

}

/*---------------------------------------------------------------------------*/

static void set_connection_address(uip_ipaddr_t *ipaddr) {
  // change this IP address depending on the node that runs the server!
  uip_ip6addr(ipaddr, 0xaaaa,0x0000,0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001); //aaaa::1/64
}


/*---------------------------------------------------------------------------*/
static void timeout_hanler(){
	static int seq_id;
//	char buf[100];

	if (state==STATE_NORMAL) {	
		if (emergency_status==true) {	
			sprintf(buf, "Emergency msg %d from the client", ++seq_id);
			uip_udp_packet_send(client_conn, &emer_reply, sizeof(emer_reply));
			emergency_status = false;
			/* debug only*/	
			PRINTF("Client sending to: ");
			PRINT6ADDR(&client_conn->ripaddr);
			//PRINTF(" (msg: %s)\n", emer_reply);
		}
	}
}


/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_echo_server_process, ev, data) {

	PROCESS_BEGIN();

	//PROCESS_PAUSE();
	//SENSORS_ACTIVATE(button_sensor);

  	NETSTACK_MAC.off(1); //de bo receiver hoat dong, tat cycling cua rx
	PRINTF("Initialization...\n"); //in ra dong Initialization=khoi tao
	init_default_parameters();

	server_conn = udp_new(NULL, UIP_HTONS(0), NULL); ////goi ham server_conn=udp_new(NULL,UIP_HTONS(0),NULL)
  	if(server_conn == NULL) { //Mo 1 ket noi moi ra, neu ko mo dc ket noi thi thoat khoi process
    	PROCESS_EXIT();
  	}
  	
  	udp_bind(server_conn, UIP_HTONS(SLS_NORMAL_PORT)); //bind: rang buoi, goi ham udp_bind(server_conn, UIP_HTONS(SLS_NORMAL_PORT=3000))
	PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl); //in ra dong lang nghe PORT 3000, gan TTL=server_conn->ttl
	
	//khac:
	etimer_set(&et, CLOCK_SECOND*30); //Set timer 30s
  	set_connection_address(&server_ipaddr); //goi ham set_connection_address(bien=dia chia server_ipaddr)
	client_conn = udp_new(&server_ipaddr, UIP_HTONS(SLS_EMERGENCY_PORT), NULL); //gan vo bien client_conn=udp_new(bien1=dia chi server_ipaddr, bien2=UIP_HTONS(SLS_EMERGENCY_PORT=3001),NULL )

 	while(1) { //vong lap chip
    	PROCESS_YIELD();
    	if(ev == tcpip_event) {//neu ev=tcpip_event->true thi thuc hien ham tcpip_handler()
      		tcpip_handler();
    	}
    	else if (ev==PROCESS_EVENT_TIMER) { //Neu ev=tcpip_event->false, xet tiep neu ev=PROCESS_EVENT_TIMER thi goi ham timeout_hanler() va reset bien et
    		timeout_hanler();
    		etimer_restart(&et);
    	}
  	}

	PROCESS_END();
}
