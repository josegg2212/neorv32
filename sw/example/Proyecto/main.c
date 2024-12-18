// #################################################################################################
// # << NEORV32 - Blinking LED Demo Program >>                                                     #
// # ********************************************************************************************* #
// # BSD 3-Clause License                                                                          #
// #                                                                                               #
// # Copyright (c) 2021, Stephan Nolting. All rights reserved.                                     #
// #                                                                                               #
// # Redistribution and use in source and binary forms, with or without modification, are          #
// # permitted provided that the following conditions are met:                                     #
// #                                                                                               #
// # 1. Redistributions of source code must retain the above copyright notice, this list of        #
// #    conditions and the following disclaimer.                                                   #
// #                                                                                               #
// # 2. Redistributions in binary form must reproduce the above copyright notice, this list of     #
// #    conditions and the following disclaimer in the documentation and/or other materials        #
// #    provided with the distribution.                                                            #
// #                                                                                               #
// # 3. Neither the name of the copyright holder nor the names of its contributors may be used to  #
// #    endorse or promote products derived from this software without specific prior written      #
// #    permission.                                                                                #
// #                                                                                               #
// # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   #
// # OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               #
// # MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    #
// # COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     #
// # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE #
// # GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    #
// # AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     #
// # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  #
// # OF THE POSSIBILITY OF SUCH DAMAGE.                                                            #
// # ********************************************************************************************* #
// # The NEORV32 Processor - https://github.com/stnolting/neorv32              (c) Stephan Nolting #
// #################################################################################################


/**********************************************************************//**
 * @file blink_led/main.c
 * @author Stephan Nolting
 * @brief Simple blinking LED demo program using the lowest 8 bits of the GPIO.output port.
 **************************************************************************/

#include <neorv32.h>



/**********************************************************************//**
 * @name User configuration
 **************************************************************************/
/**@{*/
/** UART BAUD rate */
#define BAUD_RATE 19200
/** Use the custom ASM version for blinking the LEDs defined (= uncommented) */
//#define USE_ASM_VERSION
/**@}*/
#define PWM_MAX 128

/**********************************************************************//**
 * ASM function to blink LEDs
 **************************************************************************/
extern void blink_led_asm(uint32_t gpio_out_addr);

/**********************************************************************//**
 * C function to blink LEDs
 **************************************************************************/
uint32_t keyboard(void);
void wb_regs(void);
void wb_calculadora(void);

/**********************************************************************//**
 * Main function; shows an incrementing 8-bit counter on GPIO.output(7:0).
 *
 * @note This program requires the GPIO controller to be synthesized (the UART is optional).
 *
 * @return 0 if execution was successful
 **************************************************************************/
int main() {

  // init UART (primary UART = UART0; if no id number is specified the primary UART is used) at default baud rate, no parity bits, ho hw flow control
  neorv32_uart0_setup(BAUD_RATE, PARITY_NONE, FLOW_CONTROL_NONE);

  // check if GPIO unit is implemented at all
  if (neorv32_gpio_available() == 0) {
    neorv32_uart0_print("Error! No GPIO unit synthesized!\n");
    return 1; // nope, no GPIO unit synthesized
  }

  // capture all exceptions and give debug info via UART
  // this is not required, but keeps us safe
  neorv32_rte_setup();

  // say hello
  neorv32_uart0_print("Project program\n");

  // deactivate all PWM channels
  neorv32_pwm_set(0, 0);
  neorv32_pwm_set(1, 0);
  neorv32_pwm_set(2, 0);
  neorv32_pwm_set(3, 0);

  // configure and enable PWM
  neorv32_pwm_setup(CLK_PRSC_64);

  //keyboard();
  while(1){
  wb_calculadora();
  }
}

void wb_regs(void){
 uint32_t data_in_reg=0xABCD1234;
 uint32_t data_from_reg;
 uint32_t address=0x90000000;

 neorv32_cpu_store_unsigned_word(address,data_in_reg);
 data_from_reg=neorv32_cpu_load_unsigned_word(address);

 neorv32_uart0_printf("Dato leido: %x\n",data_from_reg); 
}

//uint32_t tecla_act=0xFF;
//static uint32_t tecla_ant=0xFF;

uint32_t keyboard(void) {
  uint32_t tecla_act=0xFF;
  uint32_t tecla_ant=0xFF;
  int fil=0;
  int col=0;
  uint32_t tecla[4][4]={{0x1,0x2,0x3,0xA},{0x4,0x5,0x6,0xB},{0x7,0x8,0x9,0xC},{0x0,0xF,0xE,0xD}};
  
  

  neorv32_gpio_port_set(0xFF);

  while (1) {
    tecla_act=0xFF;
    for(col=0;col<4;col++){
      neorv32_gpio_pin_clr(col);
      for(fil=4;fil<8;fil++){
        if(neorv32_gpio_pin_get(fil)==0){
          //neorv32_uart0_printf("Pulsada tecla en fila %u y columna %u\n",7-fil,3-col);
          //neorv32_uart0_printf("Pulsada tecla %c\n",tecla[7-fil][3-col]);
          tecla_act=tecla[7-fil][3-col];
          if (tecla_act!=0xFF)
            neorv32_gpio_port_set(((int)(tecla_act)*16) & 0xF0);
        }
      }
      neorv32_gpio_pin_set(col);
    }

    if(tecla_act==0xFF && tecla_act!=tecla_ant){    //Devolver si se ha soltado el boton (act=0xFF y ant=tecla pulsada)
      //neorv32_uart0_printf("Pulsada tecla %u\n",tecla_act);
      neorv32_cpu_delay_ms(50);
      return tecla_ant;
    }
    tecla_ant=tecla_act; 
  }
}


void wb_calculadora(void) {
    uint8_t pwm = 0; 
    uint8_t ch = 0;
    uint32_t operando1=0;
    uint32_t operando2=0;
    uint32_t operando_in_reg=0;
    uint32_t operador=0;
    uint32_t recibido=0;
   
    uint32_t resultado;
    uint32_t resultado_in_reg;
    uint32_t address = 0x90000000;  // Dirección base

    // Solicitar el primer operando
    neorv32_uart0_printf("Ingrese el primer operando, el operador y el segundo operando, en ese orden.\n");
    neorv32_uart0_printf("Operadores: A:+, B:-, C:*, D:/.\n");
    //operando1 = (uint32_t)(neorv32_uart0_getc() - '0');  // Convertir de ASCII a entero
    
    while((recibido=keyboard())<10){     
      operando_in_reg=operando_in_reg*10+recibido;
    }
    
      
    
    neorv32_uart0_printf("Operando 1: %u.\n", operando_in_reg);
    neorv32_cpu_store_unsigned_word(address, operando_in_reg);  // Guardar en memoria

    // Solicitar el operador
    //neorv32_uart0_printf("Ingrese el operador (+: suma, -: resta, *: multiplicacion, /: division): \n");
    //operador = (uint32_t)(neorv32_uart0_getc());  // No convertir aun
    neorv32_uart0_printf("Operador: %x.\n", recibido);
    neorv32_cpu_store_unsigned_word(address + 8, recibido);  // Guardar en memoria

    // Solicitar el segundo operando
    neorv32_uart0_printf("Ingrese el segundo operando: \n");
    //operando2 = (uint32_t)(neorv32_uart0_getc() - '0');  // Convertir de ASCII a entero
    //recibido=0;
    //tecla_ant=0xFF;
    //tecla_act=0xFF;
    operando_in_reg=0;
    while((recibido=keyboard())<10){  
      operando_in_reg=operando_in_reg*10+recibido;
    }
    neorv32_uart0_printf("Operando 2: %u.\n", operando_in_reg);
    neorv32_cpu_store_unsigned_word(address + 4, operando_in_reg);  // Guardar en memoria

    // Leer operandos y operador desde memoria
    operando1 = neorv32_cpu_load_unsigned_word(address);
    operando2 = neorv32_cpu_load_unsigned_word(address + 4);
    operador = neorv32_cpu_load_unsigned_word(address + 8);

    // Realizar la operación según el operador
    switch (operador) {
        case 10:  // Suma (10=A=+)
            resultado_in_reg = operando1 + operando2;
            ch=0;
            break;
        case 11:  // Resta (11=B=-)
            resultado_in_reg = operando1 - operando2;
            ch=1;
            break;
        case 12:  // Multiplicación (12=C=*)
            resultado_in_reg = operando1 * operando2;
            ch=2;
            break;
        case 13:  // División (con validación) (13=D=/)
            if (operando2 != 0) {
                resultado_in_reg = operando1 / operando2;
                ch=3;
            } else {
                resultado_in_reg = 0xFFFFFFFF;  // Resultado especial para división por cero
                neorv32_uart0_printf("Error: Division por cero\n");
            }
            break;
        default:  // Operador no válido
            resultado_in_reg = 0xFFFFFFFF;  // Resultado especial para error
            neorv32_uart0_printf("Error: Operador no valido\n");
            break;
    }
    pwm=100;
    neorv32_pwm_set(ch, pwm);
    neorv32_cpu_delay_ms(1000); // wait ~10ms
    
    // deactivate all PWM channels
    neorv32_pwm_set(ch, 0);


    // Almacenar el resultado en memoria
    neorv32_cpu_store_unsigned_word(address + 12, resultado_in_reg);  // Dirección 0x9000000C

    // Leer resultado
    resultado = neorv32_cpu_load_unsigned_word(address + 12);

    // Mostrar el resultado
    neorv32_uart0_printf("Resultado: %u\n", resultado);  // Mostrar como número entero
}

