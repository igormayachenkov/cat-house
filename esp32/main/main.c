#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/idf_additions.h"
#include "esp_log.h"


static const char *TAG = "first";

void taskOne( void *pvParameters )
{
    int coreID=13;
    
    for( ;; )
    {
        coreID = xPortGetCoreID();

        ESP_LOGI(TAG, "Task One, coreID=%i", coreID);
        //printf("MSG");
        vTaskDelay(250);
     }

    /* Tasks must not attempt to return from their implementing
       function or otherwise exit. In newer FreeRTOS port
       attempting to do so will result in an configASSERT() being
       called if it is defined. If it is necessary for a task to
       exit then have the task call vTaskDelete( NULL ) to ensure
       its exit is clean. */
    vTaskDelete( NULL );
}

void taskTwo( void *pvParameters )
{
    int coreID=13;
    
    for( ;; )
    {
        coreID = xPortGetCoreID();

        ESP_LOGI(TAG, "Task Two, coreID=%i", coreID);
        //printf("MSG");
        vTaskDelay(750);
     }

    /* Tasks must not attempt to return from their implementing
       function or otherwise exit. In newer FreeRTOS port
       attempting to do so will result in an configASSERT() being
       called if it is defined. If it is necessary for a task to
       exit then have the task call vTaskDelete( NULL ) to ensure
       its exit is clean. */
    vTaskDelete( NULL );
}

/* Function that creates a task. */

void vOtherFunction( void )

{

    BaseType_t xReturned;

    TaskHandle_t xHandle = NULL;


    /* Create the task, storing the handle. */

    xReturned = xTaskCreatePinnedToCore(
        taskOne,       /* Function that implements the task. */
        "One",          /* Text name for the task. */
        4096,      /* Stack size in words, not bytes. */
        ( void * ) 1,    /* Parameter passed into the task. */
        tskIDLE_PRIORITY,/* Priority at which the task is created. */
        &xHandle,
        0 //core ID
    );      /* Used to pass out the created task's handle. */


    if( xReturned == pdPASS ){
        ESP_LOGI(TAG, "Task created");

        /* The task was created. Use the task's handle to delete the task. */

        //vTaskDelete( xHandle );

    }else{
        ESP_LOGI(TAG, "Task NOT created");

    }

}


void app_main(void)
{
    ESP_LOGI(TAG, "Example started");

    // Create tasks
     BaseType_t xReturned;

    vOtherFunction();


//     TaskHandle_t xHandle = NULL;


//     /* Create the task, storing the handle. */

     //xReturned = xTaskCreate(
    xReturned =  xTaskCreatePinnedToCore(
        taskTwo,       /* Function that implements the task. */
        "Two",          /* Text name for the task. */
        4096,      /* Stack size in words, not bytes. */
        //( void * ) 1,    /* Parameter passed into the task. */
        NULL,
        tskIDLE_PRIORITY,/* Priority at which the task is created. */
        //&xHandle,       /* Used to pass out the created task's handle. */
        NULL,       /* Used to pass out the created task's handle. */
        1 // Core ID
    );

    if( xReturned == pdPASS ){
         ESP_LOGI(TAG, "Task Two created");
     }else{
         ESP_LOGI(TAG, "Task Two NOT created");
     }



    // Start the real time scheduler.
    //vTaskStartScheduler(); ALREADY STARTED


    int coreID = 13;

    while(1){
        coreID = xPortGetCoreID();
        ESP_LOGI(TAG, "main loop, coreID=%i",coreID);
        vTaskDelay(1000.0/portTICK_PERIOD_MS);
        vTaskSuspend( NULL );
    }

}