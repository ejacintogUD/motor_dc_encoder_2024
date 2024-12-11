/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */


#include "DigitalIn.h"
#include "DigitalOut.h"
#include "InterruptIn.h"
#include "PinNames.h"
#include "PwmOut.h"
#include "Semaphore.h"
#include "SerialBase.h"
#include "ThisThread.h"
#include "Thread.h"
#include "cmsis_os2.h"
#include "mbed.h"

//definiciones 
#define TIEMPO_BLINK 500ms
#define RANURAS      400


//prototipos
void encoder_isr(void);
void func_interrupcion(void);
void func_periodico(void);


//variables 
static int pulsos =0; 
static float duty= 0.0; 


//hilos y elementos del sistema operativo  
Mutex puerto_serie;
Semaphore semaforo_interrupcion(1);
Thread Hilo_interrupcion(osPriorityNormal1,4096);
Thread hilo_periodico(osPriorityNormal,4096);


//pines y puertos 
DigitalOut led(LED1);
//InterruptIn   button(BUTTON1);

PwmOut enable_motor(D5);

InterruptIn EncoderA(D4);
DigitalIn   EncoderB(D2);



int main(void)
{

    EncoderA.enable_irq();
    EncoderA.fall(&encoder_isr);
    Hilo_interrupcion.start(func_interrupcion);
    hilo_periodico.start(func_periodico);
    //PWM 
    enable_motor.period(0.000833);//1,2 khz
    enable_motor.write(duty);

    puerto_serie.lock();
    printf("Arranque del programa\n\r");
    puerto_serie.unlock();
    while(true)
    {

        duty += 0.05;
        enable_motor.write(duty);
        if (duty==1.0) duty =0.0;
        ThisThread::sleep_for(TIEMPO_BLINK);
        
    }


}


void encoder_isr (void)
{
    semaforo_interrupcion.release();
    
}


void func_interrupcion(void)
{
    while(true)
    {
    semaforo_interrupcion.acquire();    
    pulsos++;
    }
}

void func_periodico (void)
{

    while(true)
    {
        printf("RPM %u \n\r", (pulsos/RANURAS)*60);
        pulsos=0;
        ThisThread::sleep_for(1s);
    }




}




