/*
 *
 * Lab 3
 *
 * Name: Bernardo de Almeida Abreu
 * UNI:  bd2440
 *
 * Name: Henrique Pizzol Grando
 * UNI:  hp2409
 * 
 * Name: Lucas Ikenaga Barros
 * UNI:  li2176
 *
 * Name: Tomas Mantelato
 * UNI:  tm2779
 * 
 */

#ifndef _VGA_LED_H
#define _VGA_LED_H

#include <linux/ioctl.h>


typedef struct 
{
    unsigned char id;
	short sprite_x, sprite_y;
  	unsigned char shape;
  	unsigned char count;
  	unsigned char layer;
    unsigned char orientation;	
} sprite_info;

typedef struct
{
	unsigned char life_1, life_2;
  	unsigned int background_color;	
} screen;

typedef struct
{
	sprite_info sprite;
	screen background;
} vga_screen_arg_t;


/* ioctls and their arguments */
#define VGA_LED_WRITE_DIGIT _IOW('q', 1, vga_screen_arg_t *)
#define VGA_LED_READ_DIGIT  _IOR('q', 2, vga_screen_arg_t *)

#endif
