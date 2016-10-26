#include "wiced.h"
#include "mqtt_api.h"
#include <math.h>
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define CUBE_Alphabet		"a"
#define CUBE_Melody			"m"
#define CUBE_NUM		6

//--------------------lis3dh--------------------
#define LIS3DH_CTRL_REG1 0x20
#define LIS3DH_CTRL_REG2 0x21
#define LIS3DH_CTRL_REG3 0x22
#define LIS3DH_CTRL_REG4 0x23
#define LIS3DH_CTRL_REG5 0x24
#define LIS3DH_CTRL_REG6 0x25
#define LIS3DH_STATUS_REG 0x27
#define LIS3DH_OUT_X_L 0x28
#define LIS3DH_OUT_X_H 0x29
#define LIS3DH_OUT_Y_L 0x2A
#define LIS3DH_OUT_Y_H 0x2B
#define LIS3DH_OUT_Z_L 0x2C
#define LIS3DH_OUT_Z_H 0x2D
static void lis3dh_init();
static void lis3dh_run();

static wiced_spi_device_t lis3dh_spi;
static wiced_spi_message_segment_t lis3dh_msg;
static uint8_t tx_buffer[2];
static uint8_t rx_buffer[2];
static int outHigh[3],outLow[3];

static void write_register (uint8_t reg, uint8_t value);
static uint8_t read_register ( uint16_t reg );

static char msg[] =  "{\"direction\":\"%s,%d,%s,%d\"}";
static char message[40];

//---------------------mqtt---------------------
#define WICED_MQTT_DELAY_IN_MILLISECONDS     (200)
#define SERVER_IP		"52.86.204.150"	//Or 52.200.124.224 - api.artik.cloud
#define CLIENT_ID		"01Melody01"
#define TOPIC 			"/v1.1/messages/ad4986cfd73740a2993059c25932b928"



static wiced_result_t mqtt_init();
static wiced_result_t connect();
static wiced_result_t publish(uint8_t *topic, uint8_t *data, uint32_t data_len, uint8_t qos);
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
        wiced_rtos_delay_milliseconds( 100 );
       	result = mqtt_init();
    }

    printf("here");
    /* connect mqtt server */
	result = connect();
    while ( result != WICED_SUCCESS ){
    	WPRINT_APP_INFO( ("Connect to MQTT Server failed !\n") );
    	wiced_rtos_delay_milliseconds( 100 );
    	result = connect();
    }

    /* init mpp3208 */
    lis3dh_init();

    /* run mpp3208 */
	lis3dh_run();

}


static void lis3dh_init(){
	lis3dh_spi.port = WICED_SPI_1;
	lis3dh_spi.chip_select = WICED_GPIO_22;
	lis3dh_spi.speed = 100000; //4Mhz
	lis3dh_spi.mode = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_NO_DMA | SPI_MSB_FIRST);
	lis3dh_spi.bits = 16;

	wiced_spi_init(&lis3dh_spi);

	write_register(LIS3DH_CTRL_REG1, 0x57); //All axes, normal, 100Hz
	write_register(LIS3DH_CTRL_REG2, 0x00); // No HighPass filter
	write_register(LIS3DH_CTRL_REG3, 0x00); // No interrupts
	write_register(LIS3DH_CTRL_REG4, 0x0); // all defaults
	write_register(LIS3DH_CTRL_REG5, 0x0); // all defaults
	write_register(LIS3DH_CTRL_REG6, 0x0); // all defaults

	outLow[0] = LIS3DH_OUT_X_L;
	outHigh[0] = LIS3DH_OUT_X_H;
	outLow[1] = LIS3DH_OUT_Y_L;
	outHigh[1] = LIS3DH_OUT_Y_H;
	outLow[2] = LIS3DH_OUT_Z_L;
	outHigh[2] = LIS3DH_OUT_Z_H;
}

static void write_register (uint8_t reg, uint8_t value)
{
	memset(tx_buffer,0,sizeof(tx_buffer));
	tx_buffer[0] = reg & ~0xc0;		//reg & 0011 1111
	tx_buffer[1] = value;

	memset(&lis3dh_msg,0,sizeof(lis3dh_msg));
	lis3dh_msg.tx_buffer = tx_buffer;
	lis3dh_msg.rx_buffer = rx_buffer;
	lis3dh_msg.length = sizeof(tx_buffer);

	wiced_spi_transfer(&lis3dh_spi, &lis3dh_msg,1);
	wiced_rtos_delay_milliseconds( 1 );

}

static uint8_t read_register ( uint16_t reg )
{
	memset(tx_buffer,0,sizeof(tx_buffer));
	memset(rx_buffer,0,sizeof(rx_buffer));
	tx_buffer[0] = reg | 0x80;  //0x80 is first bit and read bit, second bit is multiple reading set bit
	tx_buffer[1] = 0;

	memset(&lis3dh_msg,0,sizeof(lis3dh_msg));
	lis3dh_msg.tx_buffer = tx_buffer;
	lis3dh_msg.rx_buffer = rx_buffer;
	lis3dh_msg.length = sizeof(tx_buffer);

	wiced_spi_transfer(&lis3dh_spi, &lis3dh_msg,1);
	wiced_rtos_delay_microseconds( 10 );

	return rx_buffer[1];
}


static void lis3dh_run(){
	int i,j,k;
	int sum,shock,subshock;
	int readValue[3],maxValue[3],max_dir,square[3];
	int dir, current_dir,real_dir,prev_dir,dir_count, temp_dir;
	int16_t temp;

	while(1){
		current_dir = 0;
		shock = 0;
		subshock = 0;
		for(j=0;j<10;j++){
			while(read_register(LIS3DH_STATUS_REG) & 0x8 == 0 ){};
			sum = 0;

			for(i=0;i<3;i++){
				temp = ((read_register(outHigh[i]) << 8 ) | read_register(outLow[i]));
				readValue[i] = (int) (temp/160.0);
				square[i] = readValue[i] * readValue[i];

				if(current_dir < square[i]){
					temp_dir = i;
					dir = temp_dir;
					if(readValue[i]<0){
						dir += 3;
					}
					current_dir = square[i];
				}
				readValue[i] = abs(readValue[i]);
				sum+=readValue[i];
			}

			if(shock<sum){
				shock = sum;
				max_dir = dir;
				maxValue[0] = readValue[0];
				maxValue[1] = readValue[1];
				maxValue[2] = readValue[2];
				for(k=0;k<3;k++){
					if(k!=temp_dir)
						subshock+=maxValue[k];
				}
			}
			wiced_rtos_delay_milliseconds( 10 );
		}

		//printf("a %d b %d\n",shock,subshock);
		if(shock>175){
			if(subshock>180){
				memset(message,0,sizeof(message));
				switch(max_dir){
				case 0:sprintf(message, msg, CUBE_Melody, CUBE_NUM, "+z", shock);break;
				case 1:sprintf(message, msg, CUBE_Melody, CUBE_NUM, "+y", shock);break;
				case 2:sprintf(message, msg, CUBE_Melody, CUBE_NUM, "+x", shock);break;
				case 3:sprintf(message, msg, CUBE_Melody, CUBE_NUM, "-z", shock);break;
				case 4:sprintf(message, msg, CUBE_Melody, CUBE_NUM, "-y", shock);break;
				case 5:sprintf(message, msg, CUBE_Melody, CUBE_NUM, "-x", shock);break;
				}
				wiced_rtos_delay_milliseconds( 10 );

				publish(TOPIC,message, sizeof(message),0);
				wiced_rtos_delay_milliseconds( 200 );

			}
		}

		/*-check rotation-*/
		if(real_dir != max_dir){
			if(prev_dir != max_dir){
				prev_dir = max_dir;
				dir_count=0;
			}else{
				dir_count++;
			}
		}
		else{
			dir_count++;
		}

		if(dir_count==5){
			real_dir = temp_dir;
			switch(max_dir){
			case 0:sprintf(message, msg, CUBE_Alphabet, CUBE_NUM, "+z",0);break;
			case 1:sprintf(message, msg, CUBE_Alphabet, CUBE_NUM, "+y",0);break;
			case 2:sprintf(message, msg, CUBE_Alphabet, CUBE_NUM, "+x",0);break;
			case 3:sprintf(message, msg, CUBE_Alphabet, CUBE_NUM, "-z",0);break;
			case 4:sprintf(message, msg, CUBE_Alphabet, CUBE_NUM, "-y",0);break;
			case 5:sprintf(message, msg, CUBE_Alphabet, CUBE_NUM, "-x",0);break;
			}

			wiced_rtos_delay_milliseconds( 50 );
			//printf("%s \n",message);
			publish(TOPIC,message, sizeof(message),0);
			wiced_rtos_delay_milliseconds( 500 );
		}
		wiced_rtos_delay_milliseconds( 10 );
	}
}

static wiced_result_t mqtt_init()
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

static wiced_result_t connect()
{

    wiced_mqtt_pkt_connect_t conninfo;

    memset( &conninfo, 0, sizeof( conninfo ) );

    conninfo.port_number = 8883;                   /* set to 0 indicates library to use default settings */
    conninfo.mqtt_version = WICED_MQTT_PROTOCOL_VER4;
    conninfo.clean_session = 1;
    conninfo.client_id = CLIENT_ID;
    conninfo.keep_alive = 10;
    conninfo.username = "ad4986cfd73740a2993059c25932b928";
    conninfo.password = "df6a5140e6a14632948ab7ccd78761d5";
    conninfo.peer_cn = NULL;

    if ( wiced_mqtt_connect( mqtt_object, &ip, WICED_STA_INTERFACE, callbacks,NULL, WICED_TRUE ,&conninfo )!= WICED_SUCCESS)
    {
        WPRINT_APP_INFO(( "ERROR: Broker is not connected \n" ));
        return WICED_ERROR;
    }

     return WICED_SUCCESS;

}

static wiced_result_t publish(uint8_t *topic, uint8_t *data, uint32_t data_len, uint8_t qos)
{
    wiced_mqtt_msgid_t pktid;
    pktid = wiced_mqtt_publish ( mqtt_object, topic, data, data_len, qos);
    if(  pktid == 0 )
    {
       WPRINT_APP_INFO(( "ERROR! Message is not published\n" ));
       return WICED_ERROR;
    }

    wiced_rtos_delay_milliseconds( WICED_MQTT_DELAY_IN_MILLISECONDS  );
    return WICED_SUCCESS;
}

static wiced_result_t mqtt_connection_event_cb( wiced_mqtt_object_t mqtt_object, wiced_mqtt_event_info_t *event )
{

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
        }
            break;
        default:
            break;
    }
    return WICED_SUCCESS;
}
