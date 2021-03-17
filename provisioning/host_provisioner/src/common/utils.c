/*************************************************************************
    > File Name: utils.c
    > Author: Kevin
    > Created Time: 2019-03-06
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "utils.h"
#include "prov_assert.h"

#include "err_code.h"

/* Defines  *********************************************************** */
#define STR_HEX_VALID(x)                  ((((x) >= '0') && ((x) <= '9')) || (((x) >= 'a') && ((x) <= 'f'))  || (((x) >= 'A') && ((x) <= 'F')))
/* Static Variables *************************************************** */

/* Static Functions Declaractions ************************************* */

/* Global Variables *************************************************** */
int array_all_0(uint8_t *array, size_t len)
{
  int i = 0;

  if (!array) {
    return 0;
  }

  for (i = 0; i < len; i++) {
    if (array[i] != 0) {
      return 0;
    }
  }

  return 1;
}

int getLeftFirstOne(unsigned int v)
{
  v ^= v - 1;
  v = (v & 0x55555555) + ((v >> 1) & 0x55555555);
  v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
  v = (v + (v >> 4)) & 0x0F0F0F0F;
  v += v >> 8;
  v += v >> 16;

  return (v & 0x3F) - 1;
}

int getRightFirstOne(unsigned int v)
{
  int pos = 0;

  if (!v) {
    return -1;
  }

  if (!(v & 0xFFFF0000)) {
    v <<= 16;
    pos += 16;
  }
  if (!(v & 0xFF000000)) {
    v <<= 8;
    pos += 8;
  }
  if (!(v & 0xF0000000)) {
    v <<= 4;
    pos += 4;
  }
  if (!(v & 0xC0000000)) {
    v <<= 2;
    pos += 2;
  }
  if (!(v & 0x80000000)) {
    pos += 1;
  }

  return pos;
}

char toHexChar(char x)
{
  /* Calling function needs to make sure the format 0-9, a-f, A-F */
  if (x >= 0 && x <= 9) {
    return x + '0';
  } else if (x >= 0xa && x <= 0xf) {
    return x = x + 'a' - 10;
  } else {
    hardASSERT(0);
    return x;
  }
}

void uint32ToStr(uint16_t v, char *p)
{
  p[7] = toHexChar(v & 0x0000000F);
  p[6] = toHexChar((v & 0x000000F0) >> 4);
  p[5] = toHexChar((v & 0x00000F00) >> 8);
  p[4] = toHexChar((v & 0x0000F000) >> 12);
  p[3] = toHexChar((v & 0x000F0000) >> 16);
  p[2] = toHexChar((v & 0x00F00000) >> 20);
  p[1] = toHexChar((v & 0x0F000000) >> 24);
  p[0] = toHexChar((v & 0xF0000000) >> 28);
  p[8] = 0;
}

void uint16ToStr(uint16_t v, char *p)
{
  p[3] = toHexChar(v & 0x000F);
  p[2] = toHexChar((v & 0x00F0) >> 4);
  p[1] = toHexChar((v & 0x0F00) >> 8);
  p[0] = toHexChar((v & 0xF000) >> 12);
  p[4] = 0;
}

void uint8ToStr(uint16_t v, char *p)
{
  p[1] = toHexChar(v & 0x0F);
  p[0] = toHexChar((v & 0xF0) >> 4);
  p[2] = 0;
}

/**
 * @brief	str2int - Convert string to unsigned integer
 *
 * @param	input   - input string value
 * @param	length  - length of the input string
 * @param	p_ret   - Converted integer, if return is not #0, it should be ignored.
 *
 * @return        - success on 0, fail on none-zero
 */
int str2uint(const char *input, size_t length, uint32_t *p_ret)
{
  int base = 10, ret = 0, tmp, i;
  char *x_ret = NULL;
  x_ret = strstr(input, "0x");
  if (x_ret) {
    base = 16;
    /* Format 0x---... */
    x_ret  += 2;
    length -= 2;
    for (i = length - 1; i >= 0; i--) {
      char tmp_c = x_ret[i];
      if (tmp_c >= '0' && tmp_c <= '9') {
        tmp = tmp_c - '0';
      } else if (tmp_c >= 'a' && tmp_c <= 'f') {
        tmp = tmp_c - 'a' + 10;
      } else if (tmp_c >= 'A' && tmp_c <= 'F') {
        tmp = tmp_c - 'A' + 10;
      } else {
        return E_INVP;
      }
      ret += tmp * pow(base, length - 1 - i);
    }
  } else {
    x_ret = (char *)input;
    base = 10;
    for (i = length - 1; i >= 0; i--) {
      char tmp_c = x_ret[i];
      if (tmp_c >= '0' && tmp_c <= '9') {
        tmp = tmp_c - '0';
      } else {
        return E_INVP;
      }
      ret += tmp * pow(base, length - 1 - i);
    }
  }
  *p_ret = ret;
  return 0;
}

/**
 * @brief	int2str   - Conver unsigned integer to string
 *
 * @param	input			- Integer to convert
 * @param	base_type - Hex or Dec
 * @param	length		- Length of #str[], avoid writing overflow
 * @param	str[]			- Output memory, should be ignore if return value is not #0
 *
 * @return	        - success on 0, fail on none-zero
 */
int uint2str(uint64_t input, uint8_t base_type, size_t length, char str[])
{
  uint64_t ret = 0;
  uint8_t base = 10, remaining = 0, idx = 0;
  /* pvPortMalloc(); */
  if (base_type == BASE_DEC) {
    ret = input / base;
    remaining = input % base;
    while (1) {
      if (idx == length) {
        return E_NOESIZE;
      }
      str[length - 1 - idx++] = '0' + remaining;
      if (!ret) {
        memmove(str, str + length - idx, idx);
        break;
      }
      remaining = ret % base;
      ret = ret / base;
    }
  } else if (base_type == BASE_HEX) {
    base = 16;
    ret = input / base;
    remaining = input % base;
    str[0] = '0';
    str[1] = 'x';
    while (1) {
      if (idx > length - 2) {
        return E_NOESIZE;
      }
      if (remaining < 10) {
        str[length - 1 - idx++] = '0' + remaining;
      } else {
        str[length - 1 - idx++] = 'a' + remaining - 10;
      }
      if (!ret) {
        memmove(str + 2, str + length - idx, idx);
        break;
      }
      remaining = ret % base;
      ret = ret / base;
    }
  } else {
    return E_INVP;
  }
  return E_SUC;
}

unsigned int bitPopcount(uint32_t bitmap)
{
  // hamming weight algorithm from Wikipedia (popcount_3)
  bitmap = bitmap - ((bitmap >> 1) & 0x55555555);
  bitmap = (bitmap & 0x33333333) + ((bitmap >> 2) & 0x33333333);
  return (((bitmap + (bitmap >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

int charBuf2str(const char src[], size_t srcLen, uint8_t rev,
                char dest[], size_t destLen)
{
  int len;
  const unsigned char *p;

  p = (const unsigned char *)src;
  if (p == NULL) {
    return E_INVP;
  }
  len = srcLen;

  if (destLen < len * 2) {
    return E_NOESIZE;
  }

  for (int i = 0; i < len; i++) {
    if (rev) {
      dest[i * 2] = toHexChar(*p % 16);
      dest[i * 2 + 1] = toHexChar(*p / 16);
    } else {
      dest[len * 2 - 1 - i * 2] = toHexChar(*p % 16);
      dest[len * 2 - 2 - i * 2] = toHexChar(*p / 16);
    }
    p++;
  }

  return E_SUC;
}

int str2charBuf(const char src[], uint8_t rev, char dest[], size_t destLen)
{
  int len;
  const char *p;
  char ret;

  p = src;
  if (p == NULL) {
    return E_INVP;
  }
  len = strlen(p);

  if (destLen < len / 2) {
    return E_NOESIZE;
  } else if (len % 2) {
    return E_INVP;
  }

  for (int i = 0; i < len / 2; i++) {
    if (STR_HEX_VALID(*p) && STR_HEX_VALID(*(p + 1))) {
      ret = ass(getHex(*p), getHex(*(p + 1)));
    } else {
      return E_INVP;
    }

    if (rev) {
      dest[i] = ret;
    } else {
      dest[len / 2 - 1 - i] = ret;
    }
    p += 2;
  }

  return E_SUC;
}
