#include <stdlib.h>
#include <string.h>

#include "dev/leds.h"
#include "coap-engine.h"

#include "sys/log.h"

#define LOG_MODULE "Water Filter Resource"
#define LOG_LEVEL LOG_LEVEL_INFO

static void res_get_handler(
    coap_message_t *request,
    coap_message_t *response,
    uint8_t *buffer,
    uint16_t preferred_size, int32_t *offset
);

static void res_put_handler(
    coap_message_t *request,
    coap_message_t *response,
    uint8_t *buffer,
    uint16_t preferred_size, int32_t *offset
);

static void res_trigger(void);

EVENT_RESOURCE(res_water_filter,
        "title=\"Water Filter Handler\"",
        res_get_handler,
        NULL,
        res_put_handler,
        NULL,
        res_trigger
);

static bool switched_on = false;

// LEDS_RED : Water Filter OFF
// LEDS_BLUE : Water Filter ON

static void
res_get_handler(coap_message_t *request, coap_message_t *response,
	uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    int length = 0;
    const char* message = NULL;

    if(!switched_on)
    {
    	length = 3;
        message = "OFF";
    }
    else
    {
	length = 2;
        message = "ON";
    }

    memcpy(buffer, message, length);

    coap_set_header_content_format(response, TEXT_PLAIN);
    coap_set_header_etag(response, (uint8_t *)&length, 1);
    coap_set_payload(response, buffer, length);
}

static void res_put_handler(coap_message_t *request, coap_message_t *response,
	uint8_t *buffer, uint16_t preferred_size, int32_t*offset)
{
    size_t len = 0;
    const char *mode = NULL;
    int success = 0;

	//start debug
	const uint8_t *chunk;
	coap_get_payload(request, &chunk);
	printf("coap_get_payload: %s\n", chunk);
	
	len = coap_get_post_variable(request, "mode", &mode);
	printf("coap_get_query_variable: %s\n", mode);
	printf("coap_get_query_variable: %zu\n", len);

	len = coap_get_query_variable(request, "mode", &mode);
	printf("coap_get_post_variable: %s\n", mode);
	printf("coap_get_query_variable: %zu\n", len);
	//fine debug

    if((len = coap_get_post_variable(request, "mode", &mode)))
    {
        LOG_INFO("mode %s\n", mode);

        // DISATTIVO il filtro
        if(strncmp(mode, "OFF", len) == 0)
        {
	    // Ma solo se era ON
	    if (switched_on)
            {	
		leds_off(LEDS_ALL);
		leds_on(LEDS_RED);
                switched_on = false;
		success = 1;
            }
        }

        // ATTIVO il filtro
        else if(strncmp(mode, "ON", len) == 0)
        {
            // Ma solo se era OFF
            if (!switched_on)
            {	
		leds_off(LEDS_ALL);
		leds_on(LEDS_BLUE);
                switched_on = true;
		success = 1;
            }
        }
    }
    
    if(!success) {
        coap_set_status_code(response, BAD_REQUEST_4_00);
    }
}

// Serve per switchare da on ad off
static void res_trigger(){
    if (switched_on)
    {
	switched_on = false;
	leds_off(LEDS_ALL);
	leds_on(LEDS_RED);
    }
    else
    {
	switched_on = true;
	leds_off(LEDS_ALL);
	leds_on(LEDS_BLUE);
    }
}


