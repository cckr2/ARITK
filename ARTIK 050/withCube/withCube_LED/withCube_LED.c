#include "wiced.h"
#include "mqtt_api.h"
#include <string.h>
#include <math.h>

//---------------------mqtt---------------------
#define WICED_MQTT_DELAY_IN_MILLISECONDS     (50)
#define SERVER_IP		"52.86.204.150"	//Or 52.200.124.224 - api.artik.cloud
#define CLIENT_ID		"adasdssssadasd"
#define TOPIC 			"/v1.1/actions/d78cc7957094451fa75bad42cf21d9d7"

//#define SERVER_IP		"14.63.219.241"
//#define TOPIC 			"withcube_color"

static wiced_result_t mqtt_init();
static wiced_result_t connect();
static wiced_result_t subscribe(uint8_t *topic, int qos);
static wiced_result_t mqtt_connection_event_cb( wiced_mqtt_object_t mqtt_object, wiced_mqtt_event_info_t *event );

static wiced_mqtt_object_t mqtt_object;
static wiced_ip_address_t ip;
static wiced_mqtt_callback_t callbacks = mqtt_connection_event_cb;
static wiced_semaphore_t semaphore;
static wiced_mqtt_event_type_t expected_event;

//--------------------network--------------------
static const wiced_ip_setting_t device_init_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS(192,168,0,33) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS(255,255,255,  0) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS(192,168,  0,  1) ),
};

//--------------------RGB  --------------------
static void RGB_start();
static void RGB_change(int color_R, int color_G, int color_B);
wiced_pwm_t RGB_R_PWM =  WICED_PWM_1;
wiced_pwm_t RGB_G_PWM =  WICED_PWM_2;
wiced_pwm_t RGB_B_PWM =  WICED_PWM_3;

//---------------------------------------------


void application_start(void)
{
    wiced_interface_t interface;
    wiced_result_t result;

    /* Initialise the device and WICED framework */
    wiced_init( );

    /* Bring up the network interface */
    result = wiced_network_up_default( &interface, &device_init_ip_settings );
    while ( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("Bringing up network interface failed !\n") );
        wiced_rtos_delay_milliseconds( 100 );
        result = wiced_network_up_default( &interface, &device_init_ip_settings );
    }

    /* init mqtt */
    result = mqtt_init();
    while ( result != WICED_SUCCESS ){
       	WPRINT_APP_INFO( ("Init MQTT  failed !\n") );
       	result = mqtt_init();
    }

    /* connect mqtt server */
	result = connect();
    while ( result != WICED_SUCCESS ){
    	WPRINT_APP_INFO( ("Connect to MQTT Server failed !\n") );
    	result = connect();
    }

    wiced_rtos_delay_milliseconds(2000);

    /* start subscribe in mqtt*/
    subscribe(TOPIC, 0);

    /* start 3-color LED */
    RGB_start();

}

static void RGB_start(){
	wiced_pwm_init( RGB_R_PWM, 1000, 255 );
	wiced_pwm_init( RGB_G_PWM, 1000, 255 );
	wiced_pwm_init( RGB_B_PWM, 1000, 255 );

	wiced_pwm_start( RGB_R_PWM );
	wiced_pwm_start( RGB_G_PWM );
	wiced_pwm_start( RGB_B_PWM );
}


static void RGB_change(int color_R, int color_G, int color_B){
	color_R = color_R/2 +1;
	color_G = color_G/2 +1;
	color_B = color_B/2 +1;

	int i,j,k;
	for(i=0,j=0,k=0;;){
		if(i<color_R){
			i+=10;
		}else{
			i= color_R;
		}
		if(j<color_G){
			j+=10;
		}else{
			j= color_G;
		}
		if(k<color_B){
			k+=10;
		}else{
			k= color_B;
		}

		wiced_pwm_init( RGB_R_PWM, 1000, i );
		wiced_pwm_init( RGB_G_PWM, 1000, j );
		wiced_pwm_init( RGB_B_PWM, 1000, k );
		wiced_pwm_start( RGB_R_PWM );
		wiced_pwm_start( RGB_G_PWM );
		wiced_pwm_start( RGB_B_PWM );

		if((i==color_R)&&(j==color_G)&&(k==color_B)){
			break;
		}
	}
}

wiced_result_t mqtt_init()
{
    /* Memory allocated for mqtt object*/
    mqtt_object = (wiced_mqtt_object_t) malloc( WICED_MQTT_OBJECT_MEMORY_SIZE_REQUIREMENT );

    if ( mqtt_object == NULL )
     {
         WPRINT_APP_ERROR(("ERROE: Don't have memory to allocate for mqtt object...\n"));
         return WICED_ERROR;
     }

     wiced_mqtt_init( mqtt_object );
     str_to_ip(SERVER_IP, &ip);

     return WICED_SUCCESS;
}

wiced_result_t connect()
{

    wiced_mqtt_pkt_connect_t conninfo;
    memset( &conninfo, 0, sizeof( conninfo ) );

    conninfo.port_number = 8883;                   /* set to 0 indicates library to use default settings */
    conninfo.mqtt_version = WICED_MQTT_PROTOCOL_VER4;
    conninfo.clean_session = 1;
    conninfo.client_id = CLIENT_ID;
    conninfo.keep_alive = 10;
    conninfo.username = "d78cc7957094451fa75bad42cf21d9d7";
    conninfo.password = "9182a799c2ce4b9fa11a0c2d76fe7955";
    conninfo.peer_cn = NULL;

    if ( wiced_mqtt_connect( mqtt_object, &ip, WICED_STA_INTERFACE, callbacks, NULL, WICED_TRUE, &conninfo )!= WICED_SUCCESS)
    {
        WPRINT_APP_INFO(( "ERROR: Broker is not connected \n" ));
        return WICED_ERROR;
    }

     return WICED_SUCCESS;

}

wiced_result_t subscribe(uint8_t *topic, int qos)
{
    wiced_mqtt_msgid_t pktid;

   pktid = wiced_mqtt_subscribe( mqtt_object, topic, qos );
   if ( pktid == 0 )
   {
       WPRINT_APP_INFO(( "ERROR: Topic  is  not subscribed \n" ));
       return WICED_ERROR;
   }
   wiced_rtos_delay_milliseconds( WICED_MQTT_DELAY_IN_MILLISECONDS  );

    return WICED_SUCCESS;
}

wiced_result_t mqtt_connection_event_cb( wiced_mqtt_object_t mqtt_object, wiced_mqtt_event_info_t *event )
{
	char Received_msg[110];
	char *token = NULL;
	int duty_R, duty_G, duty_B,i,j,k;
	char divide1[] = ":";
	char divide2[] = ",:";
    switch ( event->type )
    {
        case WICED_MQTT_EVENT_TYPE_CONNECT_REQ_STATUS:
        case WICED_MQTT_EVENT_TYPE_DISCONNECTED:
        case WICED_MQTT_EVENT_TYPE_PUBLISHED:
        case WICED_MQTT_EVENT_TYPE_SUBCRIBED:
        case WICED_MQTT_EVENT_TYPE_UNSUBSCRIBED:
        {
            if (event->type == WICED_MQTT_EVENT_TYPE_PUBLISHED)
            {
                WPRINT_APP_INFO(( "MESSAGE [ID: %u] published\n\n",event->data.msgid) );
            }
            expected_event = event->type;
            wiced_rtos_set_semaphore( &semaphore );
        }
            break;
        case WICED_MQTT_EVENT_TYPE_PUBLISH_MSG_RECEIVED:
        {
        	wiced_mqtt_topic_msg_t msg = event->data.pub_recvd;
            WPRINT_APP_INFO(( "[MQTT] Received %.*s  for TOPIC : %.*s\n\n", (int) msg.data_len, msg.data, (int) msg.topic_len, msg.topic ));

            strcpy(Received_msg,(char*)msg.data);
            Received_msg[((int) msg.data_len)+1] = "\0"; // add end character

            token = strtok(Received_msg, divide1);
            token = strtok(NULL, divide1);
			token = strtok(NULL, divide1);
			token = strtok(NULL, divide1);
			token = strtok(NULL, divide1);

			token = strtok(token, "\"");
			token = strtok(token, divide2);
			duty_R = atoi(token);

			token = strtok(NULL, divide2);
			duty_G = atoi(token);

			token = strtok(NULL, divide2);
			duty_B = atoi(token);

//			printf("%d %d %d \n",duty_R, duty_G, duty_B);
			RGB_change(duty_R, duty_G, duty_B);
        }
            break;
        default:
            break;
    }
    return WICED_SUCCESS;
}
