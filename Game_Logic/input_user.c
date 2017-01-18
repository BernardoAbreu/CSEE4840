#include "input_user.h"
#include <stdlib.h>
#include <stdio.h>


void start_user_input(){
  if ( (keyboard = openkeyboard(&endpoint_address)) == NULL ) {
    fprintf(stderr, "Did not find the accelerometer (keyboard)\n");
  }
}

int input_from_user(){
	libusb_interrupt_transfer(keyboard, endpoint_address,
       		 (unsigned char *) &packet, sizeof(packet),
            	&transferred, 0);
    int ret = 0;
    if (transferred == sizeof(packet)) {
     	ret = get_number(packet.keycode[0]);
      }
    return ret;
	  
}
int get_number(uint8_t keycode){
     char c = 'a' + keycode - 0x04;
     if (keycode == 0x00 ) return -100;
//velocity -10 = a, -9 = b ....
  int ret = (int) (c - 107);
  return ret;
}
