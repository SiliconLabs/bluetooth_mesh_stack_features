/*************************************************************************
    > File Name: config.h
    > Author: Kevin
    > Created Time: 2019-03-05
    >Description:
 ************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

#define BG_5866_FIX                                 1
#define BG_6037_FIX                                 0

#define USE_GENERIC                                 1

#ifndef ALI_PROV_TEST
#define ALI_PROV_TEST                               0
#endif

#define CONFIG_GET_DCD                              1
#define CONFIG_ADD_APPKEY                           1
#define CONFIG_BIND_APPKEY                          1
#define CONFIG_SET_PUB                              1
#define CONFIG_ADD_SUB                              1
#define CONFIG_MISC                                 1

#define LEVEL_ERR                                   3
#define LEVEL_MSG                                   4
#define LEVEL_LOG                                   5

#ifndef PRINT_LEVEL
#define PRINT_LEVEL                                 LEVEL_MSG
#endif

#ifndef SRC_ROOT_DIR
#define SRC_ROOT_DIR                                "/home/zhfu/work/projs/provisioner/"
#endif

/* #define DISABLE_CS_OUTPUT */
#ifndef DEBUG_PRINT
#define DEBUG_PRINT                                 1
#endif

#ifndef DEBUG
#define DEBUG                                       1
#endif

#ifndef DEBUG_GUARD_TIMER_OFF
#define DEBUG_GUARD_TIMER_OFF 1
#endif

#if (DEBUG == 1)
#define PORT                                        "/dev/ttyACM0"
#define BR                                          115200
#define ENC_SOCK_FILE_PATH                          "/tmp/enc_sock"
#define UNENC_SOCK_FILE_PATH                        "/tmp/unenc_sock"
#define ENCRYPTED                                   1
#define UNENCRYPTED                                 0

#define SERIAL_PORT                                 PORT
#define BAUD_RATE                                   BR
#define SOCKET_PATH                                 ENC_SOCK_FILE_PATH
#define IS_SOCK_ENC                                 ENCRYPTED
#endif

#ifndef INPUT_FILENAME
/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_full.xml") */
/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_g3.xml") */
/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_50nodes.xml") */
/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_home.xml") */
/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_create.xml") */
/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_test.xml") */
/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_new_test.xml") */
/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_utest.xml") */

/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_full.json") */
/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_home.json") */
/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_test.json") */
#define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_newone.json")
/* #define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_50_nodes.json") */
#endif

#ifndef INPUT_TMPL_FILENAME
/* #define INPUT_TMPL_FILENAME                         (SRC_ROOT_DIR "config/template.xml") */
#define INPUT_TMPL_FILENAME                         (SRC_ROOT_DIR "config/template.json")
#endif

#ifndef OUTPUT_FILENAME
#define OUTPUT_FILENAME                             (SRC_ROOT_DIR "config/conf.out")
#endif

#ifndef CLIENT_ENCRYPTED_PATH
#define CLIENT_ENCRYPTED_PATH                       "/tmp/client_encrypted"
#endif

#ifndef CLIENT_UNENCRYPTED_PATH
#define CLIENT_UNENCRYPTED_PATH                     "/tmp/client_unencrypted"
#endif

#if (ALI_PROV_TEST == 1)
#define INPUT_FILENAME                              (SRC_ROOT_DIR "config/conf_vme.xml")
#endif
#endif

/*
 * NOTE: Make sure this value is NOT greater than the Max Prov Sessions
 * definition on the NCP target side
 */
#define MAX_PROV_SESSIONS 3
/* #define MAX_PROV_SESSIONS 2 */
/*
 * NOTE: Make sure this value is NOT greater than the Max Foundation Client Cmds
 * definition on the NCP target side
 */
#define MAX_CONCURRENT_CONFIG_NODES 6
/* #define MAX_CONCURRENT_CONFIG_NODES 2 */

#define SEND_TO_GENERIC_Q_LENGTH  10
#define SEND_TO_TBA_Q_LENGTH  10
#define SEND_TO_TBC_Q_LENGTH  6
#define SEND_TO_TBBL_Q_LENGTH 6

#if (DEBUG_PRINT == 1)
#define ACT_DEBUG   1
#define XML_DEBUG   1
#define CGP_DEBUG   1
#define TBBL_DEBUG  1
#define ACC_DEBUG   1
#endif
#define LOCK_TRACE  1
