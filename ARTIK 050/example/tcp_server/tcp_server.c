#include "wiced.h"

#define TCP_PACKET_MAX_DATA_LENGTH          (30)
#define TCP_SERVER_LISTEN_PORT              (50007) //PORT
#define TCP_SERVER_THREAD_PRIORITY          (WICED_DEFAULT_LIBRARY_PRIORITY)
/* Stack size should cater for printf calls */
#define TCP_SERVER_STACK_SIZE               (6200)
#define TCP_SERVER_COMMAND_MAX_SIZE         (10)
#define TCP_PACKET_MAX_DATA_LENGTH          (30)

/* Keepalive will be sent every 2 seconds */
#define TCP_SERVER_KEEP_ALIVE_INTERVAL      (2)
/* Retry 10 times */
#define TCP_SERVER_KEEP_ALIVE_PROBES        (5)
/* Initiate keepalive check after 5 seconds of silence on a tcp socket */
#define TCP_SERVER_KEEP_ALIVE_TIME          (5)
#define TCP_SILENCE_DELAY                   (30)

typedef struct
{
    wiced_bool_t quit;
    wiced_tcp_socket_t socket;
}tcp_server_handle_t;

static void tcp_server_thread_main(uint32_t arg);
static wiced_result_t tcp_server_process(  tcp_server_handle_t* server, wiced_packet_t* rx_packet );

static const wiced_ip_setting_t device_init_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS(192,168,  0,  1) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS(255,255,255,  0) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS(192,168,  0,  1) ),
};

static wiced_thread_t      tcp_thread;
static tcp_server_handle_t tcp_server_handle;

void application_start(void)
{
    wiced_interface_t interface;
    wiced_result_t result;

    /* Initialise the device and WICED framework */
    wiced_init( );

    /* Bring up the network interface */
    result = wiced_network_up_default( &interface, &device_init_ip_settings );

    while( result != WICED_SUCCESS )
    {
        printf("Bringing up network interface failed !\r\n");
        result = wiced_network_up_default( &interface, &device_init_ip_settings );
    }

    /* Create a TCP server socket */
    while (wiced_tcp_create_socket(&tcp_server_handle.socket, interface) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP socket creation failed\n"));
    }

    while (wiced_tcp_listen( &tcp_server_handle.socket, TCP_SERVER_LISTEN_PORT ) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP server socket initialization failed\n"));
    }

    /* Start a tcp server thread */
    WPRINT_APP_INFO(("Creating tcp server thread \n"));
    wiced_rtos_create_thread(&tcp_thread, TCP_SERVER_THREAD_PRIORITY, "Demo tcp server", tcp_server_thread_main, TCP_SERVER_STACK_SIZE, &tcp_server_handle);

}

static wiced_result_t tcp_server_process(  tcp_server_handle_t* server, wiced_packet_t* rx_packet )
{
    char*           request;
    uint16_t        request_length;
    uint16_t        available_data_length;
    wiced_packet_t* tx_packet;
    char*           tx_data;

    wiced_packet_get_data( rx_packet, 0, (uint8_t**) &request, &request_length, &available_data_length );

    /* Null terminate the received string */
    request[request_length] = '\x0';
    WPRINT_APP_INFO(("Received data: %s \n", request));

    /* Send echo back */
    if (wiced_packet_create_tcp(&server->socket, TCP_PACKET_MAX_DATA_LENGTH, &tx_packet, (uint8_t**)&tx_data, &available_data_length) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP packet creation failed\n"));
        return WICED_ERROR;
    }

    /* Write the message into tx_data"  */
    memcpy(tx_data, request, request_length);
    tx_data[request_length] = '\x0';

    /* Set the end of the data portion */
    wiced_packet_set_data_end(tx_packet, (uint8_t*)tx_data + request_length);

    /* Send the TCP packet */
    if (wiced_tcp_send_packet(&server->socket, tx_packet) != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("TCP packet send failed\n"));

        /* Delete packet, since the send failed */
        wiced_packet_delete(tx_packet);

        server->quit=WICED_TRUE;
        return WICED_ERROR;
    }
    WPRINT_APP_INFO(("Echo data: %s\n", tx_data));

    return WICED_SUCCESS;
}

static void tcp_server_thread_main(uint32_t arg)
{
    tcp_server_handle_t* server = (tcp_server_handle_t*) arg;

    while ( server->quit != WICED_TRUE )
    {
        wiced_packet_t* temp_packet = NULL;

        /* Wait for a connection */
        wiced_result_t result = wiced_tcp_accept( &server->socket );

#ifdef TCP_KEEPALIVE_ENABLED
        result = wiced_tcp_enable_keepalive(&server->socket, TCP_SERVER_KEEP_ALIVE_INTERVAL, TCP_SERVER_KEEP_ALIVE_PROBES, TCP_SERVER_KEEP_ALIVE_TIME );
        if( result != WICED_SUCCESS )
        {
            WPRINT_APP_INFO(("Keep alive initialization failed \n"));
        }
#endif /* TCP_KEEPALIVE_ENABLED */

        if ( result == WICED_SUCCESS )
        {
            /* Receive the query from the TCP client */
            if (wiced_tcp_receive( &server->socket, &temp_packet, WICED_WAIT_FOREVER ) == WICED_SUCCESS)
            {
                /* Process the client request */
                tcp_server_process( server, temp_packet );

                /* Delete the packet, we're done with it */
                wiced_packet_delete( temp_packet );

#ifdef TCP_KEEPALIVE_ENABLED
                WPRINT_APP_INFO(("Waiting for data on a socket\n"));
                /* Check keepalive: wait to see whether the keepalive protocol has commenced */
                /* This is achieved by waiting forever for a packet to be received on the TCP connection*/
                if (wiced_tcp_receive( &server->socket, &temp_packet, WICED_WAIT_FOREVER ) == WICED_SUCCESS)
                {
                    tcp_server_process( server, temp_packet );
                    /* Release the packet, we don't need it any more */
                    wiced_packet_delete( temp_packet );
                }
                else
                {
                    WPRINT_APP_INFO(("Connection has been dropped by networking stack\n\n"));
                }
#endif /* TCP_KEEPALIVE_ENABLED */

            }
            else
            {
                /* Send failed or connection has been lost, close the existing connection and */
                /* get ready to accept the next one */
                wiced_tcp_disconnect( &server->socket );
            }
        }
    }
    WPRINT_APP_INFO(("Disconnect\n"));

    wiced_tcp_disconnect( &server->socket );

    WICED_END_OF_CURRENT_THREAD( );
}

