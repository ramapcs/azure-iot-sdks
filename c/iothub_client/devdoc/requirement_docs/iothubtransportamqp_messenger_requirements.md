# IoTHubTransportAMQP_Messenger Requirements

 
## Overview

This module provides an abstraction for the IoTHubTransportAMQP to send events and receive messages.  
   
   
## Exposed API

```c
	typedef enum MESSENGER_ERROR_TAG
	{
		MESSENGER_ERROR_CANNOT_CREATE_RECEIVER,
		MESSENGER_ERROR_RECEIVER_CLOSED_UNEXPECTEDLY
		MESSENGER_ERROR_SENDER_CLOSED_UNEXPECTEDLY,
	} MESSENGER_ERROR;

	typedef enum MESSENGER_SEND_RESULT_TAG
	{
		MESSENGER_SEND_RESULT_SUCCESS,
		MESSENGER_SEND_RESULT_ERROR
	} MESSENGER_SEND_RESULT;

	typedef void (*ON_MESSENGER_ERROR_CALLBACK)(MESSENGER_ERROR code, void* context);
	typedef void (*ON_MESSAGE_SEND_COMPLETE)(MESSENGER_SEND_RESULT send_result, void* context);
	typedef void(*ON_MESSAGE_RECEIVED)(IOTHUB_MESSAGE_HANDLE message, void* context);

	typedef struct MESSENGER_CONFIG_TAG
	{
		char* device_id;
		
		char* iothub_host_fqdn;
		
		PDLIST_ENTRY wait_to_send_list;
		
		ON_MESSAGE_SEND_COMPLETE on_event_send_completed_callback;

		void* on_event_send_completed_context;

		ON_MESSENGER_ERROR_CALLBACK on_error_callback;
		
		void* on_error_context;
	} MESSENGER_CONFIG;

	typedef struct MESSENGER_STATE* MESSENGER_HANDLE;

	extern MESSENGER_HANDLE messenger_create(const MESSENGER_CONFIG* messenger_config);

	extern int messenger_subscribe_for_messages(MESSENGER_HANDLE messenger_handle, ON_MESSAGE_RECEIVED on_message_received_callback, void* context);

	extern int messenger_unsubscribe_for_messages(MESSENGER_HANDLE messenger_handle);

	extern int messenger_start(MESSENGER_HANDLE messenger_handle, SESSION_HANDLE session_handle); 

	extern int messenger_stop(MESSENGER_HANDLE messenger_handle);

	extern void messenger_do_work(MESSENGER_HANDLE messenger_handle);

	extern void messenger_destroy(MESSENGER_HANDLE messenger_handle);
```


## messenger_create

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If parameter `messenger_config` is NULL, messenger_create() shall return NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `messenger_config->device_id` is NULL, messenger_create() shall return NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `messenger_config->iothub_host_fqdn` is NULL, messenger_create() shall return NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `messenger_config->wait_to_send_list` is NULL, messenger_create() shall return NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `messenger_config->on_event_send_completed_callback` is NULL, messenger_create() shall return NULL**]**


**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**messenger_create() shall allocate memory for the messenger state**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If malloc() fails, messenger_create() shall fail and return NULL**]**


**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**messenger_create() shall save a copy of `messenger_config->device_id` into the messenger state**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If STRING_construct() fails, messenger_create() shall fail and return NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**messenger_create() shall save a copy of `messenger_config->iothub_host_fqdn` into the messenger state**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If STRING_construct() fails, messenger_create() shall fail and return NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**The pointer `messenger_config->wait_to_send_list` shall be saved into `state->wait_to_send_list`**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**The pointer `messenger_config->on_event_send_completed_callback` shall be saved into `state->on_event_send_completed_callback`**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**The pointer `messenger_config->on_event_send_completed_context` shall be saved into `state->on_event_send_completed_context`**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If no failures occurr, messenger_create() shall return a handle to the messenger state**]**


## messenger_subscribe_for_messages

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `messenger_handle` is NULL, messenger_subscribe_for_messages() shall fail and return __LINE__**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `state->receive_messages` is already true, messenger_subscribe_for_messages() shall fail and return __LINE__**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `on_message_received_callback` is NULL, messenger_subscribe_for_messages() shall fail and return __LINE__**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`on_message_received_callback` shall be saved on `state->on_message_received_callback`**]**
 
**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`context` shall be saved on `state->on_message_received_context`**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**messenger_subscribe_for_messages() shall set `state->receive_messages` to true**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If no failures occurr, messenger_subscribe_for_messages() shall return 0**]**


## messenger_unsubscribe_for_messages

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `messenger_handle` is NULL, messenger_unsubscribe_for_messages() shall fail and return __LINE__**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `state->receive_messages` is already false, messenger_unsubscribe_for_messages() shall fail and return __LINE__**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**messenger_unsubscribe_for_messages() shall set `state->receive_messages` to false**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**messenger_unsubscribe_for_messages() shall set `state->on_message_received_callback` to NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**messenger_unsubscribe_for_messages() shall set `state->on_message_received_context` to NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If no failures occurr, messenger_unsubscribe_for_messages() shall return 0**]**


## messenger_start

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `messenger_handle` is NULL, messenger_start() shall fail and return __LINE__**]**




## messenger_stop

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `messenger_handle` is NULL, messenger_stop() shall fail and return __LINE__**]**




## messenger_do_work

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `messenger_handle` is NULL, messenger_do_work() shall fail and return**]**


### Create a message receiver

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `state->receive_messages` is true and `state->message_receiver` is NULL, a message_receiver shall be created**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**A parameter, named `devices_path`, shall be created concatenating `state->iothub_host_fqdn`, "/devices/" and `state->device_id`**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `devices_path` fails to be created, messenger_do_work() shall fail and return**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**A parameter, named `message_receive_address`, shall be created concatenating "amqps://", `devices_path` and "/messages/devicebound"**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `message_receive_address` fails to be created, messenger_do_work() shall fail and return**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**A `target` parameter shall be created with messaging_create_target() using an unique string label per AMQP session**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**A `source` parameter shall be created with messaging_create_source() using `message_receive_address`**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->receiver_link` shall be set using link_create(), passing `state->session_handle`, `link_name`, "role_receiver", `source` and `target` as parameters**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If link_create() fails, messenger_do_work() shall fail and return**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->receiver_link` settle mode shall be set to "receiver_settle_mode_first" using link_set_rcv_settle_mode(), **]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If link_set_rcv_settle_mode() fails, messenger_do_work() shall fail and return**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->receiver_link` maximum message size shall be set to 65536 using link_set_max_message_size()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If link_set_max_message_size() fails, it shall be logged and ignored.**]**


**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->receiver_link` should have a property "com.microsoft:client-version" set as `CLIENT_DEVICE_TYPE_PREFIX/IOTHUB_SDK_VERSION`**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `state->receiver_link` fails to have the client type and version set on its properties, the failure shall be ignored**]**


**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->message_receiver` shall be created using messagereceiver_create(), passing the `state->receiver_link` and a state change callback**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If messagereceiver_create() fails, messenger_do_work() shall fail and return**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->message_receiver` shall be opened using messagereceiver_open(), passing a callback to receive C2D messages**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If messagereceiver_open() fails, messenger_do_work() shall fail and return**]**

The following requirements shall apply before messenger_do_work() returns:
**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If messenger_do_work() fails, it shall invoke `state->on_error_callback`, if provided, with error code MESSENGER_ERROR_CANNOT_CREATE_RECEIVER**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**Before messenger_do_work() returns, it shall destroy all the temporary memory it allocated**]**


### Destroy the message receiver

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `state->receive_messages` is false and `state->message_receiver` is not NULL, it shall be destroyed**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->message_receiver` shall be closed using messagereceiver_close()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If messagereceiver_close() fails, it shall be logged and ignored**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->message_receiver` shall be destroyed using messagereceiver_destroy()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->message_receiver` shall be set to NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->receiver_link` shall be destroyed using link_destroy()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->receiver_link` shall be set to NULL**]**


### Send pending events
// TODO: review this. There should be NO callbacks fps needed from upper layer (remove ON_MESSAGE_SEND_COMPLETE).
**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**messenger_do_work() shall move each event to be sent from `state->wait_to_send_list` to `state->in_progress_list`**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**A MESSAGE_HANDLE shall be obtained out of the event's IOTHUB_MESSAGE_HANDLE instance by using message_create_from_iothub_message()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If message_create_from_iothub_message() fails, `state->on_event_send_completed_callback` shall be invoked with result MESSENGER_SEND_RESULT_ERROR**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If message_create_from_iothub_message() fails, messender_do_work() shall fail and return**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**The MESSAGE_HANDLE shall be submitted for sending using messagesender_send(), passing a callback to track when the event has been sent**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If messagesender_send() fails, the event shall be rolled back from `state->in_progress_list` to `state->wait_to_send_list`**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**The MESSAGE_HANDLE shall be destroyed using message_destroy().**]**


When messagesender_send() calls back,
**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If no failure occurs, `state->on_event_send_completed_callback` shall be invoked with result MESSENGER_SEND_RESULT_SUCCESS**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If a failure occurred, `state->on_event_send_completed_callback` shall be invoked with result MESSENGER_SEND_RESULT_ERROR**]**



## messenger_destroy

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If `messenger_handle` is NULL, messenger_destroy() shall fail and return**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If the messenger is opened, `state->message_sender` shall be stopped using messagesender_stop()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**If the messenger is opened, `state->message_sender` shall be destroyed using messagesender_destroy()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->message_receiver` shall be stopped using messagereceiver_stop()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->message_receiver` shall be destroyed using messagereceiver_destroy()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->sender_link` shall be destroyed using link_destroy()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->receiver_link` shall be destroyed using link_destroy()**]**

Note: when events are moved from wait_to_send_list to in_progress_list, they are moved from beginning (oldest element) to end (newest).
Due to the logic used by messenger_do_work(), all elements of in_progress_list are always older than any current element on in_progress_list.
In that case, when they are rolled back, they need to go to the beginning of the wait_to_send_list. 
**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**All elements of `state->in_progress_list` shall be moved to the beginning of `state->wait_to_send_list`**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->iothub_host_fqdn` shall be destroyed using STRING_delete()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**`state->device_id` shall be destroyed using STRING_delete()**]**

**SRS_IOTHUBTRANSPORTAMQP_MESSENGER_09_001: [**messenger_destroy() shall destroy the messenger state with free()**]**