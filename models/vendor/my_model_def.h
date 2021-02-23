#ifndef _MY_MODEL_DEF_H_
#define _MY_MODEL_DEF_H_

#define PROV_LOCALLY

#define PRIMARY_ELEMENT									0
#define MY_VENDOR_ID										0x1111

#define MY_MODEL_SERVER_ID							0x1111
#define MY_MODEL_CLIENT_ID							0x2222

#define TEMP_DATA_LENGTH								4
#define UPDATE_INTERVAL_LENGTH					1
#define UNIT_DATA_LENGTH								1

#define NUMBER_OF_OPCODES								10

#define ACK_REQ													(0x1)
#define STATUS_UPDATE_REQ								(0x2)

#define INDEX_OF(x)											((x) - 1)

typedef enum {
	temperature_get = 0x1,
	temperature_status,
	unit_get,
	unit_set,
	unit_set_unack,
	unit_status,
	update_interval_get,
	update_interval_set,
	update_interval_set_unack,
	update_interval_status
} my_msg_t;

typedef enum {
	celsius = 0x1,
	fahrenheit
} unit_t;

typedef struct {
	uint16 elem_index;
	uint16 vendor_id;
	uint16 model_id;
	uint8 publish; // publish - 1, not - 0
	uint8 opcodes_len;
	uint8 opcodes_data[NUMBER_OF_OPCODES];
} my_model_t;
#endif
