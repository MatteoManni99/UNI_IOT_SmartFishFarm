#include <stdlib.h>
#include <string.h>

#include "dev/leds.h"
#include "coap-engine.h"

#include "sys/log.h"

#define LOG_MODULE "Heat Pump Resource"
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

//static void res_trigger(void);

EVENT_RESOURCE(res_heat_pump,
        "title=\"Heat Pump Handler\"",
        res_get_handler,
        NULL,
        res_put_handler,
        NULL,
        NULL
);

static int status = 0; // -1 ON COLD, 0 OFF, 1 ON HOT

// LEDS_RED : Heat Pump ON HOT
// LEDS_BLUE : Heat Pump ON COLD
// LEDS_GREEN : Heat Pump OFF

static void
res_get_handler(coap_message_t *request, coap_message_t *response,
	uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    int length = 0;
    const char* message = NULL;

    switch (status)
    {
    case 0:
        length = 3;
        message = "OFF";
        break;
    
    case 1:
        length = 6;
        message = "ON_HOT";
        break;
    
    case -1:
        length = 7;
        message = "ON_COLD";
        break;

    memcpy(buffer, message, length);
  
    coap_set_header_content_format(response, TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
    coap_set_header_etag(response, (uint8_t *)&length, 1);
    coap_set_payload(response, buffer, length);
}

static void res_put_handler(coap_message_t *request, coap_message_t *response,
	uint8_t *buffer, uint16_t preferred_size, int32_t*offset)
{
    size_t len = 0;
    const char *mode = NULL;
    //uint8_t led = 0;
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
        LOG_INFO("Mode %s\n", mode);

        // DISATTIVO la pompa
        if(strncmp(mode, "OFF", len) == 0)
        {
	    if (status != 0)
            {	
		leds_off(LEDS_ALL);
                leds_on(LEDS_GREEN);
                status = 0;
		success = 1;
            }
        }

        // ATTIVO la pompa mode HOT
        else if(strncmp(mode, "ON_HOT", len) == 0)
        {
            // Ma solo se era OFF
            if (status == 0)
            {	
		leds_off(LEDS_ALL);
                leds_on(LEDS_RED);
                status = 1;
		success = 1;
            }
        }

         // ATTIVO la pompa mode COLD
        else if(strncmp(mode, "ON_COLD", len) == 0)
        {
            // Ma solo se era OFF
            if (status == 0)
            {
		leds_off(LEDS_ALL);
                leds_on(LEDS_BLUE);
                status = -1;
		success = 1;
            }
	}
    }
    
    if(!success) {
        coap_set_status_code(response, BAD_REQUEST_4_00);
    }
}

