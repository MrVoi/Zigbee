
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

#include "dev/light-sensor.h"
#include "dev/sht11/sht11-sensor.h"

#include <stdio.h>
#include <math.h>

#include <string.h>

#define MAX_CMD_DATA_LEN  20
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "dev/watchdog.h"
#include "dev/leds.h"
#include "net/rpl/rpl.h"

#include "udp-echo-server.h"
#include "dev/uart1.h"
/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

#define MAX_PAYLOAD_LEN 120
static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static uint16_t len;
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static struct etimer et;

static struct cmd_struct_t cmd, reply;
static struct uip_udp_conn *server_conn;
static char buf[MAX_PAYLOAD_LEN];
static uint16_t len;
static unsigned int state;

/*****************************************************************************/
static void sensorHandler(void);
static void tcpip_handler(void);
static void initSystem(void);
/*---------------------------------------------------------------------------*/
PROCESS(udp_echo_server_process, "UDP echo server process");
AUTOSTART_PROCESSES(&udp_echo_server_process);
/*---------------------------------------------------------------------------*/
static void tcpip_handler(void)
{

    PRINTF("TCP/IP HANDLER\n");
    memset(buf, 0, MAX_PAYLOAD_LEN);
    if (uip_newdata()) {
        len = uip_datalen();
        memcpy(buf, uip_appdata, len);
        PRINTF("%u bytes from [", len);
        PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
        PRINTF("]:%u\n", UIP_HTONS(UIP_UDP_BUF->srcport));
        uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
        server_conn->rport = UIP_UDP_BUF->srcport;      
        
        memset(&reply, 0, sizeof(reply));
        cmd = *(struct cmd_struct_t *)(&buf);
        //if(cmd.cmd==CMD_TO_START)
         //   PRINTF("Cmd to start");
        //else if(cmd.cmd==CMD_TO_STOP)
        //    PRINTF("Cmd to stop");
        //else PRINTF("Unknow cmd");


        reply = cmd;

        if (cmd.cmd == CMD_TO_START)
        {  
            if(state==IDLE)
            { 
                PRINTF("Cmd to start\n");
                leds_on(LEDS_GREEN);
                
                reply.cmd=REPLY_START;
                state = WORKING;
            }
            else
            {   PRINTF("Already stated\n");
                reply.cmd=REPLY_START;
            }
        }
        else if (cmd.cmd == CMD_TO_STOP)
        {
            if(state==WORKING)
            {
                state = IDLE;
                leds_off(LEDS_GREEN);
                PRINTF("Cmd to stop\n");
                reply.cmd=REPLY_STOP;
            }
            else
            {
                PRINTF("Already stop\n");
                reply.cmd=REPLY_STOP;
            }

        }
        else
        {
            PRINTF("Unknow cmd\n");
            reply.cmd=REPLY_UNKNOW;
            
        }
        uip_udp_packet_send(server_conn, &reply, sizeof(reply));
        if(cmd.cmd==CMD_TO_STOP)
        {
            PRINTF("Delete addr\n");
            uip_create_unspecified(&server_conn->ripaddr);
            server_conn->rport = 0;
        }
    }
    return;
}
/******************************************************************************/
static void initSystem(void)
{
    state = IDLE;
}

/******************************************************************************/

static void sensorHandler(void)
{

    static int val;
    static float s;
    SENSORS_ACTIVATE(light_sensor);
    SENSORS_ACTIVATE(sht11_sensor);
    val = sht11_sensor.value(SHT11_SENSOR_TEMP);
    if (val != -1)
    {
        s = ((0.01*val) - 39.60);
        reply.temperature = (uint16_t)(s * 100);
        //dec = s;
        //frac = s - dec;
        //printf("\nTemperature=%d.%02u C (%d)\n", dec, (unsigned int)(frac * 100),val);
    }
    val = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
    if (val != -1)
    {
        s = (((0.0405*val) - 4) + ((-2.8 * 0.000001)*(pow(val, 2))));
        reply.humidity = (uint16_t)(s * 100);
        //dec = s;
        //frac = s - dec;
        //printf("Humidity=%d.%02u %% (%d)\n", dec, (unsigned int)(frac * 100),val);
    }
    val = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);
    if (val != -1)
    {
        s = (float)(val * 0.4071);
        reply.lux = (uint16_t)(s * 100);
        //dec = s;
        //frac = s - dec;
        //printf("Light=%d.%02u lux (%d)\n", dec, (unsigned int)(frac * 100),val);
    }
    SENSORS_DEACTIVATE(light_sensor);
    SENSORS_DEACTIVATE(sht11_sensor);
}

/******************************************************************************/

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_echo_server_process, ev, data)
{

    PROCESS_BEGIN();
    PRINTF("Starting UDP echo server\n");
    initSystem();
    server_conn = udp_new(NULL, UIP_HTONS(0), NULL);
    udp_bind(server_conn, UIP_HTONS(3000));

    PRINTF("Listen port: 3000, TTL=%u\n", server_conn->ttl);
    etimer_set(&et, CLOCK_SECOND*1);
    while (1) {
        PROCESS_YIELD();
        if (ev == tcpip_event) {
            tcpip_handler();
        }
        
        else if (ev = PROCESS_EVENT_TIMER)
        {
            if(state==WORKING)
            {
                PRINTF('Sending data\n');
                reply.cmd=REPLY_DATA;
                sensorHandler();
                uip_udp_packet_send(server_conn, &reply, sizeof(reply));
            }
            etimer_restart(&et);
        }
    }
    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
* @}
* @}
*/
