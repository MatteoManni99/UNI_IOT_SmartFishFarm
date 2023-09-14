#include "contiki.h"

#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uiplib.h"

#include "sys/ctimer.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"

#include "sys/etimer.h"
#include "dev/leds.h"
#include "dev/button-hal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sys/log.h"
#define LOG_MODULE "Actuator Temp"
#define LOG_LEVEL LOG_LEVEL_INFO

extern coap_resource_t res_heat_pump;

static struct etimer process_timer;

#define STATE_TIMER (CLOCK_SECOND * 2)

#define SERVER_IP "coap://[fd00::1]:5683"
#define ID_ACTUATOR 1
#define TYPE_ACTUATOR "heat_pump"

static bool registered = false;

static char reg_payload[50];

static bool have_connection(void)
{
    if(uip_ds6_get_global(ADDR_PREFERRED) == NULL || uip_ds6_defrt_choose() == NULL)
    {
        return false;
    }
    return true;
}

void handler(coap_message_t *response){
    const uint8_t *chunk;
    coap_get_payload(response, &chunk);

    if(response != NULL)
    {   
        if(strcmp((char*)chunk, "REGIST") == 0)
        {
	    LOG_INFO("Registration completed\n");
            registered = true;
	    // Partiamo da una condizione di OFF, imposto il led su GREEN
	    leds_on(LEDS_GREEN);
        }
    }

    if(!registered)
    {
        LOG_INFO("Something went wrong in Registration\n");
    }
}

PROCESS(coap_server, "Tempereature Actuator");
AUTOSTART_PROCESSES(&coap_server);

PROCESS_THREAD(coap_server, ev, data)
{
    PROCESS_BEGIN();

    static coap_endpoint_t server_ep;
    static coap_message_t request[1];

    coap_activate_resource(&res_heat_pump, "res_heat_pump");

    etimer_set(&process_timer, STATE_TIMER);

    coap_endpoint_parse(SERVER_IP, strlen(SERVER_IP), &server_ep);

    while(1)
    {
	
        PROCESS_YIELD();
	
        if(ev == PROCESS_EVENT_TIMER && data == &process_timer)
        {
            // Registra
            if(have_connection())
            {
                LOG_INFO("Connected to Border Router\n");
		
                // Invia una richiesta di registrazione BLOCCANTE
                if(!registered)
                {
		    // Stoppo il timer perhè non ha senso che esegua in parallelo alla richiesta bloccante, altrimenti manderebbe eventi
		    etimer_stop(&process_timer);

		    int len_payload = sprintf(reg_payload, "{\"actuator_id\": \"%d\",\"type\":\"%s\"}", ID_ACTUATOR, TYPE_ACTUATOR);

                    coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
                    coap_set_header_uri_path(request, "registrator");
                    coap_set_payload(request, reg_payload, len_payload);

		    LOG_INFO("Sending registration request to Application Coap Server\n");
                    COAP_BLOCKING_REQUEST(&server_ep, request, handler);

		    // Faccio ripartire il timer
	            etimer_set(&process_timer, STATE_TIMER);
                }
                
                else
                {
		    LOG_INFO("Registred in Application DataBase\n");
                }
            }
            else
            {
                LOG_INFO("Connecting to Border Router\n");
            }

        }

        etimer_reset(&process_timer);
	
    }

    PROCESS_END();
}
