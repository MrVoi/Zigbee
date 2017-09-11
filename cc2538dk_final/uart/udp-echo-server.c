/*
 * Copyright (c) 2012, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** \addtogroup cc2538-examples
 * @{
 *
 * \defgroup cc2538-echo-server cc2538dk UDP Echo Server Project
 *
 *  Tests that a node can correctly join an RPL network and also tests UDP
 *  functionality
 * @{
 *
 * \file
 *  An example of a simple UDP echo server for the cc2538dk platform
 */
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "dev/watchdog.h"
#include "dev/leds.h"
#include "net/rpl/rpl.h"
#include "dev/leds.h"
#include "dev/uart1.h"
#include "lib/ringbuf.h"
#include "sys/clock.h"
#include "sls_1.h"	

/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#define MAX_PAYLOAD_LEN 120
/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static uint16_t len;


/* SLS define */
static sens_struct_t sens;
static net_struct_t net_db;
static cmd_struct_t cmd;

static uip_ipaddr_t bripaddr;
static uint16_t port;

static char str_reply[80];
static char str_cmd[10];
static char str_arg[10];
static char str_rx[MAX_PAYLOAD_LEN];

unsigned int i=0;
char str_urx[8];

char d[8];
  
static 	radio_value_t aux;

static 	char *p;

/* define prototype of fucntion call */
static void get_radio_parameter(void);
static void init_default_parameters(void);
static void reset_parameters(void);
static int str_tarx(unsigned char c);
static void sens_getdata(void);
void get_border_router_addr(void);
void set_border_router_addr(void);
/*---------------------------------------------------------------------------*/
PROCESS(udp_echo_server_process, "UDP echo server process");
AUTOSTART_PROCESSES(&udp_echo_server_process);
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
	//char *search = " ";
  memset(buf, 0, MAX_PAYLOAD_LEN);
  if(uip_newdata()) {
    leds_on(LEDS_RED);
    len = uip_datalen();
    memcpy(buf, uip_appdata, len);
    //PRINTF("Received from [");
    //PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    //PRINTF("]:%u ", UIP_HTONS(UIP_UDP_BUF->srcport));
	//PRINTF("%u bytes DATA: %s\n",len, buf);
		
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
    server_conn->rport = UIP_UDP_BUF->srcport;

		get_border_router_addr();
		get_radio_parameter();
		reset_parameters();
		
		strcpy(str_rx,buf);
    		p = strtok (str_rx," ");  
			if (p != NULL) {
				strcpy(str_cmd,p);
    			p = strtok (NULL, " ,");
				if (p != NULL) {
					strcpy(str_arg,p);
				}			
			}
		}
		//PRINTF("str_rx = %s", str_rx);
  		
		
		//PRINTF("CMD = %s ARG = %s\n",str_cmd, str_arg);		
		if (strstr(str_cmd,SLS_START)!=NULL) {
			//PRINTF ("Execute CMD = %s\n",SLS_LIGHT_ON);
			uart1_writeb('s');
			uart1_writeb('t');
			uart1_writeb('a');
			uart1_writeb('r');
			uart1_writeb('t');
			uart1_writeb('\n');	
			while(1){
				if(i==8){
					sprintf(str_reply,"Replied=%s\n",str_urx);
					memset(&str_urx[0],0,sizeof(str_urx));
					i=0;
					break;
				}
			}
				
		}
		else if (strstr(str_cmd,SLS_STOP)!=NULL){
			uart1_writeb('s');
			uart1_writeb('t');
			uart1_writeb('o');
			uart1_writeb('p');
			uart1_writeb('\n');
		}
		else if (strstr(str_cmd, SLS_TEMP)!=NULL) {
			//PRINTF ("Execute CMD = %s\n",SLS_FAN_ON);
			uart1_writeb('t');
			sprintf(str_reply, "Replied=%d\n", sens.temp);
		}
		else if (strstr(str_cmd, SLS_SFR)!=NULL) {
			//PRINTF ("Execute CMD = %s\n",SLS_FAN_OFF);
			uart1_writeb('d');
			sprintf(str_reply, "Replied=%d\n", sens.sfr);
		}
		else if (strstr(str_cmd, SLS_GET_SENS_STATUS)!=NULL) {
			sprintf(str_reply, "Replied:id=%u;temp=%d;light=%s;alarm=%s;rain=%s;falarm=%s;fan=%s; \n", sens.id,	sens.temp, sens.light, sens.alarm, sens.rain, sens.falarm, sens.fan);
		}		
		else if (strstr(str_cmd, SLS_GET_NW_STATUS)!=NULL) {
			sprintf(str_reply, "Replied:channel=%u;rssi=%ddBm;lqi=%u;tx_power=%ddBm;panid=0x%02X; \n", 
					net_db.channel, net_db.rssi, net_db.lqi, net_db.tx_power, net_db.panid);
		}		
		else {
			reset_parameters();
			sprintf(str_reply,"unknown cmd");
		}
		//PRINTF("str_reply=%s\n",str_reply);
	

	/* echo back to sender */	
   /* PRINTF("Echo back to [");
    PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
    PRINTF("]:%u %u bytes\n", UIP_HTONS(UIP_UDP_BUF->srcport), sizeof(str_reply));*/
    //uip_udp_packet_send(server_conn, "Server-reply\n", sizeof("Server-reply"));
    uip_udp_packet_send(server_conn, str_reply, sizeof(str_reply));
    uip_create_unspecified(&server_conn->ripaddr);
    server_conn->rport = 0;
  
  leds_toggle(LEDS_GREEN);
  return;
}
/*---------------------------------------------------------------------------*/
void set_border_router_addr(void){	
	uip_ipaddr_copy(&server_conn->ripaddr,&bripaddr);
	server_conn->rport = UIP_UDP_BUF->srcport;
}
/*---------------------------------------------------------------------------*/
void get_border_router_addr(void){	
	uip_ipaddr_copy(&bripaddr,&UIP_IP_BUF->srcipaddr);
	port= UIP_UDP_BUF->srcport;
}
/*---------------------------------------------------------------------------*/
static void reset_parameters(void) {
	memset(&str_cmd[0], 0, sizeof(str_cmd)); //sao chep ky tu 0 toi n=sizeof(str_cmd) ky tu dau tien cua chuoi duoc tro toi boi tham so &str_cmd[0]
	memset(&str_arg[0], 0, sizeof(str_arg)); //sao chep ky tu 0 toi n=sizeof(str_arg) ky tu dau tien cua chuoi duoc tro toi boi tham so &str_arg[0]
	memset(&str_reply[0], 0, sizeof(str_reply)); //sao chep ky tu 0 toi n=sizeof(str_reply) ky tu dau tien cua chuoi duoc tro toi boi tham so &str_reply[0]
}

/*---------------------------------------------------------------------------*/
static void get_radio_parameter() {
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
	

	cmd.sfd = 0x7E;
	cmd.seq	= 1;
	cmd.type = MSG_TYPE_REP;
	cmd.len = 7;

	net_db.panid = SLS_PAN_ID;

	sens.id = 0x80;
	sens.panid =SLS_PAN_ID;
	sens.temp= 31;

}
/*---------------------------------------------------------------------------*/

static int str_tarx(unsigned char c)
{
	if (i>=3){
		sens_getdata();
	}

	str_urx[i]=c;
	i++;
	return 1;
}
/*---------------------------------------------------------------------------*/
static void sens_getdata(void){

if(strstr(str_urx,SLS_SFD)!=NULL){
	PRINTF("chuoi str_urx= %s",str_urx);
	leds_toggle(LEDS_BLUE);
	sens.sfd=SLS_SFD;
	memset(&str_urx[0],0,sizeof(str_urx));
	i=0;
	PRINTF("%s\n",sens.sfd);
}
if(strstr(str_urx,SLS_ID)!=NULL){
	PRINTF("chuoi str_urx= %s",str_urx);
	leds_toggle(LEDS_BLUE);
	memset(&str_urx[0],0,sizeof(str_urx));
	i=0;
	//PRINTF("%s\n",d);
}
if(strstr(str_urx,SLS_PH)!=NULL){
	sens.pH=(atoi(str_urx))/10;	
	memset(&str_urx[0],0,sizeof(str_urx));
	i=0;
	printf("%d\n",sens.pH);
}
if(strstr(str_urx,SLS_TEMP)!=NULL){
	PRINTF("chuoi str_urx= %s",str_urx);
	leds_toggle(LEDS_BLUE);
	//str_urx[0]=0;
	//str_urx[1]=0;
	sens.temp= atoi(str_urx); //chuyen chuoi str_urx thanh so nguyen kieu long va gan vo bien sens.temp.
	memset(&str_urx[0],0,sizeof(str_urx));
	i=0;
	PRINTF("%d\n",sens.temp);
	//uart1_writeb('y');
}
if(strstr(str_urx,SLS_TUR)!=NULL){
	sens.tur=atoi(str_urx);
	memset(&str_urx[0],0,sizeof(str_urx));
	i=0;
	printf("%d\n",sens.tur);
}
if(strstr(str_urx,SLS_SFR)!=NULL){
	leds_on(BLUE);
	//str_urx[0]=0;
	//str_urx[1]=0;
	sens.sfr=atoi(str_urx);
	memset(&str_urx[0],0,sizeof(str_urx));
	i=0;
	printf("%d\n",sens.sfr);
}

}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_echo_server_process, ev, data)
{


  PROCESS_BEGIN();

	PRINTF("Initialization....\n");
	init_default_parameters();
	uart1_init(UART1_BAUD2UBR(115200));	
  PRINTF("Starting UDP echo server\n");

  server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
  udp_bind(server_conn, UIP_HTONS(3000));

  PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl);
  uart1_set_input(str_tarx);

  while(1) {
    PROCESS_YIELD();
		uart1_set_input(str_tarx);
    	if(ev == tcpip_event) {
      		tcpip_handler();
    	}	
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */