#ifndef _INPUT_USER_H_
#define _INPUT_USER_H_

#include "input_user.h"
#include "usbkeyboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_HOST "192.168.1.1"
#define SERVER_PORT 42000

#define BUFFER_SIZE 128

/*
 * References:
 *
 * http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html
 * http://www.thegeekstuff.com/2011/12/c-socket-programming/
 * 
 */


struct libusb_device_handle *keyboard;
uint8_t endpoint_address;

  int err, col;

  struct sockaddr_in serv_addr;

  struct usb_keyboard_packet packet;
  int transferred;
  char keystate[12];


// starts keyboard
void start_user_input();

// used on the game.c
int input_from_user();


int get_number(uint8_t keycode);


#endif
