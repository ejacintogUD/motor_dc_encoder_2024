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
#define TIEMPO_BLINK 1s
#define RANURAS      400


//prototipos
void encoder_isr(void);
void func_interrupcion(void);
void func_periodico(void);
void pulling(void);

//variables 
static int pulsos =0; 
static float duty= 0.97; 
static int duty_int = 0;
static bool bandera_sentido = false;


//hilos y elementos del sistema operativo  
Mutex puerto_serie;
Semaphore semaforo_interrupcion(1);
Thread Hilo_interrupcion(osPriorityNormal1,4096);
Thread hilo_periodico(osPriorityNormal,4096);
Thread hilo_pulling(osPriorityNormal, 2048);


//pines y puertos 
DigitalOut led(LED1);
BusOut sentido_motor(PB_13, PB_14);
DigitalIn button(BUTTON1);

PwmOut enable_motor(D5);

InterruptIn EncoderA(D4);
DigitalIn   EncoderB(D7);



int main(void)
{

    EncoderA.enable_irq();
    EncoderA.rise(&encoder_isr);
    Hilo_interrupcion.start(func_interrupcion);
    hilo_periodico.start(func_periodico);
    hilo_pulling.start(pulling);
    //PWM 
    enable_motor.period(0.000833);//1,2 khz
    enable_motor.write(duty);

    puerto_serie.lock();
    printf("Arranque del programa\n\r");
    puerto_serie.unlock();
    while(true)
    {

        duty -= 0.02;
        enable_motor.write(duty);
        if (duty<= 0.47) duty =0.97;
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
    if (EncoderB) bandera_sentido = true;  // voy para la derecha
    else          bandera_sentido = false;
    }
}

void func_periodico (void)
{
    while(true)
    {
        printf("RPM: %04u,", (pulsos/RANURAS)*60);
        duty_int = int(duty*100);
        printf ("Duty: %02u \t", duty_int);
        pulsos=0;
        if (bandera_sentido) printf("Izq \n\r");
        else                 printf("Der \n\r");
        ThisThread::sleep_for(1s);
    }
}


void pulling(void)
{
    while(true)
    {
        if (button) sentido_motor = 0x1;
        else        sentido_motor = 0x2;
        ThisThread::sleep_for(100ms);
    }
}

