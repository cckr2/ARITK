#include "wiced.h"
#define RX_BUFFER_SIZE    64
#define TEST_STR          "\r\nType something! Keystrokes are echoed to the terminal ...\r\n> "

wiced_uart_config_t uart_config =
{
    .baud_rate    = 115200,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
};

wiced_uart_config_t uart_config2 =
{
    .baud_rate    = 115200,
    .data_width   = DATA_WIDTH_8BIT,
    .parity       = NO_PARITY,
    .stop_bits    = STOP_BITS_1,
    .flow_control = FLOW_CONTROL_DISABLED,
};



wiced_ring_buffer_t rx_buffer,rx_buffer2;
uint8_t             rx_data[RX_BUFFER_SIZE],rx_data2[RX_BUFFER_SIZE];

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    char c;
    uint32_t expected_data_size = 1;
    /* Initialise ring buffer */
    ring_buffer_init(&rx_buffer, rx_data, RX_BUFFER_SIZE );
    ring_buffer_init(&rx_buffer2, rx_data2, RX_BUFFER_SIZE );

    /* Initialise UART. A ring buffer is used to hold received characters */
    wiced_uart_init( STDIO_UART, &uart_config, &rx_buffer );

    wiced_uart_init( WICED_UART_2, &uart_config2, &rx_buffer2);

    /* Send a test string to the terminal */
    wiced_uart_transmit_bytes( STDIO_UART, TEST_STR, sizeof( TEST_STR ) - 1 );

    /* Wait for user input. If rsseceived, echo it back to the terminal */
    while ( wiced_uart_receive_bytes( STDIO_UART, &c, &expected_data_size, WICED_NEVER_TIMEOUT ) == WICED_SUCCESS )
    {
        wiced_uart_transmit_bytes( WICED_UART_2, &c, 1 );
        expected_data_size = 1;
    }
}
