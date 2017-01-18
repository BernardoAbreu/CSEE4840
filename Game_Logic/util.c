#include "util.h"

extern int vga_led_fd;
extern screen back;
extern sprite_info ground[3][3];
extern int line_length[3];

void clean ()
{ 
    back.life_1 = 0;
    back.life_2 = 0;
    back.background_color = 0x0;
    back.choice = CLOUDS;
  
    sprite_info cleaner;
    cleaner.count = 0;

    int i, j;
    for (i = 0; i < 2; i++)
    {
        cleaner.layer = i;

        for (j = 0; j < 8; j++)
        {
            cleaner.id = j; 
            write_info(cleaner, back);
        }
    }

}

void write_sprite(sprite_info sprite)
{
    vga_screen_arg_t screen_game;
    screen_game.sprite = sprite;
    screen_game.option = SPRITE;
    if (ioctl(vga_led_fd, VGA_LED_WRITE_DIGIT, &screen_game))
    {
        perror("ioctl(VGA_LED_WRITE_DIGIT) failed");
        return;
    }
}

void write_screen(screen background)
{
    vga_screen_arg_t screen_game;
    screen_game.background = background;
    screen_game.option = BACK;
    if (ioctl(vga_led_fd, VGA_LED_WRITE_DIGIT, &screen_game))
    {
        perror("ioctl(VGA_LED_WRITE_DIGIT) failed");
        return;
    }
}


void write_info(sprite_info sprite, screen background)
{
    vga_screen_arg_t screen_game;
    screen_game.sprite = sprite;
    screen_game.background = background;
    screen_game.option = BOTH;
    if (ioctl(vga_led_fd, VGA_LED_WRITE_DIGIT, &screen_game))
    {
        perror("ioctl(VGA_LED_WRITE_DIGIT) failed");
        return;
    }
}

void generate_ground (int line, int row)
{            

    // Cleaning row to be used
    if (line_length[row] != -1)
    {   
        int j;
        for (j = 0; j < line_length[row]; j++)
        {
            ground[row][j].count = 0;
            write_sprite(ground[row][j]);
        }
    }

    int blocks_chance = rand() % 20;
    // 3 blocks = 50% chance
    // 2 blocks = 40% chance 
    // 1 block  = 10% chance

    if (blocks_chance < 10)
    {   
        // Checking if it won't surpass the maximum number of sprites allowed(8)
        int i;
        int sum = 0;
        for (i = 0; i < 3; i++)
            sum += line_length[i];
        sum -= line_length[row];

        if (sum == 6)
        {
            line_length[row] = 2;
        }
        else
        {
            line_length[row] = 3;
        }
    }
    else if (blocks_chance < 18)
    {
        line_length[row] = 2;
    }
    else
    {
        line_length[row] = 1;
    }

    // Defining the ground size, starting point, id and layer
    // The starting point is defined depending on how many blocks we
    // have per line (line_length[row]) and an offset of 8 pixels on 
    // each side of the screen is added.  (8 ~ 632)

    int j;
    int dx = (632 - 8) / line_length[row];        // size of each column 
    for (j = 0; j < line_length[row]; j++)
    {
        ground[row][j].shape = GROUND;
        ground[row][j].pos.y = line;
        ground[row][j].layer = SCENARIO;
        ground[row][j].orientation = RIGHT;

        // Defining the id of each sprite
        if (row != 0)
        {
            ground[row][j].id = 
                ground[row - 1][line_length[row - 1] - 1].id + j + 1;
        }
        else
        {
            ground[row][j].id = j;
        }

        int begin = dx * j + 8;                 // begin of curr. column
        
        // 5 blocks long = 50% chance
        // 4 blocks long = 40% chance
        // 3 blocks long = 10% chance
        
        int length_chance = rand() % 20;

        if (length_chance < 10)
        {     
            ground[row][j].count = 5;
            ground[row][j].pos.x =
                (rand() % (dx - ground[row][j].count)) + begin;

        }
        else if (length_chance < 18)
        {
            ground[row][j].count = 4;
            ground[row][j].pos.x =
                (rand() % (dx - ground[row][j].count)) + begin;
        }
        else
        {
            ground[row][j].count = 3;
            ground[row][j].pos.x =
                (rand() % (dx - ground[row][j].count)) + begin;
        }
    }
}

void x_translation (character *c, character other)
{
    int collision = 0;
    
    // Checking collision

    // If sprites have pixels on the same line
    if (((other.pos->y + OFFSET <= c->pos->y + OFFSET) &&
        (other.pos->y + OFFSET >= c->pos->y - OFFSET)) ||
        ((other.pos->y - OFFSET <= c->pos->y + OFFSET) &&
        (other.pos->y - OFFSET >= c->pos->y - OFFSET)))
    {
        // Check collision in case of negative speed
        if (c->vx < 0)
        {
            if (other.pos->x + OFFSET < c->pos->x - OFFSET)
            {
                collision = (other.pos->x + OFFSET >= c->pos->x + c->vx - OFFSET);

                if (collision)
                {
                    c->pos->x = other.pos->x + 2 * OFFSET + 1;
                    return;
                }

            }
        }

        // Check collision in case of positive speed
        if (c->vx > 0)
        {
            if (other.pos->x - OFFSET > c->pos->x + OFFSET)
            {
                collision = (other.pos->x - OFFSET <= c->pos->x + c->vx + OFFSET);

                if (collision)
                {
                    c->pos->x = other.pos->x - 2 * OFFSET - 1;
                    return;
                }
            }
        }
    }
    
    // No collision
    c->pos->x += c->vx;

   // Checking borders condition
   if (c->pos->x <= 0)
    c->pos->x = 640 + c->pos->x;
   if (c->pos->x >= 640)
    c->pos->x = c->pos->x - 640;

}

void y_translation (character *c, character other)
{
    int collision = 0;
    int first_collision;
    
    // Checking collision

    // If the other character has pixels on the same column
    if (((other.pos->x + OFFSET <= c->pos->x + OFFSET) &&
        (other.pos->x + OFFSET >= c->pos->x - OFFSET)) ||
        ((other.pos->x - OFFSET <= c->pos->x + OFFSET) &&
        (other.pos->x - OFFSET >= c->pos->x - OFFSET)))
    {
        // Check collision in case of negative speed (jump)
        if (c->vy < 0)
        {
            if (other.pos->y + OFFSET < c->pos->y - OFFSET)
            {
                collision = (other.pos->y + OFFSET >= c->pos->y + c->vy - OFFSET);

                if (collision)
                {
                    c->pos->y = other.pos->y + 2 * OFFSET + 1;
                    return;
                }

            }
        }

        // Check collision in case of positive speed (fall)
        if (c->vy > 0)
        {
            if (other.pos->y - OFFSET > c->pos->y + OFFSET)
            {
                collision = (other.pos->y - OFFSET <= c->pos->y + c->vy + OFFSET);

                if (collision)
                {
                    first_collision = other.pos->y - 2 * OFFSET - 1;
                    c->jumping = 0;
                }
            }
        }
    }

    // Check for platform collision (only fall apply)
    if (c->vy > 0)
    {
        int i, j;
        for (i = 0; i < 3; i++)
        {
            for (j = 0; j < line_length[i]; j++)
            {
                // Checking if it's above one of the platforms
                if ((c->pos->x + OFFSET > ground[i][j].pos.x - OFFSET) &&
                    (c->pos->x - OFFSET < ground[i][j].pos.x + 
                        2 * ground[i][j].count * OFFSET - OFFSET))
                {
                    if (c->pos->y  + OFFSET < ground[i][j].pos.y - OFFSET)
                    {
                        int collision2 = (c->pos->y + c->vy + OFFSET >= 
                            ground[i][j].pos.y - OFFSET);

                        if (collision2)
                        {
            			    c->jumping = 0;
                            
                            int second_collision = ground[i][j].pos.y - 2 * OFFSET - 1;
                            
                            if (collision)
                            {
                                c->pos->y = (first_collision < second_collision) ?
                                    first_collision : second_collision;
                                return;
                            }
                            
                            c->pos->y = second_collision;
            			    return;
                        }
                    }
                }
            }
        }
    }

    if (collision)
    {
        c->pos->y = first_collision;
        return;
    }

    // No collision
    c->pos->y += c->vy;
    c->jumping = 1;
}
