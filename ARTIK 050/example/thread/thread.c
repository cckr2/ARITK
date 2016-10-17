#include "wiced.h"

typedef struct
{
    wiced_bool_t quit;
    int num;
}my_thread_hadle_t;

void subthread_main(uint32_t arg);
wiced_thread_t      my_thread;
my_thread_hadle_t thread_handle;

void application_start(void)
{
    int i=0;
    wiced_result_t result;
    wiced_init( );

    thread_handle.quit = WICED_FALSE;
    thread_handle.num = 0;

    //thtead, priority, name, function, handler
    result = wiced_rtos_create_thread(&my_thread, WICED_APPLICATION_PRIORITY, "testThread", subthread_main, WICED_DEFAULT_APPLICATION_STACK_SIZE, NULL);

    for(i=0;i<1000;i++){
    	printf("Main thread\n");
    	thread_handle.num++;
    }

    printf("Kill Sub Thread\n");

    thread_handle.quit = WICED_TRUE;

    printf("Main Thread Die\n");

}

void subthread_main(uint32_t arg)
{
    while ( thread_handle.quit != WICED_TRUE )
    {
    	printf("Sub thread num : %d\n",thread_handle.num);
	}

    wiced_rtos_delay_milliseconds( 1000 );

    //Although Main Thread Die, Sub Thread Alive
    printf("Sub Thread Die\n");
	WICED_END_OF_CURRENT_THREAD( );
}
