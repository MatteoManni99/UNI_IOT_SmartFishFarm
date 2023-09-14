#include "contiki.h"

#include "net/routing/routing.h"

#include "net/ipv6/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"

#include "sys/etimer.h"
#include "sys/ctimer.h"

#include "lib/sensors.h"
#include "dev/button-hal.h"
#include "dev/leds.h"

#include "os/sys/log.h"

#include "mqtt.h"
#include "mqtt-prop.h"

#include <string.h>
#include <strings.h>
#include <stdarg.h>


#define LOG_MODULE "Sensor Quality"
#define LOG_LEVEL LOG_LEVEL_INFO

// Setting Broker Parameters
#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1"
#define DEFAULT_BROKER_PORT 1883
static const char *broker_ip = MQTT_CLIENT_BROKER_IP_ADDR;

// Set sensor id - in our case there will be only one quality sensor with the id equals to 2
#define SENSOR_ID 2

// Setting other parameters
#define MAX_TCP_SEGMENT_SIZE 32
#define CONFIG_IP_ADDR_STR_LEN 64
#define BUFFER_SIZE 64
#define PUB_BUFFER_SIZE 64

#define SUB_TOPIC "quality_sampling"
#define PUB_TOPIC "quality"

// mqtt parameters declaration
mqtt_status_t status;

static struct mqtt_connection conn;
char broker_addr[CONFIG_IP_ADDR_STR_LEN];

static struct mqtt_message *msg_ptr = 0;

/*
msg_pointer->topic - Contains the topic
msg_pointer->payload_chunk - Contains the payload
msg_pointer->payload_lenght - Contains the payload lenghts
*/

// Buffers
static char client_id[BUFFER_SIZE];
static char pub_buffer[PUB_BUFFER_SIZE];

// Setting Periods
#define CONNECTION_PERIOD (CLOCK_SECOND * 2)
#define SAMPLING_PERIOD (CLOCK_SECOND * 2)

static int state_machine_period = SAMPLING_PERIOD;
static struct etimer periodic_state_timer;

// Defining MQTT States
static uint8_t state;

#define STATE_INIT              0
#define STATE_NET_OK            1
#define STATE_CONNECTING        2
#define STATE_CONNECTED         3
#define STATE_SUBSCRIBED        4
#define STATE_DISCONNECTED      5

PROCESS(mqtt_client_process, "MQTT Water Quality Sensing");
AUTOSTART_PROCESSES(&mqtt_client_process);

//Variables for sensing simulating
static bool filter = false;
static int quality = 100; //quality measured in percentage
static int variation = 0;

// Here should be the function that sense the quality
static void simulate_quality()
{   
    variation = rand()%(2); //rand from 0 to 1
    if(filter) quality = quality + variation;
    else quality = quality - variation;

    //cap the quality value to 100
    if (quality>100) quality = 100;
    else if (quality<0) quality = 0;
    LOG_INFO("%d\n",quality);
}


// Message handler for changing sampling period
// payload_chunk = {"sampling_period"= x} , the new sampling period will be x

static char* key = "\"sampling_period\":";
static int new_period = 0;
static char* value_position = "";

static void message_handler(const char *topic, const uint8_t *chunk)
{
    // Changing sampling period
    LOG_INFO("Message received at topic '%s': %s\n", topic, chunk);

    if(strcmp((char*)topic, "quality_sampling") == 0)
    {
	// Json parsing
    	value_position = strstr(((const char *)msg_ptr->payload_chunk), key);
    
    	if (value_position) {
           printf("%s\n", value_position);
           // Utilizza sscanf per leggere il valore (presumendo che sia un intero)
           
	   if (sscanf(value_position + strlen(key), "%d", &new_period) == 1)
	   {
		if(new_period == -1) // this case is only for simulating a filtering on/off, using an escamotage
		{
		    filter = !filter;
		}
		else
		{
		    printf("New quality sampling period: %d\n sec", new_period);
		    state_machine_period = (CLOCK_SECOND * new_period);
		}
       	   }
	   else
	   {
           	printf("Message received is not valid\n");
           }

    	}
	else
	{
           printf("Message received is not valid\n");
    	}
    }
}

//Check connection to Border Router
static bool have_connection(void)
{
    //Ritorna true solo se il nodo corrente ha un Public IP
    if(uip_ds6_get_global(ADDR_PREFERRED) == NULL || uip_ds6_defrt_choose() == NULL)
    {
        return false;
    }
    return true;
}

//Callback function  
static void mqtt_event (struct mqtt_connection *m, mqtt_event_t event, void *data)
{
    switch (event)
    {
        case MQTT_EVENT_CONNECTED:
            //Connessione riuscita
            LOG_INFO("MQTT connection event\n");
            state = STATE_CONNECTED;
            break;

        case MQTT_EVENT_DISCONNECTED:
            LOG_INFO("MQTT not connected\n");
            //Disconnessione
            state = STATE_DISCONNECTED;
            break;

        case MQTT_EVENT_PUBLISH:
            LOG_INFO("MQTT publish event\n");
            //Someone publish with a subscribed topic
            msg_ptr = data;
            message_handler(msg_ptr->topic, msg_ptr->payload_chunk);
            break;

        case MQTT_EVENT_SUBACK:
            //Subscribe Event
            #if MQTT_311
                mqtt_suback_event_t *suback_event = (mqtt_suback_event_t *)data;

                if(suback_event->success) 
                {
                    LOG_INFO("Application is subscribed to topic successfully\n");
                } 
                else
                {
                    LOG_INFO("Application failed to subscribe to topic (ret code %x)\n", suback_event->return_code);
                }
            #else
                LOG_INFO("Application is subscribed to topic successfully\n");
            #endif
            break;

        case MQTT_EVENT_UNSUBACK:
            //Unsubscribe Event - not utilized in our case
            LOG_INFO("Application is unsubscribed to topic successfully\n");
            
            break;

        case MQTT_EVENT_PUBACK:
            //Punlish Event
            LOG_INFO("Publishing complete.\n");
            break;

        case MQTT_EVENT_CONNECTION_REFUSED_ERROR:
            //Connection error
            LOG_INFO("Connection Refused.\n");
            break;

        default:
            LOG_INFO("Unhandled MQTT event occurs: %i\n", event);
            break;
    }
}

static void mqtt_state_machine()
{

    switch (state)
    {
	case STATE_INIT:
	
            LOG_INFO("State Init\n");

	    leds_off(LEDS_ALL);
	    leds_on(LEDS_GREEN);
	    leds_on(LEDS_RED);
		
	    mqtt_register(&conn, &mqtt_client_process, client_id, mqtt_event, MAX_TCP_SEGMENT_SIZE);
	    
	    //Registered to the Broker
            state = STATE_NET_OK;
		
            break;

        case STATE_NET_OK:
 
            LOG_INFO("State Net Ok\n");
            
	    if(have_connection())
            {
		memcpy(broker_addr, broker_ip, strlen(broker_ip));

		mqtt_connect(&conn, broker_addr, DEFAULT_BROKER_PORT, 3 * CLOCK_SECOND, MQTT_CLEAN_SESSION_ON);
            	// Connected to BorderRouter
   		state = STATE_CONNECTING;
            }
            
            break;

        case STATE_CONNECTING:
            LOG_INFO("Connecting\n");
        
            break;

        case STATE_CONNECTED:
            LOG_INFO("State Connected\n");

            status = mqtt_subscribe(&conn, NULL, SUB_TOPIC, MQTT_QOS_LEVEL_0);

            if (status == MQTT_STATUS_OUT_QUEUE_FULL)
            {
                LOG_ERR("Comand queue was full!\n");
            }

	    //Subscribed to a Topic
            state = STATE_SUBSCRIBED;
            break;

        case STATE_SUBSCRIBED:
            LOG_INFO("State Subscribed\n");

	    leds_off(LEDS_ALL);
	    leds_on(LEDS_GREEN);
	    
	    //Generating a fake temperature 
            simulate_quality();
	    
	    //Creating the json payload
            sprintf(pub_buffer, "{\"sensor_id\": \"%d\",\"quality\":\"%d\"}", SENSOR_ID, quality);
            LOG_INFO("Invio: %s\n", pub_buffer);
            mqtt_publish (&conn, NULL, PUB_TOPIC, (u_int8_t *)pub_buffer, strlen(pub_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
            break;

        case STATE_DISCONNECTED:
            //Disconnesso dal broker
            LOG_INFO("State Disconnected\n");
            state = STATE_NET_OK;
            break;
        
        default:
            break;
    }

    // Resetto il timer della state machine
    etimer_reset(&periodic_state_timer);
    if(state == STATE_SUBSCRIBED) etimer_set(&periodic_state_timer, state_machine_period);

}

PROCESS_THREAD(mqtt_client_process, ev, data)
{
    PROCESS_BEGIN();
    LOG_INFO("Starting MQTT Client\n");

    etimer_set(&periodic_state_timer, CONNECTION_PERIOD);
    sprintf(client_id, "%02x%02x%02x%02x%02x%02x",
            linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
            linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
            linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

    state = STATE_INIT;

    while(1)
    {
        PROCESS_YIELD();

        if(ev == PROCESS_EVENT_TIMER && data == &periodic_state_timer)
	{
            mqtt_state_machine();
        }
    }

    PROCESS_END();
}
