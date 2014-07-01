/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here


/*			CARTEL DE PRUEBA		*/

int main(void) {

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;

#include "lpc17xx_i2c.h"
#include "lpc17xx_pinsel.h"

int cont, estado, EstadoPuls, PULSADO, A, tickpuls;

void parpadear();
void pulsador();

void SysTick_Handler(void){ //equivale a la isr (mi isr)
		if(cont) cont --;
		tickpuls++;
}

int main(void) {
	SysTick_Config(SystemCoreClock / 1000);//cuenta cada 1 ms

	LPC_GPIO0->FIODIR |= 1<<22;/*ESTO HAY QUE PONERLO PARA DEFINIRLO COMO SALIDA!!!!!*/
	LPC_GPIO0->FIODIR &= ~(1<<18);//seteo pin 18 como entrada (no es necesario)(cuidado no poner todo en 0 sino el led puede quedar como entrada tambien)

	//buffers y variables auxiliares
		int i;
		//para I2C:
		uint8_t txbuf[] = {0,0,0};//la estructura de transmision de datos. los primeros 2 son la dir, parte alta y baja
		uint8_t rxbuf[] = {0};//para recibir necesito un solo byte. tiene que ser si o si un vector

		//primero, configuro pines para que funcionen como i2c
		PINSEL_CFG_Type pinConf;//defino una estructura del tipo pinsel_cfg

		pinConf.OpenDrain = PINSEL_PINMODE_NORMAL; //no open-drain
		pinConf.Pinmode = PINSEL_PINMODE_NORMAL; //pull-up resistor

		pinConf.Portnum = PINSEL_PORT_0; //port 0
		pinConf.Pinnum = PINSEL_PIN_19;	//pin 19

		pinConf.Funcnum = PINSEL_FUNC_3; //func 3: I2C (ver manual de usuario)

		PINSEL_ConfigPin(&pinConf);

		pinConf.Pinnum = PINSEL_PIN_20; //pin 20 (los otros datos quedan iguales)

		PINSEL_ConfigPin(&pinConf);

		//configuro i2c y lo activo
		I2C_Init(LPC_I2C1, 100000);
		I2C_Cmd(LPC_I2C1, ENABLE);

		//estructura de configuracion para i2c
		I2C_M_SETUP_Type i2cSetup;

		/* Leo byte (primero escribo dirección: 2 bytes) */
		i2cSetup.sl_addr7bit = 0x50;
		i2cSetup.tx_data = txbuf;
		i2cSetup.tx_length = 2;
		i2cSetup.retransmissions_max = 3;
		i2cSetup.rx_data = 0;
		i2cSetup.rx_length = 0;
		I2C_MasterTransferData(LPC_I2C1, &i2cSetup, I2C_TRANSFER_POLLING);

		//delay por software calculado con el teorema de los cinco dedos oscilantes
		for(i=0; i<0xFFFF; i++);

		/* Leo byte (lectura propiamente dicha: 1 byte) */
		i2cSetup.sl_addr7bit = 0x50;
		i2cSetup.tx_data = 0;
		i2cSetup.tx_length = 0;
		i2cSetup.retransmissions_max = 3;
		i2cSetup.rx_data = rxbuf;
		i2cSetup.rx_length = 1;
		I2C_MasterTransferData(LPC_I2C1, &i2cSetup, I2C_TRANSFER_POLLING);

		//dejo preparado la estructura para escribir lo que este en txbuf
		i2cSetup.tx_data = txbuf;
		i2cSetup.tx_length = 3;
		i2cSetup.retransmissions_max = 3;
		i2cSetup.rx_data = 0;
		i2cSetup.rx_length = 0;

	//INICIALIZACION
	PULSADO = 0;
	estado = rxbuf[0];//arranca en el ultimo estado que lo deje
	EstadoPuls = 0;
	tickpuls = 0;
	cont = A;
	switch(estado){
		case 0: A=125; break;
		case 1: A=250; break;
		case 2: A=500; break;
		case 3: A=1000; break;
	}

	while(1){
		parpadear();
		pulsador();
	}
}

void pulsador(void) {
		int puls = LPC_GPIO0->FIOPIN & 1<<18;
		switch(EstadoPuls){
			case 0: if(!puls){
						EstadoPuls=1;
						tickpuls=0;
					}
					break;
			case 1: if(tickpuls >= 20){
						if(puls)
							EstadoPuls=0;
						else
							EstadoPuls=2;
					}
					break;
			case 2: if(puls){
						EstadoPuls=3;
						tickpuls=0;
					}
					break;
			case 3: if(tickpuls>=20){
						if(puls) {
							PULSADO=1;
							EstadoPuls=0;
						}
						else EstadoPuls=2;
					}
					break;
		}
}

void parpadear(void){
	if(!cont){
			LPC_GPIO0->FIOPIN ^= 1<<22;
			cont = A;
		}
		if(PULSADO){
			PULSADO = 0;//dejo el flag de boton presionado de vuelta en cero
			switch (estado){
				case 0: estado=1; txbuf[2]=1; A = 125;
					break;
				case 1: estado=2; txbuf[2]=2; A = 250;
					break;
				case 2: estado=3; txbuf[2]=3; A = 500;
					break;
				case 3: estado=0; txbuf[2]=0; A = 1000;
					break;
			}
		/* Escribo byte*/
		I2C_MasterTransferData(LPC_I2C1, &i2cSetup, I2C_TRANSFER_POLLING);

		//delay por software calculado con el teorema de los cinco dedos oscilantes
		for(i=0; i<0xFFFF; i++);
		}
}
}
