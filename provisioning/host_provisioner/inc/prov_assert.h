/*************************************************************************
    > File Name: prov_assert.h
    > Author: Kevin
    > Created Time: 2019-03-06
    >Description:
 ************************************************************************/

#ifndef PROV_ASSERT_H
#define PROV_ASSERT_H
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

#define ASSERT_ST HL_START "ASSERT ERROR" HL_END
#define softASSERT(x)             \
  do {                            \
    if ((x) == 0) {               \
      CS_ERR(ASSERT_ST            \
             ", %s:%d\n",         \
             __FILE__,            \
             (uint32_t)__LINE__); \
    }                             \
  } while (0)

#define hardASSERT(x)             \
  do {                            \
    if ((x) == 0) {               \
      CS_ERR(ASSERT_ST            \
             ", %s:%d\n",         \
             __FILE__,            \
             (uint32_t)__LINE__); \
      abort();                    \
    }                             \
  } while (0)

#define softSucASSERT(x)          \
  do {                            \
    if ((x) != E_SUC) {           \
      CS_ERR(ASSERT_ST            \
             ", %s:%d\n",         \
             __FILE__,            \
             (uint32_t)__LINE__); \
      PRINT_IF_ERR((x));          \
    }                             \
  } while (0)

#define hardSucASSERT(x)          \
  do {                            \
    if ((x) != E_SUC) {           \
      CS_ERR(ASSERT_ST            \
             ", %s:%d\n",         \
             __FILE__,            \
             (uint32_t)__LINE__); \
      PRINT_IF_ERR((x));          \
      abort();                    \
    }                             \
  } while (0)

#define errASSERT(x, msg)         \
  do {                            \
    if ((x) == 0) {               \
      CS_ERR(ASSERT_ST            \
             ", %s:%d\n",         \
             __FILE__,            \
             (uint32_t)__LINE__); \
      CS_ERR(msg);                \
      abort();                    \
    }                             \
  } while (0)
#endif
