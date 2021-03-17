/***************************************************************************//**
 * @file
 * @brief read_char.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if _WIN32 == 1
#include <conio.h>
#else // _WIN32 != 1
#include <termios.h>
#include <time.h>
#endif // _WIN32 != 1

#include "read_char.h"

#if _WIN32 == 1
int32_t readChar(void)
{
  int32_t character;

  // Check if button was pressed
  if (kbhit()) {
    // Button was pressed, query button
    character = getch();
    // Echo back character pressed
    printf("%c", character);
    // Convert Carriage Return to New Line
    if (character == '\r') {
      printf("\n");
    }
    // Flush STDOUT buffer
    fflush(stdout);
  } else {
    // No character was pressed, return 0
    character = '\0';
  }

  return character;
}

#else // _WIN32 != 1
int32_t readChar(void)
{
  int32_t character;
  struct termios orig_term_attr;
  struct termios new_term_attr;

  //set the terminal to raw mode
  tcgetattr(fileno(stdin), &orig_term_attr);
  memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
  new_term_attr.c_lflag &= ~(ICANON);
  new_term_attr.c_cc[VTIME] = 0;
  new_term_attr.c_cc[VMIN] = 0;
  tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);

  // read a character from the stdin stream without blocking
  // returns EOF (-1) if no character is available
  character = fgetc(stdin);

  // restore the original terminal attributes
  tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);

  return character;
}
#endif // _WIN32 != 1
