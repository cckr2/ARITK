#include "wiced.h"

#define TCP_PACKET_MAX_DATA_LENGTH        30
#define TCP_CLIENT_INTERVAL               2
#define TCP_SERVER_PORT                   50007
#define TCP_CLIENT_CONNECT_TIMEOUT        500
#define TCP_CLIENT_RECEIVE_TIMEOUT        300
#define TCP_CONNECTION_NUMBER_OF_RETRIES  3

/* Change the server IP address to match the TCP echo server address */
#define TCP_SERVER_IP_ADDRESS MAKE_IPV4_ADDRESS(192,168,219,137)


static wiced_result_t tcp_communication();

static const wiced_ip_setting_t device_init_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS(192,168,  0,  1) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS(255,255,255,  0) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS(192,168,  0,  1) ),
};

static wiced_tcp_socket_t  tcp_client_socket;
static wiced_timed_event_t tcp_client_event;

void application_start(void)
{
    wiced_interface_t interface;
    wiced_result_t result;
    int	connection_retries;

    /* Initialise the device and WICED framework */
    wiced_init( );

    /* Bring up the network interface */
    result = wiced_network_up_default( &interface, &device_init_ip_settings );

    while ( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("Bringing up network interface failed !\n") );
        result = wiced_network_up_default( &interface, &device_init_ip_settings );
    }

    /* Create a TCP socket */
    while ( wiced_tcp_create_socket( &tcp_client_socket, interface ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("TCP socket creation failed\n") );
    }

    /* Bind to the socket */
    while (wiced_tcp_bind( &tcp_client_socket, TCP_SERVER_PORT )!= WICED_SUCCESS ){

    };

    /* Connect to the remote TCP server, try several times */
    const wiced_ip_address_t INITIALISER_IPV4_ADDRESS( server_ip_address, TCP_SERVER_IP_ADDRESS );
    connection_retries = 0;
    do
    {
        result = wiced_tcp_connect( &tcp_client_socket, &server_ip_address, TCP_SERVER_PORT, TCP_CLIENT_CONNECT_TIMEOUT );
        connection_retries++;
    }
    while( ( result != WICED_SUCCESS ) && ( connection_retries < TCP_CONNECTION_NUMBER_OF_RETRIES ) );

    /* data transfer */
    tcp_communication( );


    /* Close the connection */
    wiced_tcp_disconnect(&tcp_client_socket);

}


wiced_result_t tcp_communication( )
{
    wiced_result_t           result;
    wiced_packet_t*          packet;
    wiced_packet_t*          rx_packet;
    char*                    tx_data;
    char*                    rx_data;
    uint16_t                 rx_data_length;
    uint16_t                 available_data_length;

    /* Create the TCP packet. Memory for the tx_data is automatically allocated */
    if (wiced_packet_create_tcp(&tcp_client_socket, TCP_PACKET_MAX_DATA_LENGTH, &packet, (uint8_t**)&tx_data, &available_data_length) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP packet creation failed\n"));
        return WICED_ERROR;
    }

    /* Write the message into tx_data"  */
    sprintf(tx_data, "%s", "ping\n");

    /* Set the end of the data portion */
    wiced_packet_set_data_end(packet, (uint8_t*)tx_data + strlen(tx_data));

    /* Send the TCP packet */
    if (wiced_tcp_send_packet(&tcp_client_socket, packet) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP packet send failed\n"));

        /* Delete packet, since the send failed */
        wiced_packet_delete(packet);

        return WICED_ERROR;
    }

    /* Receive a response from the server and print it out to the serial console */
    result = wiced_tcp_receive(&tcp_client_socket, &rx_packet, TCP_CLIENT_RECEIVE_TIMEOUT);
    if( result != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("TCP packet reception failed\n"));

        /* Delete packet, since the receive failed */
        wiced_packet_delete(rx_packet);

        return WICED_ERROR;
    }

    /* Get the contents of the received packet */
    wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);

    /* Null terminate the received string */
    rx_data[rx_data_length] = '\x0';
    printf("%s\n",rx_data);

    wiced_packet_delete(packet);
    wiced_packet_delete(rx_packet);

    return WICED_SUCCESS;
}



