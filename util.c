#include "util.h"

extern int vga_led_fd;
extern int line_length[3];
extern sprite_info *ground[3];

void write_info(sprite_info sprite, screen background)
{
    vga_screen_arg_t screen_game;
    screen_game.sprite = sprite;
    screen_game.background = background;
    if (ioctl(vga_led_fd, VGA_LED_WRITE_DIGIT, &screen_game))
    {
        perror("ioctl(VGA_LED_WRITE_DIGIT) failed");
        return;
    }
}

void generate_ground (int line, int row)
{            

    screen back;
    back.life_1 = 2;
    back.life_2 = 2;
    back.background_color = 0x152050;

    // 3 blocks = 50% chance
    // 2 blocks = 40% chance 
    // 1 block  = 10% chance

    if (line_length[row] != -1)
    {   
        int j;
        for (j = 0; j < line_length[row]; j++)
        {
            ground[row][j].count = 0;
            write_info(ground[row][j], back);
            
        }
        free(ground[row]);
    }

    int blocks_chance = rand() % 20;

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
            ground[row] = (sprite_info *) malloc (2 * sizeof(sprite_info));
            line_length[row] = 2;
        }
        else
        {
            ground[row] = (sprite_info *) malloc (3 * sizeof(sprite_info));
            line_length[row] = 3;
        }
    }
    else if (blocks_chance < 18)
    {
        ground[row] = (sprite_info *) malloc (2 * sizeof(sprite_info));
        line_length[row] = 2;
    }
    else
    {
        ground[row] = (sprite_info *) malloc (1 * sizeof(sprite_info));
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
        
        // 6 blocks long = 50% chance
        // 5 blocks long = 40% chance
        // 4 blocks long = 10% chance
        
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
                    c->pos->x = other.pos->x + OFFSET + 1;
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
                    c->pos->x = other.pos->x - OFFSET - 1;
                }
            }
        }
    }
    if (!collision)
    {
        c->pos->x += c->vx;
    }
}