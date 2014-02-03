/* ChibiOS/GFX demo - "neko"
 * A port of the classic japanese computer desktop toy (although most of
 * us know it from the Macintosh Desk Accessory that came a year later..)
 * --
 * Copyright Chris Baird <cjb@brushtail.apana.org.au> April 2013
 */

/* This file is part of stmneko.
 *
 * stmneko is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * stmndeko distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ch.h"
#include "hal.h"
#include "gfx.h"

#include "neko_bitmaps.h"

coord_t neko_x=144, neko_y=104;
color_t background = HTML2COLOR(0xC0C0C0);

char *rundir[] =
  {
    upleft1_bits, upleft2_bits, upright1_bits, upright2_bits,
    dwleft1_bits, dwleft2_bits, dwright1_bits, dwright2_bits,
    up1_bits,     up2_bits,     up1_bits,      up2_bits,
    down1_bits,   down2_bits,   down1_bits,    down2_bits,
    left1_bits,   left2_bits,   right1_bits,   right2_bits,
    left1_bits,   left2_bits,   right1_bits,   right2_bits,
  };


/* ---------------------------------------------------------------------- */

void draw_1bpp_bitmap (coord_t x, coord_t y, int width, int height,
		       color_t colour, char bitmap[])
{
  int i, j, mask;
  char *p = bitmap;

  for (j = 0; j < height; y++, j++)
    {
      mask = 1;
      for (i = 0; i < width; x++, i++)
	{
	  if (*p & mask)
	    gdispDrawPixel (x, y, colour);
	  if ((mask <<= 1) == 256)
	    p++, mask = 1;
	}
      x -= width;
    }
}


/* ---------------------------------------------------------------------- */

void draw_kitty (int delay, char bitmap[])
{
  draw_1bpp_bitmap (neko_x, neko_y, 32,32, 0, bitmap);
  chThdSleepMilliseconds (delay);
  draw_1bpp_bitmap (neko_x, neko_y, 32,32, background, bitmap);
}


void sleep_cycle (void)
{
  draw_kitty (500, sleep1_bits);
  draw_kitty (500, sleep2_bits);
}


void yawn_cycle (void)
{
  draw_kitty (1000, mati2_bits);
  draw_kitty (1500, mati3_bits);
}


void scratch_cycle (void)
{
  draw_kitty (500, kaki1_bits);
  draw_kitty (500, kaki2_bits);
}


void lick_cycle (void)
{
  draw_kitty (500, mati2_bits);
  draw_kitty (500, jare2_bits);
}


/* ---------------------------------------------------------------------- */

unsigned short random (void)
{
  static unsigned short lfsr = 0xACE1u;
  unsigned bit;

  bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
  return lfsr = (lfsr >> 1) | (bit << 15);
}


/* ---------------------------------------------------------------------- */


int main(void)
{
  int i;

  halInit();
  chSysInit();
  gdispInit();
  chThdSleepMilliseconds (10);
  gdispClear(background);	/* glitches.. */
  chThdSleepMilliseconds (10);
  gdispClear(background);	/* glitches.. */
  chThdSleepMilliseconds (10);
  gdispClear(background);	/* glitches.. */

  draw_kitty (1500, mati2_bits);
  draw_kitty (500, awake_bits);

  while(TRUE)
    {
      if (random() & 1)
	lick_cycle(), lick_cycle(), lick_cycle();
      if (random() & 1)
	scratch_cycle(), scratch_cycle(), scratch_cycle();
      if (random() & 8)
	{
	  yawn_cycle();
	  sleep_cycle(), sleep_cycle(), sleep_cycle(),
	  sleep_cycle(), sleep_cycle(), sleep_cycle();
	}

      /* chase sequence */

      draw_kitty (500, awake_bits);
      draw_kitty (500, mati2_bits);

      coord_t dest_x = -30 + (random() % 348);
      coord_t dest_y = -30 + (random() % 300);

      if (dest_x < 0)   dest_x = 0;
      if (dest_x > 288) dest_x = 288;
      if (dest_y < 0)   dest_y = 0;
      if (dest_y > 208) dest_y = 208;

      int dx = abs (dest_x - neko_x);
      int dy = abs (dest_y - neko_y);
      int sx = neko_x < dest_x ? 1 : -1;
      int sy = neko_y < dest_y ? 1 : -1;
      int err = (dx > dy ? dx : -dy)/2;
      int e2;
      char *step1_bits, *step2_bits, *step_temp;

      i = 3 + sx + 2*sy + 8*((2*dx)<dy) + 16*((2*dy)<dx);
      step1_bits = rundir[i];
      step2_bits = rundir[i+1];

      int run_little_kitty = 1;
      while (run_little_kitty)
	{
	  draw_kitty (250, step1_bits);

	  for (i = 0; i < 5; i++)
	    if ((neko_x == dest_x) && (neko_y == dest_y))
	      run_little_kitty = 0;
	    else
	      {
		e2 = err;
		if (e2 > -dx)
		  err -= dy, neko_x += sx;
		if (e2 < dy)
		  err += dx, neko_y += sy;
	      }

	  step_temp = step1_bits;
	  step1_bits = step2_bits;
	  step2_bits = step_temp;
	}

      /* scratch sequence (if needed) */

      step1_bits = NULL;

      if (neko_x == 0)
	step1_bits = ltogi1_bits, step2_bits = ltogi2_bits;
      if (neko_x == 288)
	step1_bits = rtogi1_bits, step2_bits = rtogi2_bits;
      if (neko_y == 0)
	step1_bits = utogi1_bits, step2_bits = utogi2_bits;
      if (neko_y == 208)
	step1_bits = dtogi1_bits, step2_bits = dtogi2_bits;

      if (step1_bits)
	for (i = 0; i < 4; i++)
	  {
	    draw_kitty (333, step1_bits);
	    draw_kitty (333, step2_bits);
	  }
    }
}
