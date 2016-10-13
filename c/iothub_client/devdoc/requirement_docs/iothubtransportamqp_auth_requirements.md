
# IoTHubTransportAMQP_Auth Requirements

 
## Overview

This module provides APIs for authenticating devices registered on the IoT Hub C client AMQP protocol.  
   
   
## Exposed API


```c
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

typedef void(*ON_AUTHENTICATION_STOP_COMPLETED)(DELETE_SAS_TOKEN_RESULT result, void* context);
typedef void(*ON_AUTHENTICATION_STATUS_CHANGED)(void* context, AUTHENTICATION_STATUS* old_status, AUTHENTICATION_STATUS* new_status);

typedef enum CREDENTIAL_TYPE_TAG 
{ 
	CREDENTIAL_TYPE_NONE,
	CREDENTIAL_TYPE_X509, 
	CREDENTIAL_TYPE_DEVICE_KEY, 
	CREDENTIAL_TYPE_DEVICE_SAS_TOKEN, 
} CREDENTIAL_TYPE; 

typedef struct AUTHENTICATION_STATE* AUTHENTICATION_HANDLE;


extern AUTHENTICATION_HANDLE authentication_create(const AUTHENTICATION_CONFIG* config);

extern void authentication_do_work(AUTHENTICATION_HANDLE authentication_handle);

extern int authentication_start(AUTHENTICATION_HANDLE authentication_handle, const CBS_HANDLE cbs_handle, ON_AUTHENTICATION_STATUS_CHANGED on_status_changed, const void* context);

extern int authentication_stop(AUTHENTICATION_HANDLE authentication_handle, ON_AUTHENTICATION_STOP_COMPLETED on_stop_completed, const void* context);

extern int authentication_get_credential_type(AUTHENTICATION_HANDLE authentication_handle, CREDENTIAL_TYPE* type);

extern int authentication_set_option(AUTHENTICATION_HANDLE authentication_handle, const char* name, const void* value);

extern int authentication_destroy(AUTHENTICATION_HANDLE authentication_handle);
```


### authentication_create

```c
AUTHENTICATION_HANDLE authentication_create(const AUTHENTICATION_CONFIG* config)
```

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_001: [**If parameter config, config->device_id or config->iot_hub_host_fqdn are NULL, authentication_create() shall fail and return NULL.**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_002: [**authentication_create() shall allocate memory for a new authenticate state structure AUTHENTICATION_STATE.**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_003: [**If malloc() fails, authentication_create() shall fail and return NULL.**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_004: [**authentication_create() shall set the initial status of AUTHENTICATION_STATE as AUTHENTICATION_STATUS_NONE.**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_006: [**authentication_create() shall save a copy of `device_config->deviceId` into the AUTHENTICATION_STATE instance.**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_007: [**If STRING_construct() fails to copy `device_config->deviceId`, authentication_create() shall fail and return NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_008: [**authentication_create() shall save a copy of `iot_hub_host_fqdn` into the AUTHENTICATION_STATE instance.**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_009: [**If STRING_clone() fails to copy `iot_hub_host_fqdn`, authentication_create() shall fail and return NULL**]**


#### DEVICE_SAS_TOKEN authentication

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_010: [**If `device_config->deviceSasToken` is not NULL, authentication_create() shall save a copy into the AUTHENTICATION_STATE instance.**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_011: [**If STRING_construct() fails to copy `device_config->deviceSasToken`, authentication_create() shall fail and return NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_012: [**If `device_config->deviceSasToken` is not NULL, authentication_create() shall set the credential type in the AUTHENTICATION_STATE as DEVICE_SAS_TOKEN.**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_014: [**If the credential type is DEVICE_SAS_TOKEN or DEVICE_KEY, authentication_create() shall set sasTokenKeyName in the AUTHENTICATION_STATE as a non-NULL empty string.**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_015: [**If STRING_new() fails and cannot create sasTokenKeyName, authentication_create() shall fail and return NULL**]**


#### DEVICE_KEY authentication

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_016: [**If `device_config->deviceKey` is not NULL, authentication_create() shall save a copy into the AUTHENTICATION_STATE instance.**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_017: [**If STRING_construct() fails to copy `device_config->deviceKey`, authentication_create() shall fail and return NULL**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_018: [**If `device_config->deviceKey` is not NULL, authentication_create() shall set the credential type in the AUTHENTICATION_STATE as DEVICE_KEY.**]**


#### Unknown authentication

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_019: [**If `device_config->deviceKey` and `device_config->deviceSasToken` are NULL, authentication_create() shall fail and return**]**


#### General

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_021: [**If any failure occurs, authentication_create() shall free any memory it allocated previously, including the AUTHENTICATION_STATE**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_022: [**If no failure occurs, authentication_create() shall return a reference to the AUTHENTICATION_STATE handle**]**


### authentication_do_work

```c
void authentication_do_work(AUTHENTICATION_HANDLE authentication_handle)
```

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_023: [**If authentication_state is NULL, authentication_authenticate() shall fail and return an error code**]**


#### DEVICE_KEY authentication

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_024: [**If the credential type is DEVICE_KEY, authentication_authenticate() shall create a SAS token and put it to CBS**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_026: [**The SAS token expiration time shall be calculated adding `sas_token_lifetime` to the current number of seconds since epoch time UTC**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_076: [**A STRING_HANDLE, referred to as `devices_path`, shall be created from the following parts: iot_hub_host_fqdn + "/devices/" + device_id**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_077: [**If `devices_path` failed to be created, authentication_authenticate() shall fail and return an error code**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_025: [**The SAS token shall be created using SASToken_Create(), passing the deviceKey, device_path, sasTokenKeyName and expiration time as arguments**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_027: [**If SASToken_Create() fails, authentication_authenticate() shall fail and return an error code**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_028: [**The SAS token shall be sent to CBS using cbs_put_token(), using `servicebus.windows.net:sastoken` as token type and `devices_path` as audience**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_029: [**If cbs_put_token() succeeds, authentication_authenticate() shall set the state status to AUTHENTICATION_STATUS_IN_PROGRESS**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_030: [**If cbs_put_token() succeeds, authentication_authenticate() shall set `current_sas_token_put_time` with the current time**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_031: [**authentication_authenticate() shall free the memory allocated for the new SAS token using STRING_delete()**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_032: [**If cbs_put_token() fails, authentication_authenticate() shall fail and return an error code**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_033: [**If cbs_put_token() succeeds, authentication_authenticate() shall return success code 0**]**


#### DEVICE_SAS_TOKEN authentication

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_034: [**If the credential type is DEVICE_SAS_TOKEN, authentication_authenticate() shall put the SAS token provided to CBS**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_078: [**A STRING_HANDLE, referred to as `devices_path`, shall be created from the following parts: iot_hub_host_fqdn + "/devices/" + device_id**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_079: [**If `devices_path` failed to be created, authentication_authenticate() shall fail and return an error code**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_035: [**The SAS token provided shall be sent to CBS using cbs_put_token(), using `servicebus.windows.net:sastoken` as token type and `devices_path` as audience**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_036: [**If cbs_put_token() fails, authentication_authenticate() shall fail and return an error code**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_037: [**If cbs_put_token() succeeds, authentication_authenticate() shall set the state status to AUTHENTICATION_STATUS_IN_PROGRESS**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_038: [**If cbs_put_token() succeeds, authentication_authenticate() shall set `current_sas_token_put_time` with the current time**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_040: [**If cbs_put_token() succeeds, authentication_authenticate() shall return success code 0**]**


##### cbs_put_token() callback (applicable to when authentication is done using DEVICE_KEY or DEVICE_SAS_TOKEN)

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_041: [**When cbs_put_token() calls back, if the result is CBS_OPERATION_RESULT_OK the state status shall be set to AUTHENTICATION_STATUS_AUTHENTICATED**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_042: [**When cbs_put_token() calls back, if the result is not CBS_OPERATION_RESULT_OK the state status shall be set to AUTHENTICATION_STATUS_FAILED**]**




### authentication_get_status

```c
AUTHENTICATION_STATUS authentication_get_status(AUTHENTICATION_HANDLE authentication_state)
```

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_044: [**If authentication_state is NULL, authentication_get_status() shall fail and return AUTHENTICATION_STATUS_NONE**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_045: [**If the credential type is DEVICE_KEY and current status is AUTHENTICATION_STATUS_IN_PROGRESS, authentication_get_status() shall check for authentication timeout**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_046: [**The authentication timeout shall be computed comparing the last time a SAS token was put (`current_sas_token_put_time`) to `cbs_request_timeout`**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_047: [**If authentication has timed out, authentication_get_status() shall set the status of the state to AUTHENTICATION_STATUS_FAILED_TIMEOUT**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_048: [**If the credential type is DEVICE_KEY and current status is AUTHENTICATION_STATUS_AUTHENTICATED, authentication_get_status() shall check if SAS token must be refreshed**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_049: [**The SAS token expiration shall be computed comparing its create time to `sas_token_refresh_time`**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_050: [**If the SAS token must be refreshed, authentication_get_status() shall set the status of the state to AUTHENTICATION_STATUS_REFRESH_REQUIRED**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_051: [**If the credential type is DEVICE_SAS_TOKEN and current status is AUTHENTICATION_STATUS_IN_PROGRESS, authentication_get_status() shall check for authentication timeout**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_052: [**The authentication timeout shall be computed comparing the last time the SAS token was put (`current_sas_token_put_time`) to `cbs_request_timeout`**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_053: [**If authentication has timed out, authentication_get_status() shall set the status of the state to AUTHENTICATION_STATUS_FAILED_TIMEOUT**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_054: [**After checks and updates, authentication_get_status() shall return the status saved on the AUTHENTICATION_STATE**]**


### authentication_get_credential_type

```c
int authentication_get_credential_type(AUTHENTICATION_HANDLE authentication_handle, CREDENTIAL_TYPE* type)
```

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_055: [**If authentication_state is NULL, authentication_get_credential_type() shall fail and return NONE**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_056: [**If authentication_state is not NULL, authentication_get_credential_type() shall return the type of the credential**]**


### authentication_reset

```c
int authentication_reset(AUTHENTICATION_HANDLE authentication_state)
```

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_061: [**If authentication_state is NULL, authentication_reset() shall fail and return an error code**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_062: [**If the credential type is X509, authentication_reset() shall set the status to AUTHENTICATION_STATUS_IDLE and return with success code 0**]**


The following apply if the credential type is DEVICE_KEY or DEVICE_SAS_TOKEN:

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_063: [**If the authentication_state status is AUTHENTICATION_STATUS_FAILED or AUTHENTICATION_STATUS_REFRESH_REQUIRED, authentication_reset() shall set the status to AUTHENTICATION_STATUS_IDLE**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_064: [**If the authentication_state status is AUTHENTICATION_STATUS_AUTHENTICATED or AUTHENTICATION_STATUS_IN_PROGRESS, authentication_reset() delete the previous token using cbs_delete_token()**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_065: [**If cbs_delete_token fails, authentication_reset() shall fail and return an error code**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_066: [**If cbs_delete_token calls back with result different than CBS_OPERATION_RESULT_OK, authentication_reset() shall set the state status to AUTHENTICATION_STATUS_FAILED**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_067: [**If no error occurs, authentication_reset() shall return 0**]**


### authentication_destroy

```c
int authentication_destroy(AUTHENTICATION_HANDLE authentication_state)
```

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_068: [**If authentication_state is NULL, authentication_destroy() shall fail and return**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_069: [**authentication_destroy() shall destroy the AUTHENTICATION_STATE->device_id using STRING_delete()**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_070: [**authentication_destroy() shall destroy the AUTHENTICATION_STATE->iot_hub_host_fqdn using STRING_delete()**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_071: [**If the credential type is DEVICE_KEY, authentication_destroy() shall destroy `deviceKey` in AUTHENTICATION_STATE using STRING_delete()**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_072: [**If the credential type is DEVICE_KEY, authentication_destroy() shall destroy `sasTokenKeyName` in AUTHENTICATION_STATE using STRING_delete()**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_073: [**If the credential type is DEVICE_SAS_TOKEN, authentication_destroy() shall destroy `deviceSasToken` in AUTHENTICATION_STATE using STRING_delete()**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_074: [**If the credential type is DEVICE_SAS_TOKEN, authentication_destroy() shall destroy `sasTokenKeyName` in AUTHENTICATION_STATE using STRING_delete()**]**

**SRS_IOTHUBTRANSPORTAMQP_AUTH_09_075: [**authentication_destroy() shall free the AUTHENTICATION_STATE**]**
