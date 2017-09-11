#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"
#include "dev/watchdog.h"
#include "dev/leds.h"
#include "net/rpl/rpl.h"
#include "dev/button-sensor.h"
#include "debug.h"

#include "dev/uart0.h"

#include "sls.h"

/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#define MAX_PAYLOAD_LEN 120
#define SEND_INTERVAL		5 * CLOCK_SECOND

/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *server_conn;
static struct uip_udp_conn *g_conn;
/*---------------------------------------------------------------------------*/
static char buf[MAX_PAYLOAD_LEN];
static uint16_t len;

static unsigned char rx[8];

/*---------------------------------------------------------------------------*/
/* SLS define */
static 	led_struct_t led_db;

static 	gw_struct_t gw_db;

static 	net_struct_t net_db;

static 	cmd_struct_t cmd, reply, emer_reply;


/*---------------------------------------------------------------------------*/
//static 	radio_value_t aux;
static	int	state;
/*---------------------------------------------------------------------------*/
static bool	emergency_status;
/*---------------------------------------------------------------------------*/

/* define prototype of fucntion call */

//static 	void get_radio_parameter(void);
static 	void init_default_parameters(void);
static 	void reset_parameters(void);

static 	void send_cmd_to_led_driver();
static	void process_hello_cmd(void);
static	void print_cmd_data(void);
static 	void send_reply (void);
static	void blink_led (unsigned char led);
static 	bool is_cmd(void);
static 	bool is_cmd_of_led(void);

static 	void buf2cmd(void);
static 	void cmd2reply();

static void change_emer(void);

/*---------------------------------------------------------------------------*/

PROCESS(udp_echo_server_process, "UDP echo server process");
AUTOSTART_PROCESSES(&udp_echo_server_process);

/*---------------------------------------------------------------------------*/
static void process_req_cmd(void){
	reply.type =  MSG_TYPE_REP;
	reply.err_code = ERR_NORMAL;

	if (state==STATE_NORMAL) {
		switch (cmd.cmd) {			
			case CMD_LED_ON:
				leds_on(LEDS_GREEN);
				led_db.status = STATUS_LED_ON;
				PRINTF ("Execute CMD = %s\n",CMD_LED_ON);
				
				break;
			case CMD_LED_OFF:
				leds_off(LEDS_GREEN);
				led_db.status = STATUS_LED_OFF;
				PRINTF ("Execute CMD = %d\n",CMD_LED_OFF);
				
				break;
			case CMD_LED_DIM:
				leds_toggle(LEDS_BLUE);
				led_db.status = STATUS_LED_DIM;
				led_db.dim = cmd.arg[0];			
				PRINTF ("Execute CMD = %d; value %d\n",CMD_LED_DIM, led_db.dim);
				
				break;
			case CMD_GET_LED_STATUS:
				reply.arg[0] = led_db.id;
				reply.arg[1] = led_db.power;
				reply.arg[2] = led_db.temperature;
				reply.arg[3] = led_db.dim; 
				reply.arg[4] = led_db.status;
				break;			
			/* network commands */				
			case CMD_LED_REBOOT:
				send_reply();
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
			case CMD_REPAIR_ROUTE:
				rpl_repair_root(RPL_DEFAULT_INSTANCE);
				break;
			default:
				reply.err_code = ERR_UNKNOWN_CMD;			
		}
	}
	else if (state==STATE_HELLO) {
		PRINTF("in HELLO state: no process REQ cmd\n");	
		reply.err_code = ERR_IN_HELLO_STATE;
	}
	
}	

/*---------------------------------------------------------------------------*/
static void process_hello_cmd(void){
	reply.type =  MSG_TYPE_HELLO;
	reply.err_code = ERR_NORMAL;

	if (state==STATE_HELLO) {
		switch (cmd.cmd) {
			case CMD_LED_HELLO:
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
static void print_cmd_data(void) {
	uint8_t i;	
  	PRINTF("data = [");
	for (i=0;i<MAX_CMD_DATA_LEN;i++) 
    	PRINTF("0x%02X,",cmd.arg[i]);
  	PRINTF("]\n");
}

/*---------------------------------------------------------------------------*/
static void send_reply (void) {
	/* echo back to sender */	
	PRINTF("Reply to [");
	PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
	PRINTF("]:%u %u bytes\n", UIP_HTONS(UIP_UDP_BUF->srcport), sizeof(reply));
	uip_udp_packet_send(server_conn, &reply, sizeof(reply));

	/* Restore server connection to allow data from any node */
	uip_create_unspecified(&server_conn->ripaddr);
	//memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
	server_conn->rport = 0;

	blink_led(LEDS_RED);		
}

static bool is_cmd(void) {
	return (cmd.cmd==CMD_GET_NW_STATUS) ||
			(cmd.cmd==CMD_GET_GW_STATUS) ||
			(cmd.cmd==CMD_LED_HELLO) ||
			(cmd.cmd==CMD_REPAIR_ROUTE) ||
			(cmd.cmd==CMD_GW_HELLO) ||		
			(cmd.cmd==CMD_SET_APP_KEY) ||		
			(cmd.cmd==CMD_GET_APP_KEY) ||
			(cmd.cmd==CMD_LED_ON) ||
			(cmd.cmd==CMD_LED_OFF) ||
			(cmd.cmd==CMD_LED_DIM);
}

/*---------------------------------------------------------------------------*/
static void tcpip_handler(void)	{
	
	memset(buf, 0, MAX_PAYLOAD_LEN);
  	if(uip_newdata()) {
  		blink_led(LEDS_RED);
    	len = uip_datalen();
    	memcpy(buf, uip_appdata, len);
		
    	PRINTF("Received from [");
    	PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    	PRINTF("]:%u ", UIP_HTONS(UIP_UDP_BUF->srcport));
			PRINTF("%u bytes DATA\n",len);
		
    	uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    	server_conn->rport = UIP_UDP_BUF->srcport;

			//get_radio_parameter();
			reset_parameters();
			
			buf2cmd();

			PRINTF("Rx Cmd-Struct: sfd=0x%02X; len=%d; seq=%d; type=0x%02X; cmd=0x%02X; err_code=0x%02X\n",cmd.sfd, cmd.len, 
										cmd.seq, cmd.type, cmd.cmd, cmd.err_code);
			print_cmd_data();
		
			cmd2reply();		
			/* get a REQ */
			if (is_cmd()){
				if (cmd.type==MSG_TYPE_REQ) {
					process_req_cmd();
					reply.type = MSG_TYPE_REP;
				}
				/* get a HELLO */
				else if (cmd.type==MSG_TYPE_HELLO) {
					process_hello_cmd();	
					reply.type = MSG_TYPE_HELLO;
					//send_reply(reply);	
				}
				else if (cmd.type==MSG_TYPE_EMERGENCY) {
				}
				send_reply();
			}
		else{	//echo lai goi tin da nhan	
			send_reply();
		}
  }

	return;
}

/*---------------------------------------------------------------------------*/
static void timeout_handler(void) {

  static int seq_id;
  struct uip_udp_conn *this_conn;

  leds_on(LEDS_RED);
  memset(buf, 0, MAX_PAYLOAD_LEN);
  seq_id++;


  this_conn = g_conn;
  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL) {
  	return;
  }

  PRINTF("Client to: ");
  PRINT6ADDR(&this_conn->ripaddr);

  memcpy(buf, &seq_id, sizeof(seq_id));

  PRINTF(" Remote Port %u,", UIP_HTONS(this_conn->rport));
  PRINTF(" (msg=0x%04x), %u bytes\n", *(uint16_t *) buf, sizeof(seq_id));

  uip_udp_packet_send(this_conn, buf, sizeof(seq_id));
  leds_off(LEDS_RED);
}


/*---------------------------------------------------------------------------*/
static void blink_led(unsigned char led) {
	leds_on(led);
	clock_delay_usec((uint16_t)3000000);
	leds_off(led);
}

/*---------------------------------------------------------------------------*/
static void reset_parameters(void) {
	memset(&reply, 0, sizeof(reply));
}

/*---------------------------------------------------------------------------*/
/*static void get_radio_parameter(void) {
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
}*/

/*---------------------------------------------------------------------------*/
static void init_default_parameters(void) {

	state = STATE_NORMAL;

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
}
/*---------------------------------------------------------------------------*/
static 	void buf2cmd(void)
{
	uint8_t i;
	cmd.sfd = (uint8_t)buf[0] ;
	cmd.len = (uint8_t)buf[1] ;
	cmd.seq = ((uint16_t) buf[2]<<8) || (uint16_t)buf[3];
	cmd.type = (uint8_t)buf[4];
	cmd.cmd = (uint8_t)buf[5];
	cmd.err_code = ((uint16_t) buf[6]<<8) || (uint16_t)buf[7];
	for (i=0;i<MAX_CMD_DATA_LEN;i++)  {
    	cmd.arg[i] = (uint8_t)buf[i+8];
	}
			
}
/*---------------------------------------------------------------------------*/
static 	void cmd2reply(void)
{
	uint8_t i;
	reply.sfd = cmd.sfd ;
	reply.len = cmd.len;
	reply.seq = cmd.seq;
	reply.type = cmd.type;
	reply.cmd = cmd.cmd;
	reply.err_code = cmd.err_code;
	for (i=0;i<MAX_CMD_DATA_LEN;i++)  {
    	reply.arg[i] = cmd.arg[i];
	}
			
}
static void change_emer(void){
	uip_ipaddr_t ipaddr;
	static uint16_t temp, pH, tur, dist;
	temp = ((uint16_t)rx[0])<<8 | (uint16_t)rx[1];
	pH = ((uint16_t)rx[2])<<8 | (uint16_t)rx[3];
	tur = ((uint16_t)rx[4])<<8 | (uint16_t)rx[5];
	dist = ((uint16_t)rx[6])<<8 | (uint16_t)rx[7];
	
	uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, ip[0], ip[1], ip[2], ip[3]);
	uip_ipaddr_copy(&g_conn->ripaddr, &ipaddr);
}
/*---------------------------------------------------------------------------*/
int uart_rx_callback(unsigned char c)
{
	static int i = 0;
	if (c == 0x7E){
		i=1;
		return 1;
	}
	if (i ==8){
		rx[i-1] = c;
		uart0_writeb(rx[i-1]);
		i=0;
		change_emer();
		return 1;
	}
	else if(i>0){
		rx[i-1] = c;
		uart0_writeb(rx[i-1]);
		i++;
		return 1;
	}
 	//uart0_writeb(c);
 	//return 1;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_echo_server_process, ev, data) {
	
	static struct etimer et;
	uip_ipaddr_t ipaddr;


	PROCESS_BEGIN();

  NETSTACK_MAC.off(1);

	init_default_parameters();

	server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
 	if(server_conn == NULL) {
    PROCESS_EXIT();
  }
  udp_bind(server_conn, UIP_HTONS(SLS_NORMAL_PORT));

	uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0x0212, 0x4b00, 0x72b, 0xf4b2);
  g_conn = udp_new(&ipaddr, UIP_HTONS(3005), NULL);
  if(!g_conn) {
    PRINTF("udp_new emer_conn error.\n");
  }
  udp_bind(g_conn, UIP_HTONS(3002));

  etimer_set(&et, SEND_INTERVAL);

  uart0_set_input(uart_rx_callback);

  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {
      timeout_handler();
      etimer_restart(&et);
    } else if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

	PROCESS_END();
}
