// Copyright (c) Microsoft. All rights reserved. 
// Licensed under the MIT license. See LICENSE file in the project root for full license information. 

#ifndef IOTHUBTRANSPORTAMQP_AUTH_H 
#define IOTHUBTRANSPORTAMQP_AUTH_H 

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdint.h>
#include "azure_c_shared_utility/strings.h" 
#include "azure_c_shared_utility/sastoken.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_uamqp_c/cbs.h"
#include "iothub_transport_ll.h" 

#ifdef __cplusplus 
extern "C" 
{ 
#endif 

typedef struct AUTHENTICATION_CONFIG_TAG
{
	const char* device_id;
	const char* device_key;
	const char* device_sas_token;
	const char* iot_hub_host_fqdn;

} AUTHENTICATION_CONFIG;

typedef enum AUTHENTICATION_STATUS_TAG 
{
	AUTHENTICATION_STATUS_NONE,
	AUTHENTICATION_STATUS_IDLE,
	AUTHENTICATION_STATUS_STARTED,
	AUTHENTICATION_STATUS_AUTHENTICATING,
	AUTHENTICATION_STATUS_AUTHENTICATED,
	AUTHENTICATION_STATUS_REFRESHING,
	AUTHENTICATION_STATUS_DEAUTHENTICATING,
	AUTHENTICATION_STATUS_FAILED, 
	AUTHENTICATION_STATUS_FAILED_TIMEOUT
} AUTHENTICATION_STATUS; 
 
typedef enum DELETE_SAS_TOKEN_RESULT_TAG
{
	DELETE_SAS_TOKEN_RESULT_SUCCESS,
	DELETE_SAS_TOKEN_RESULT_ERROR
} DELETE_SAS_TOKEN_RESULT;

typedef enum CREDENTIAL_TYPE_TAG 
{ 
	CREDENTIAL_TYPE_NONE,
	CREDENTIAL_TYPE_X509, 
	CREDENTIAL_TYPE_DEVICE_KEY, 
	CREDENTIAL_TYPE_DEVICE_SAS_TOKEN, 
} CREDENTIAL_TYPE; 
 
typedef struct X509_CREDENTIAL_TAG 
{ 
	const char* x509certificate; 
	const char* x509privatekey; 
} X509_CREDENTIAL; 
 
typedef union CREDENTIAL_DATA_TAG 
{ 
	// Key associated to the device to be used. 
	STRING_HANDLE deviceKey; 
 
	// SAS associated to the device to be used. 
	STRING_HANDLE deviceSasToken; 
 
	// CREDENTIAL_TYPE_X509  
	X509_CREDENTIAL x509credential; 
} CREDENTIAL_DATA; 
 
typedef struct AMQP_TRANSPORT_CREDENTIAL_TAG 
{ 
	CREDENTIAL_TYPE type; 
	CREDENTIAL_DATA data; 
} DEVICE_CREDENTIAL; 

typedef void(*ON_AUTHENTICATION_STOP_COMPLETED)(DELETE_SAS_TOKEN_RESULT result, void* context);
typedef void(*ON_AUTHENTICATION_STATUS_CHANGED)(void* context, AUTHENTICATION_STATUS* old_status, AUTHENTICATION_STATUS* new_status);

typedef struct AUTHENTICATION_STATE* AUTHENTICATION_HANDLE;
 
/** @brief Creates a state holder for all authentication-related information and connections. 
* 
*   @returns an instance of the AUTHENTICATION_HANDLE if succeeds, NULL if any failure occurs. 
*/ 
extern AUTHENTICATION_HANDLE authentication_create(const AUTHENTICATION_CONFIG* config);

/** @brief Gets the type of the credential the state was set to use.
*
*   @returns If succeeds, returns 0 and an CREDENTIAL_TYPE indicating the credential type, or non-zero if it fails.
*/
extern int authentication_get_credential_type(AUTHENTICATION_HANDLE authentication_handle, CREDENTIAL_TYPE* type);

/** @brief Prepares the state to authenticate a device. 
* 
*	@details Causes the state to store the CBS_HANDLE to be used, and the callbacks to be fired on state changes.
* 
*   @returns 0 if it succeeds, non-zero if it fails. 
*/ 
extern int authentication_start(AUTHENTICATION_HANDLE authentication_handle, const CBS_HANDLE cbs_handle, ON_AUTHENTICATION_STATUS_CHANGED on_status_changed, const void* context);

/** @brief De-authenticates without destroying the authentication state. 
* 
*   @details A SAS token delete command is sent to the CBS if needed, and the CBS_HANDLE is then discarded by the state. 
* 
*   @returns 0 if it succeeds, non-zero if it fails. 
*/ 
extern int authentication_stop(AUTHENTICATION_HANDLE authentication_handle, ON_AUTHENTICATION_STOP_COMPLETED on_stop_completed, const void* context);

/** @brief Causes the authentication state to authenticate, refresh SAS token, compute timeouts, trigger saved callbacks.
*
*   @details This must be called frequently for the authentication state to process properly.
*
*   @returns 0 if it succeeds, non-zero if it fails.
*/
extern int authentication_do_work(AUTHENTICATION_HANDLE authentication_handle);

/** @brief Sets options on the authentication state.
*
*   @details Options supported: 
*
*   @returns 0 if it succeeds, non-zero if it fails.
*/
extern int authentication_set_option(AUTHENTICATION_HANDLE authentication_handle, const char* name, const void* value);

/** @brief De-authenticates the device and destroy the state instance. 
* 
*   @details Closes the subscription to cbs if in use, destroys the cbs instance if it is the last device registered. 
*            No action is taken if certificate-based authentication if used. 
* 
*   @returns Nothing. 
*/ 
extern void authentication_destroy(AUTHENTICATION_HANDLE authentication_handle); 
 
#ifdef __cplusplus 
} 
#endif 
 
#endif /*IOTHUBTRANSPORTAMQP_AUTH_H*/ 