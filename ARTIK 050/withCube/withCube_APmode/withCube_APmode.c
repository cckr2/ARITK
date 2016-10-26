#include "wiced.h"
#include "resources.h"

#define MAX_SOFT_AP_CLIENTS  (7)
#define TCP_PACKET_MAX_DATA_LENGTH        30
#define INTERVAL               500
#define TCP_SERVER_PORT                   50007
#define TCP_CLIENT_CONNECT_TIMEOUT        500
#define TCP_CLIENT_RECEIVE_TIMEOUT        300
#define TCP_CONNECTION_NUMBER_OF_RETRIES  3
#define TCP_SERVER_IP_ADDRESS MAKE_IPV4_ADDRESS(192,168,0,77)

#define RANGE 200
#define THRESEHOLD_X 2835
#define THRESEHOLD_Y 2215
#define THRESEHOLD_Z 2000

typedef struct
{
    int         count;
    wiced_mac_t mac_list[MAX_SOFT_AP_CLIENTS];
} client_info_t;

static const wiced_ip_setting_t ap_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS( 255,255,255,  0 ) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
};

static wiced_result_t tcp_client();
static void mcp3208_init();
static void mcp3208_run(uint32_t arg);

static wiced_tcp_socket_t  tcp_client_socket;
static wiced_timed_event_t tcp_client_event, mcp3208_event;
static wiced_spi_device_t mcp3208_spi;
static wiced_spi_message_segment_t mcp3208_msg[3];
static int low_x,low_y,low_z,high_x,high_y,high_z;
static uint8_t txb[3][3];
static uint8_t rxb[3][3];
static int tcp_state=0, flag = 0,count =1;

void application_start(void)
{
	wiced_result_t  result;
    client_info_t   client_info;
    client_info.count = 0;

    /* Initialise Wiced system */
    wiced_rtos_delay_milliseconds( 500 );
    wiced_init();
    wiced_rtos_delay_milliseconds( 500 );

    /* Bring up the softAP interface  */
    result = wiced_network_up( WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &ap_ip_settings );
    while ( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("Bringing up network interface failed !\n") );
        wiced_network_up( WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &ap_ip_settings );
    }

    while ( client_info.count < MAX_SOFT_AP_CLIENTS)
    {
        client_info.count = MAX_SOFT_AP_CLIENTS;

        /* Get the list of the stations connected to Wiced soft AP */
        result = wiced_wifi_get_associated_client_list( &client_info, sizeof( client_info ) );

        WPRINT_APP_INFO( ("Clients connected %d..\r\n", client_info.count) );
        wiced_rtos_delay_milliseconds( 500 );
    }

    wiced_rtos_delay_milliseconds( 5000 );
    WPRINT_APP_INFO( ("accept End\r\n\r\n") );

    /* Create a TCP socket */
    if ( wiced_tcp_create_socket( &tcp_client_socket, WICED_AP_INTERFACE ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("TCP socket creation failed\n") );
    }

    /* Bind to the socket */
    wiced_tcp_bind( &tcp_client_socket, TCP_SERVER_PORT );

    /* init mpp3208 */
    mcp3208_init();

    /* Register a function to send TCP packets */
    wiced_rtos_register_timed_event( &tcp_client_event, WICED_NETWORKING_WORKER_THREAD, &tcp_client, INTERVAL * MILLISECONDS, 0 );


}
void mcp3208_init(){
	int i;

    low_x = THRESEHOLD_X-RANGE;
    low_y = THRESEHOLD_Y-RANGE;
    low_z = THRESEHOLD_Z-RANGE;
    high_x = THRESEHOLD_X+RANGE;
    high_y = THRESEHOLD_Y+RANGE;
    high_z = THRESEHOLD_Z+RANGE;

	for(i=0;i<3;i++){
		txb[i][0] = (0x06 | ((i & 0x07) >> 2));
		txb[i][1] = ((i & 0x07) << 6);
		txb[i][2] = 0x0;
	}

	mcp3208_spi.port = WICED_SPI_1;
	mcp3208_spi.chip_select = WICED_GPIO_22;
	mcp3208_spi.speed = 1000000;
	mcp3208_spi.mode = (SPI_CLOCK_RISING_EDGE | SPI_CLOCK_IDLE_HIGH | SPI_NO_DMA | SPI_MSB_FIRST);
	mcp3208_spi.bits = 8;
	wiced_spi_init(&mcp3208_spi);

}


void mcp3208_run(uint32_t arg){
	int i,j;
	uint16_t readValue[3];

	for(j=0;j<3000;j++){
		for(i=0;i<3;i++){
			memset(&mcp3208_msg[i],0,sizeof(mcp3208_msg[i]));
			mcp3208_msg[i].tx_buffer = txb[i];
			mcp3208_msg[i].rx_buffer = rxb[i];
			mcp3208_msg[i].length = sizeof(txb[i]);
			wiced_spi_transfer(&mcp3208_spi, &mcp3208_msg[i],1);
			readValue[i] = (((rxb[i][1] & 0x0F) << 8) | rxb[i][2]);
			wiced_rtos_delay_milliseconds( 10 );
		}

		//printf("X : %d /Y : %d /z : %d \n", readValue[0],readValue[1],readValue[2]);
		if( (low_x<readValue[0]) && (readValue[0]<high_x) && (low_y<readValue[1]) && (readValue[1]<high_y) && (low_z<readValue[2]) && (readValue[2]<high_z)){
			//printf("X : %d /Y : %d /z : %d \n", readValue[0],readValue[1],readValue[2]);
		}else{
			printf("X %d < %d < %d /Y : %d < %d < %d  /z : %d < %d < %d  \n",low_x ,readValue[0],high_x,low_y,readValue[1],high_y,low_z,readValue[2],high_z);
			flag = 1;
			break;
		}
	}

}

void tcp_init(){
    wiced_result_t           result;
	int                      connection_retries;
	connection_retries = 0;
    const wiced_ip_address_t INITIALISER_IPV4_ADDRESS( server_ip_address, TCP_SERVER_IP_ADDRESS );

	do
	{
		result = wiced_tcp_connect( &tcp_client_socket, &server_ip_address, TCP_SERVER_PORT, TCP_CLIENT_CONNECT_TIMEOUT );
		connection_retries++;
	}
	while( ( result != WICED_SUCCESS ) && ( connection_retries < TCP_CONNECTION_NUMBER_OF_RETRIES ) );
	if( result != WICED_SUCCESS)
	{
		WPRINT_APP_INFO(("Unable to connect to the server! Halt.\n"));
	}else{
		WPRINT_APP_INFO(("Connect to the server! \n"));
		tcp_state=1;
		wiced_rtos_register_timed_event( &mcp3208_event, WICED_NETWORKING_WORKER_THREAD, &mcp3208_run, INTERVAL * MILLISECONDS, 0 );
	}
}


wiced_result_t tcp_client( void* arg )
{
    wiced_packet_t*          packet;
    char*                    tx_data;

    uint16_t                 available_data_length;
    WPRINT_APP_INFO(("here\n"));
    UNUSED_PARAMETER( arg );

    if(!tcp_state){
    	tcp_init();
    }
	if(flag && tcp_state){
		flag=0;
		if (wiced_packet_create_tcp(&tcp_client_socket, TCP_PACKET_MAX_DATA_LENGTH, &packet, (uint8_t**)&tx_data, &available_data_length) != WICED_SUCCESS)
		{
			WPRINT_APP_INFO(("TCP packet creation failed\n"));
			return WICED_ERROR;
		}

		//Random Data save in Packet
		sprintf(tx_data, "%d\n", count);
		count++;
		/* Set the end of the data portion */
		wiced_packet_set_data_end(packet, (uint8_t*)tx_data + strlen(tx_data));

		/* Send the TCP packet */
		if (wiced_tcp_send_packet(&tcp_client_socket, packet) != WICED_SUCCESS)
		{
			WPRINT_APP_INFO(("TCP packet send failed\n"));
			wiced_tcp_disconnect(&tcp_client_socket);


			return WICED_ERROR;
		}
		wiced_packet_delete(packet);
	}

    return WICED_SUCCESS;

}



