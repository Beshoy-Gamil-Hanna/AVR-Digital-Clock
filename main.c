/*
 * main.c
 *
 *  Created on: Oct 10, 2026
 *      Author: Beshoy
 */

#include "APP/APP_interface.h"


int main(void)
{
    /* Initialize Application */

    APP_vidInit();


    /* Main Loop */

    while(1)
    {
        APP_vidTask();
    }

    return 0;
}
