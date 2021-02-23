/*************************************************************************
    > File Name: server_app.h
    > Author: Kevin
    > Created Time: 2018-09-16
    >Description: 
 ************************************************************************/

#ifndef SERVER_APP_H
#define SERVER_APP_H

#define LOG_MODULE_PRESENT	1

#if (!LOG_MODULE_PRESENT)
// define all macros in log module to null operation to avoid compiling errors
#define LOG_ASSERT(x)
#define LOG(...)
#define LOGN()
#define UINT8_ARRAY_DUMP(array_base, array_size)
#define LOG_DIRECT_ERR(_prefix_, ...)
#define LOGE(_prefix_, ...)
#define LOGW(_prefix_, ...)
#define LOGI(_prefix_, ...)
#define LOGD(_prefix_, ...)
#define LOGV(_prefix_, ...)
#define ERROR_ADDRESSING()
#define INIT_LOG()
#define EVT_LOG_C(_evt_name_, _attached_, ...)
#define EVT_LOG_I(_evt_name_, _attached_, ...)
#define EVT_LOG_V(_evt_name_, _attached_, ...)
#define SE_CALL(x)				\
		do{							\
			x;						\
		}while(0)
#define GENERAL_ERR_CHECK(x)
#endif

void AppHandler(void);

#endif
