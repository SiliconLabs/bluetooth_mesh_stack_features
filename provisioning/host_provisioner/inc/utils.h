/*************************************************************************
    > File Name: utils.h
    > Author: Kevin
    > Created Time: 2019-03-06
    >Description:
 ************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"
#include "logging.h"
#include "bg_stack/gecko_bglib.h"

#define BASE_DEC                                    0
#define BASE_HEX                                    1

#define RTT_CTRL_RESET                "[0m"         // Reset to default colors
#define RTT_CTRL_CLEAR                "[2J"         // Clear screen, reposition cursor to top left

#define RTT_CTRL_TEXT_BLACK           "[2;30m"
#define RTT_CTRL_TEXT_RED             "[2;31m"
#define RTT_CTRL_TEXT_GREEN           "[2;32m"
#define RTT_CTRL_TEXT_YELLOW          "[2;33m"
#define RTT_CTRL_TEXT_BLUE            "[2;34m"
#define RTT_CTRL_TEXT_MAGENTA         "[2;35m"
#define RTT_CTRL_TEXT_CYAN            "[2;36m"
#define RTT_CTRL_TEXT_WHITE           "[2;37m"

#define RTT_CTRL_TEXT_BRIGHT_BLACK    "[1;30m"
#define RTT_CTRL_TEXT_BRIGHT_RED      "[1;31m"
#define RTT_CTRL_TEXT_BRIGHT_GREEN    "[1;32m"
#define RTT_CTRL_TEXT_BRIGHT_YELLOW   "[1;33m"
#define RTT_CTRL_TEXT_BRIGHT_BLUE     "[1;34m"
#define RTT_CTRL_TEXT_BRIGHT_MAGENTA  "[1;35m"
#define RTT_CTRL_TEXT_BRIGHT_CYAN     "[1;36m"
#define RTT_CTRL_TEXT_BRIGHT_WHITE    "[1;37m"

#define RTT_CTRL_BG_BLACK             "[24;40m"
#define RTT_CTRL_BG_RED               "[24;41m"
#define RTT_CTRL_BG_GREEN             "[24;42m"
#define RTT_CTRL_BG_YELLOW            "[24;43m"
#define RTT_CTRL_BG_BLUE              "[24;44m"
#define RTT_CTRL_BG_MAGENTA           "[24;45m"
#define RTT_CTRL_BG_CYAN              "[24;46m"
#define RTT_CTRL_BG_WHITE             "[24;47m"

#define RTT_CTRL_BG_BRIGHT_BLACK      "[4;40m"
#define RTT_CTRL_BG_BRIGHT_RED        "[4;41m"
#define RTT_CTRL_BG_BRIGHT_GREEN      "[4;42m"
#define RTT_CTRL_BG_BRIGHT_YELLOW     "[4;43m"
#define RTT_CTRL_BG_BRIGHT_BLUE       "[4;44m"
#define RTT_CTRL_BG_BRIGHT_MAGENTA    "[4;45m"
#define RTT_CTRL_BG_BRIGHT_CYAN       "[4;46m"
#define RTT_CTRL_BG_BRIGHT_WHITE      "[4;47m"

#define OUTPUT_PREFIX                               "> "
#define DBG_PREFIX                                  "                                       ------- "
#define ERROR_PREFIX                                "<<ERROR>> : "
#define DBG_PADDING                                 "\n"
#define DBG_FLAG                                    DBG_COLOR " [DBG] " RTT_CTRL_RESET
#define LOG_FLAG                                    LOG_COLOR " [LOG] " RTT_CTRL_RESET
#define MSG_FLAG                                    MSG_COLOR " [MSG] " RTT_CTRL_RESET
#define ERR_FLAG                                    ERR_COLOR " [ERR] " RTT_CTRL_RESET
#define DBG_CONTEXT                                 "   >>"
#define FILE_LINE_INFO                              "[%s:%d]"
#define COLOR_PLACEHOLDER                           "%s"
#define FLAG_PLACEHOLDER                            "%s"

#define DBG_LOG_PREFIX_BODY DBG_PREFIX FILE_LINE_INFO DBG_PADDING LOG_FLAG DBG_CONTEXT
#define DBG_MSG_PREFIX_BODY DBG_PREFIX FILE_LINE_INFO DBG_PADDING MSG_FLAG DBG_CONTEXT
#define DBG_ERR_PREFIX_BODY DBG_PREFIX FILE_LINE_INFO DBG_PADDING ERR_FLAG DBG_CONTEXT

#define OUT_LOG_PREFIX_BODY LOG_FLAG OUTPUT_PREFIX
#define OUT_MSG_PREFIX_BODY MSG_FLAG OUTPUT_PREFIX
#define OUT_ERR_PREFIX_BODY ERR_FLAG OUTPUT_PREFIX

#define LOG_COLOR RTT_CTRL_TEXT_BRIGHT_GREEN
#define MSG_COLOR RTT_CTRL_TEXT_BRIGHT_BLUE
#define ERR_COLOR RTT_CTRL_TEXT_BRIGHT_RED
#define DBG_COLOR RTT_CTRL_RESET

#define HIGHLIGHT_COLOR RTT_CTRL_TEXT_BRIGHT_YELLOW RTT_CTRL_BG_RED

#define DBG_PLH_PREFIX_BODY DBG_PREFIX FILE_LINE_INFO DBG_PADDING FLAG_PLACEHOLDER DBG_CONTEXT

/* #define DBG_PLH_PREFIX  COLOR_PLACEHOLDER DBG_PLH_PREFIX_BODY */
#define DBG_PLH_PREFIX  DBG_PLH_PREFIX_BODY

#define DBG_DBG_PREFIX  DBG_LOG_PREFIX_BODY
#define DBG_LOG_PREFIX  DBG_LOG_PREFIX_BODY
#define DBG_MSG_PREFIX  DBG_MSG_PREFIX_BODY
#define DBG_ERR_PREFIX  DBG_ERR_PREFIX_BODY

#define OUT_PLH_PREFIX_BODY FLAG_PLACEHOLDER OUTPUT_PREFIX
/* #define OUT_PLH_PREFIX  COLOR_PLACEHOLDER OUT_PLH_PREFIX_BODY */
#define OUT_PLH_PREFIX  OUT_PLH_PREFIX_BODY

#define OUT_LOG_PREFIX  OUT_LOG_PREFIX_BODY
#define OUT_MSG_PREFIX  OUT_MSG_PREFIX_BODY
#define OUT_ERR_PREFIX  OUT_ERR_PREFIX_BODY

#ifdef __FILENAME__
#define __FILENAME                                  (__FILENAME__)
#else
#define __FILENAME                                  (__FILE__)
#endif

#define MAX(a, b)                                   ((a) > (b) ? (a) : (b))
#define MIN(a, b)                                   ((a) < (b) ? (a) : (b))

#define BIT_SET(v, bit)                             ((v) |= (1 << (bit)))
#define BIT_CLEAR(v, bit)                           ((v) &= ~(1 << (bit)))
#define IS_BIT_SET(v, bit)                          ((v) & (1 << (bit)) ? 1 : 0)

#define BUILD_UINT16(l, h)                          (uint16_t)(((l) & 0xFF) | (((h) & 0xFF) << 8))

#define SC(x, eMsg)                                 \
  do {                                              \
    if ((x) != 0) {                                 \
      CS_ERR("%s AT" FILE_LINE_INFO "\n",           \
             eMsg, __FILENAME, (uint32_t)__LINE__); \
    }                                               \
  } while (0)

#define BG_SC(x, eMsg)                              \
  do {                                              \
    if ((x)->result != 0) {                         \
      CS_ERR("BG %s AT" FILE_LINE_INFO "\n",        \
             eMsg, __FILENAME, (uint32_t)__LINE__); \
    }                                               \
  } while (0)

#define ROOT_P_TO_FILE(__fmt, __color, __flag, ...) \
  do {                                              \
    logToFile(DBG_PLH_PREFIX __fmt,                 \
              __FILENAME,                           \
              (uint32_t)__LINE__,                   \
              __flag,                               \
              ##__VA_ARGS__);                       \
  } while (0)

#if (DEBUG_PRINT == 1)
#define ROOT_P_TO_CS(__fmt, __color, __flag, ...) \
  do {                                            \
    printf(DBG_PLH_PREFIX __fmt,                  \
           __FILENAME,                            \
           (uint32_t)__LINE__,                    \
           __flag,                                \
           ##__VA_ARGS__);                        \
    printf(RTT_CTRL_RESET);                       \
  } while (0)

#define dbgPrint(__fmt, ...)                                   \
  do {                                                         \
    ROOT_P_TO_CS(__fmt, DBG_COLOR, DBG_FLAG, ##__VA_ARGS__);   \
    ROOT_P_TO_FILE(__fmt, DBG_COLOR, DBG_FLAG, ##__VA_ARGS__); \
  } while (0)

#define LOG_PREFIX  DBG_LOG_PREFIX
#define MSG_PREFIX  DBG_MSG_PREFIX
#define ERR_PREFIX  DBG_ERR_PREFIX
#define FL_CONTENT  __FILENAME__, (uint32_t)__LINE__,
#else /* -------------------------------------------------------- */
#define ROOT_P_TO_CS(__fmt, __color, __flag, ...) \
  do {                                            \
    printf(OUT_PLH_PREFIX __fmt,                  \
           __flag,                                \
           ##__VA_ARGS__);                        \
    printf(RTT_CTRL_RESET);                       \
  } while (0)

#define dbgPrint(__fmt, ...)                                   \
  do {                                                         \
    ROOT_P_TO_FILE(__fmt, DBG_COLOR, DBG_FLAG, ##__VA_ARGS__); \
  } while (0)
#define LOG_PREFIX  OUT_LOG_PREFIX
#define MSG_PREFIX  OUT_MSG_PREFIX
#define ERR_PREFIX  OUT_ERR_PREFIX

#define FL_CONTENT
#endif

#ifndef DISABLE_CS_OUTPUT
#define DEFAULT_STRING_LEN  30
#define HL_COLOR_LEN  strlen(HIGHLIGHT_COLOR)
#define RESET_COLOR_LEN  strlen(RTT_CTRL_RESET)

#define HL_START HIGHLIGHT_COLOR
#define HL_END RTT_CTRL_RESET
#define HL_SUCCESS HIGHLIGHT_COLOR "SUCCESS" RTT_CTRL_RESET
#define HL_FAILED HIGHLIGHT_COLOR "FAILED" RTT_CTRL_RESET

#define GET_HL_STR(name, __fmt, ...)                                      \
  char hlstr_##name[DEFAULT_STRING_LEN + HL_COLOR_LEN + RESET_COLOR_LEN]; \
  do {                                                                    \
    snprintf(hlstr_##name,                                                \
             DEFAULT_STRING_LEN + HL_COLOR_LEN + RESET_COLOR_LEN,         \
             HIGHLIGHT_COLOR __fmt RTT_CTRL_RESET,                        \
             ##__VA_ARGS__);                                              \
  } while (0)

#define GET_STR(name, fmt, ...)       \
  char buf##name[DEFAULT_STRING_LEN]; \
  do {                                \
    snprintf(buf##name,               \
             DEFAULT_STRING_LEN,      \
             fmt,                     \
             ##__VA_ARGS__);          \
  } while (0)

#define CS_HLE(__fmt, hl, ...)          \
  do {                                  \
    ROOT_P_TO_CS(                       \
      __fmt,                            \
      LOG_COLOR, LOG_FLAG,              \
      ##__VA_ARGS__,                    \
      HIGHLIGHT_COLOR hl RTT_CTRL_RESET \
      );                                \
    ROOT_P_TO_FILE(                     \
      __fmt,                            \
      LOG_COLOR, LOG_FLAG,              \
      ##__VA_ARGS__,                    \
      HIGHLIGHT_COLOR hl RTT_CTRL_RESET \
      );                                \
  } while (0)

#define CS_HLB(__fmt, hl, ...)           \
  do {                                   \
    ROOT_P_TO_CS(                        \
      __fmt,                             \
      MSG_COLOR, LOG_FLAG,               \
      HIGHLIGHT_COLOR hl RTT_CTRL_RESET, \
      ##__VA_ARGS__                      \
      );                                 \
    ROOT_P_TO_FILE(                      \
      __fmt,                             \
      MSG_COLOR, LOG_FLAG,               \
      HIGHLIGHT_COLOR hl RTT_CTRL_RESET, \
      ##__VA_ARGS__                      \
      );                                 \
  } while (0)

#if (PRINT_LEVEL >= LEVEL_LOG)
#define CS_LOG(__fmt, ...)                                     \
  do {                                                         \
    ROOT_P_TO_CS(__fmt, LOG_COLOR, LOG_FLAG, ##__VA_ARGS__);   \
    ROOT_P_TO_FILE(__fmt, LOG_COLOR, LOG_FLAG, ##__VA_ARGS__); \
  } while (0)
#else /* #if (PRINT_LEVEL >= LEVEL_LOG) */
#define CS_LOG(__fmt, ...)                                     \
  do {                                                         \
    ROOT_P_TO_FILE(__fmt, LOG_COLOR, LOG_FLAG, ##__VA_ARGS__); \
  } while (0)
#endif /* #if (PRINT_LEVEL >= LEVEL_LOG) */

#if (PRINT_LEVEL >= LEVEL_MSG)
#define CS_MSG(__fmt, ...)                                     \
  do {                                                         \
    ROOT_P_TO_CS(__fmt, MSG_COLOR, MSG_FLAG, ##__VA_ARGS__);   \
    ROOT_P_TO_FILE(__fmt, MSG_COLOR, MSG_FLAG, ##__VA_ARGS__); \
  } while (0)
#else /* #if (PRINT_LEVEL >= LEVEL_MSG) */
#define CS_MSG(__fmt, ...)                                     \
  do {                                                         \
    ROOT_P_TO_FILE(__fmt, MSG_COLOR, MSG_FLAG, ##__VA_ARGS__); \
  } while (0)
#endif /* #if (PRINT_LEVEL >= LEVEL_MSG) */

#if (PRINT_LEVEL >= LEVEL_ERR)
#define CS_ERR(__fmt, ...)                                     \
  do {                                                         \
    ROOT_P_TO_CS(__fmt, ERR_COLOR, ERR_FLAG, ##__VA_ARGS__);   \
    ROOT_P_TO_FILE(__fmt, ERR_COLOR, ERR_FLAG, ##__VA_ARGS__); \
  } while (0)
#else /* #if (PRINT_LEVEL >= LEVEL_ERR) */
#define CS_ERR(__fmt, ...)                                     \
  do {                                                         \
    ROOT_P_TO_FILE(__fmt, ERR_COLOR, ERR_FLAG, ##__VA_ARGS__); \
  } while (0)
#endif /* #if (PRINT_LEVEL >= LEVEL_ERR) */

#else /* #ifndef DISABLE_CS_OUTPUT */
#define CS_LOG(__fmt, ...)
#define CS_MSG(__fmt, ...)
#define CS_ERR(__fmt, ...)
#endif /* #ifndef DISABLE_CS_OUTPUT */

#define ERR_ST HL_START "ERROR Returned" HL_END

#define PRINT_IF_ERR(x)                \
  do {                                 \
    if ((x)) {                         \
      CS_ERR(ERR_ST                    \
             " Value:%d, String:%s\n", \
             (x),                      \
             ERR_NAME[(x)]);           \
    }                                  \
  } while (0)

#define __CS_SPLIT()                              CS_MSG("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n")
#define CS_SPLIT(x)                                                        \
  do {                                                                     \
    for (int app_h_cs_split = 0; app_h_cs_split < (x); app_h_cs_split++) { \
      __CS_SPLIT();                                                        \
    }                                                                      \
  } while (0)

#define __PRINTF_SPLIT()                          printf("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n")
#define PRINTF_SPLIT(x)                                                                \
  do {                                                                                 \
    for (int app_h_printf_split = 0; app_h_printf_split < (x); app_h_printf_split++) { \
      __PRINTF_SPLIT();                                                                \
    }                                                                                  \
  } while (0)

#define LOCK_MUTEX(mutex)                            \
  do {                                               \
    if ((ret = pthread_mutex_lock(&(mutex))) != 0) { \
      errExitEN(ret, "pthread_mutex_lock");          \
    }                                                \
  } while (0)

#define UNLOCK_MUTEX(mutex)                            \
  do {                                                 \
    if ((ret = pthread_mutex_unlock(&(mutex))) != 0) { \
      errExitEN(ret, "pthread_mutex_unlock");          \
    }                                                  \
  } while (0)

int array_all_0(uint8_t *array, size_t len);
int getLeftFirstOne(unsigned int v);
int getRightFirstOne(unsigned int v);
char toHexChar(char x);
void uint32ToStr(uint16_t v, char *p);
void uint16ToStr(uint16_t v, char *p);
void uint8ToStr(uint16_t v, char *p);

int charBuf2str(const char src[], size_t srcLen, uint8_t rev,
                char dest[], size_t destLen);
int str2charBuf(const char src[], uint8_t rev,
                char dest[], size_t destLen);

int str2uint(const char *input, size_t length, uint32_t *p_ret);
int uint2str(uint64_t input, uint8_t base_type, size_t length, char str[]);
unsigned int bitPopcount(uint32_t bitmap);
int sendMeshCmdToBglibInterface(int priority,
                                int sender,
                                int id,
                                struct gecko_cmd_packet *packet);

static inline char ass(char h, char l)
{
  return h * 16 + l;
}

static inline char getHex(char x)
{
  /* Calling function needs to make sure the format 0-9, a-f, A-F */
  if (((x) >= '0') && ((x) <= '9')) {
    return (x) - '0';
  } else if (((x) >= 'a') && ((x) <= 'f')) {
    return (x) - 'a' + 10;
  } else {
    return (x) - 'A' + 10;
  }
}

static inline void errExit(const char *pMsg)
{
  perror(pMsg);
  exit(EXIT_FAILURE);
}

static inline void err_exit(const char *pMsg)
{
  perror(pMsg);
  _exit(EXIT_FAILURE);
}

static inline void errExitEN(int errnum, const char *pMsg)
{
  fprintf(stderr, "%s | error code = %d\n", pMsg, errnum);
  exit(EXIT_FAILURE);
}

static inline void printUUID(uuid_128 uuid, bool debug)
{
  if (debug) {
    dbgPrint("UUID: ");
  } else {
    CS_LOG("UUID: ");
  }

  for (int excl_utils_printUUID = 0; excl_utils_printUUID < 16; excl_utils_printUUID++) {
    printf("%02X", uuid.data[15 - excl_utils_printUUID]);
    if (excl_utils_printUUID == 2 || excl_utils_printUUID == 5) {
      printf("-");
    }
  }
  printf("\n");
}

static inline void printUUIDByRawData(const uint8_t *uuid, bool debug)
{
  if (debug) {
    dbgPrint("UUID: ");
  } else {
    CS_LOG("UUID: ");
  }

  for (int excl_utils_printUUID = 0; excl_utils_printUUID < 16; excl_utils_printUUID++) {
    printf("%02X", uuid[15 - excl_utils_printUUID]);
    if (excl_utils_printUUID == 2 || excl_utils_printUUID == 5) {
      printf("-");
    }
  }
  printf("\n");
}
#endif
