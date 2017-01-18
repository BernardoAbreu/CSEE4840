/*
 * Userspace program that communicates with the vga_led device driver
 * primarily through ioctls
 *
 * Stephen A. Edwards
 * Columbia University
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

#include "util.h"
#include "input_user.h"

int vga_led_fd;
screen back;
sprite_info ground[3][3];
//sprite_info power_sprite;
int line_length[3] = { -1, -1, -1 };

int main()
{
    static const char filename[] = "/dev/vgaled";

    printf("GAME TEST Userspace program started\n");

    if ( (vga_led_fd = open(filename, O_RDWR)) == -1)
    {
        fprintf(stderr, "could not open %s\n", filename);
        return -1;
    }

    // Seeding srand
    srand(time(NULL));

    // Cleaning sprites
    clean();

    // Generating background
    back.life_1 = 4;
    back.life_2 = 4;
    // back.choice = COLOR;
    back.background_color = 0xff0000;

    // Printing the background 
    write_screen(back);

    // Generating start ground
    int row = 0;                    // The next row of ground to be generated
    generate_ground(320, row++);
    generate_ground(160, row++);
    generate_ground(0, row++);
    row = 0;

    // Writing ground driver
    int i, j;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < line_length[i]; j++)
        {
            write_sprite(ground[i][j]);
        }
    }

    // Setting grandpa and grandma starting positions
    sprite_info grandpa_sprite;
    grandpa_sprite.pos.y = 200;
    grandpa_sprite.pos.x = 200;
    grandpa_sprite.shape = GP_STAND;
    grandpa_sprite.id = GP_ID;
    grandpa_sprite.count = 1;
    grandpa_sprite.layer = OBJECTS;
    grandpa_sprite.orientation = RIGHT;
    write_sprite(grandpa_sprite);

    sprite_info grandma_sprite;
    grandma_sprite.pos.y = 200;
    grandma_sprite.pos.x = 260;
    grandma_sprite.shape = GM_STAND;
    grandma_sprite.id = GM_ID;
    grandma_sprite.count = 1;
    grandma_sprite.layer = OBJECTS;
    grandma_sprite.orientation = LEFT;
    write_sprite(grandma_sprite);

    // Creating characters structures
    character grandpa;
    grandpa.pos = &(grandpa_sprite.pos);
    grandpa.id = GP_ID;
    grandpa.vx = 0;
    grandpa.vy = 1;
    grandpa.jumping = 0;

    character grandma;
    grandma.pos = &(grandma_sprite.pos);
    grandma.id = GM_ID;
    grandma.vx = 0;
    grandma.vy = 1;
    grandma.jumping = 0;

    // Starting peripheric
    start_user_input();

    int count_ground = 0;
    while (1)
    {   
        // Line of platforms have a 175 pixels gap between one another
        if (count_ground == 35)
        {
            generate_ground(-8, row);
            row = (row + 1) % 3; 
            count_ground = 0;
        }
        count_ground++;

        // Moves all platforms 5 pixels down
        int i, j;
        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < line_length[i]; j++)
            {
                ground[i][j].pos.y += 5;
            }
        }

        // Writing to drivers
        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < line_length[i]; j++)
            {
                write_sprite(ground[i][j]);
            }
        }

        int input = input_from_user();

        // grandpa under user motion capture
        if (input !=  -100)
        {
            grandpa.vx = -2 * input;
        }


        grandma.vx = 1;

        // Try to move grandpa
        x_translation (&grandpa, grandma);
        y_translation (&grandpa, grandma);
        

        if (grandpa.jumping)        // Fall
    	{
    	    grandpa_sprite.shape = GP_JUMP;
    	    grandpa.vy += 1;
    	}
        else                        // Jump
    	{
    	    grandpa_sprite.shape = GP_STAND;
    	    grandpa.jumping = 1;   
            grandpa.vy = -13;
    	}

        write_sprite(grandpa_sprite);

        // Try to move grandma
        x_translation (&grandma, grandpa);
        y_translation (&grandma, grandpa);
        
        if (grandma.jumping)        // Fall
    	{
    	    grandma_sprite.shape = GM_JUMP;
     	    grandma.vy += 1;
    	}
        else                        // Jump
    	{
    	    grandma_sprite.shape = GM_STAND;
    	    grandma.jumping = 1;
            grandma.vy = -13;
    	}
        
        write_sprite(grandma_sprite);

    	// Check for end of game
    	if (grandpa.pos->y >= 480 )
        {
            if (!(--back.life_2))
            {
                back.choice = COLOR;
                write_screen(back);
                exit(1);
            }
            write_screen(back);
    	    grandpa.pos->y = 20;
            write_sprite(grandpa_sprite);
        }
        if (grandma.pos->y >= 480)
        {
            grandma.pos->y = 20;
            write_sprite(grandma_sprite);
        }
              
        usleep(30000);

    }

    printf("GAME TEST Userspace program terminating\n");

    return 0;
}
