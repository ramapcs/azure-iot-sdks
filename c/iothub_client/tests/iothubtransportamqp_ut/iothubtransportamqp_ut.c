// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>

void* real_malloc(size_t size)
{
	return malloc(size);
}

void real_free(void* ptr)
{
	free(ptr);
}

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "testrunnerswitcher.h"
#include "azure_c_shared_utility/macro_utils.h"
#include "umock_c.h"
#include "umocktypes_charptr.h"
#include "umocktypes_stdint.h"
#include "umock_c_negative_tests.h"
#include "umocktypes.h"
#include "umocktypes_c.h"

#define ENABLE_MOCKS
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "azure_c_shared_utility/doublylinkedlist.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/urlencode.h"
#include "azure_c_shared_utility/tlsio.h"
#include "azure_c_shared_utility/vector.h"

#include "azure_uamqp_c/cbs.h"
#include "azure_uamqp_c/link.h"
#include "azure_uamqp_c/message.h"
#include "azure_uamqp_c/amqpvalue.h"
#include "azure_uamqp_c/message_receiver.h"
#include "azure_uamqp_c/message_sender.h"
#include "azure_uamqp_c/messaging.h"
#include "azure_uamqp_c/sasl_mssbcbs.h"
#include "azure_uamqp_c/saslclientio.h"

#include "uamqp_messaging.h"
#include "iothub_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_client_private.h"
#include "iothubtransportamqp_auth.h"
#include "iothub_client_version.h"
#undef ENABLE_MOCKS

#include "iothubtransportamqp.h"

static TEST_MUTEX_HANDLE g_testByTest;
static TEST_MUTEX_HANDLE g_dllByDll;

// Control parameters
#define INDEFINITE_TIME ((time_t)-1)
#define TEST_DEVICE_ID "deviceid"
#define TEST_DEVICE_KEY "devicekey"
#define TEST_DEVICE_SAS_TOKEN "deviceSas"
#define TEST_IOT_HUB_NAME "servername"
#define TEST_IOT_HUB_SUFFIX "domainname"
#define TEST_IOT_HUB_PORT 5671
#define TEST_PROT_GW_HOSTNAME_NULL NULL
#define TEST_PROT_GW_HOSTNAME "gw"

#define TEST_STRING_HANDLE (STRING_HANDLE)0x101
#define TEST_WAIT_TO_SEND_LIST ((PDLIST_ENTRY)0x102)
#define TEST_IOTHUB_FQDN_STRING_HANDLE (STRING_HANDLE)0x103
#define TEST_IOTHUB_FQDN_CHAR_PTR (char*)0x104
#define TEST_PROT_GW_FQDN_STRING_HANDLE (STRING_HANDLE)0x105
#define TEST_VECTOR_HANDLE (VECTOR_HANDLE)0x106
#define TEST_IOTHUB_CLIENT_LL_HANDLE (IOTHUB_CLIENT_LL_HANDLE)0x107

#define TEST_DEVICE_STATE_HANDLE (IOTHUB_DEVICE_HANDLE*)0x108
#define TEST_DEVICE_ID_STRING_HANDLE (STRING_HANDLE)0x109
#define TEST_DEVICES_PATH_STRING_HANDLE (STRING_HANDLE)0x111
#define TEST_DEVICES_PATH_CHAR_PTR (char*)0x112
#define TEST_TARGET_ADDRESS_STRING_HANDLE (STRING_HANDLE)0x113
#define TEST_MSG_RCV_ADDRESS_STRING_HANDLE (STRING_HANDLE)0x114
#define TEST_AUTHENTICATION_STATE_HANDLE (AUTHENTICATION_STATE_HANDLE)0x115

static int saved_malloc_results_count = 0;
static void* saved_malloc_results[10];

DEFINE_ENUM_STRINGS(UMOCK_C_ERROR_CODE, UMOCK_C_ERROR_CODE_VALUES)

static void on_umock_c_error(UMOCK_C_ERROR_CODE error_code)
{
    char temp_str[256];
    (void)snprintf(temp_str, sizeof(temp_str), "umock_c reported error :%s", ENUM_TO_STRING(UMOCK_C_ERROR_CODE, error_code));
    ASSERT_FAIL(temp_str);
}

MOCKABLE_FUNCTION(, IOTHUBMESSAGE_DISPOSITION_RESULT, IoTHubClient_LL_MessageCallback, IOTHUB_CLIENT_LL_HANDLE, handle, IOTHUB_MESSAGE_HANDLE, message);

static int STRING_HANDLE_Compare(STRING_HANDLE a, STRING_HANDLE b)
{
	return ((a == b) ? 0 : 1);
}

static void STRING_HANDLE_ToString(char* dest, size_t bufferSize, STRING_HANDLE a)
{
	(void)snprintf(dest, bufferSize, "[STRING_HANDLE]%d", (uint32_t)a);
}


// Function hooks

static void* TEST_malloc(size_t size)
{
	void* result = real_malloc(size);
	saved_malloc_results[saved_malloc_results_count++] = result;
	return result;
}


// Utilities

static void test_reset()
{
	saved_malloc_results_count = 0;
	umock_c_reset_all_calls();
}

static void test_cleanup()
{
	int i;
	for (i = 0; i < saved_malloc_results_count; i++)
	{
		real_free(saved_malloc_results[i]);
	}
	saved_malloc_results_count = 0;
}

static IOTHUB_CLIENT_CONFIG* create_client_config(IOTHUB_CLIENT_TRANSPORT_PROVIDER transport_provider, bool use_iothub_fqdn)
{
	static IOTHUB_CLIENT_CONFIG client_config;
	client_config.protocol = transport_provider;
	client_config.deviceId = TEST_DEVICE_ID;
	client_config.deviceKey = TEST_DEVICE_KEY;
	client_config.deviceSasToken = TEST_DEVICE_SAS_TOKEN;
	client_config.iotHubName = TEST_IOT_HUB_NAME;
	client_config.iotHubSuffix = TEST_IOT_HUB_SUFFIX;

	if (use_iothub_fqdn)
	{
		client_config.protocolGatewayHostName = NULL;
	}
	else
	{
		client_config.protocolGatewayHostName = TEST_PROT_GW_HOSTNAME;
	}

	return &client_config;
}

static IOTHUBTRANSPORT_CONFIG* create_transport_config(IOTHUB_CLIENT_TRANSPORT_PROVIDER transport_provider, bool use_iothub_fqdn)
{
	static IOTHUBTRANSPORT_CONFIG config;
	config.upperConfig = create_client_config(transport_provider, use_iothub_fqdn);
	config.waitingToSend = TEST_WAIT_TO_SEND_LIST;
	return &config;
}

static void set_exp_calls_for_IoTHubTransportAMQP_Create(IOTHUBTRANSPORT_CONFIG* config)
{
	EXPECTED_CALL(malloc(0)).IgnoreArgument_size();

	if (config->upperConfig->protocolGatewayHostName != NULL)
	{
		STRICT_EXPECTED_CALL(STRING_construct(TEST_PROT_GW_HOSTNAME)).SetReturn(TEST_PROT_GW_FQDN_STRING_HANDLE);
	}
	else
	{
		STRICT_EXPECTED_CALL(malloc(strlen(config->upperConfig->iotHubName) + strlen(config->upperConfig->iotHubSuffix) + 2));
		EXPECTED_CALL(STRING_construct(0)).SetReturn(TEST_IOTHUB_FQDN_STRING_HANDLE);
	}

	STRICT_EXPECTED_CALL(VECTOR_create(sizeof(IOTHUB_DEVICE_HANDLE)));
}

static TRANSPORT_LL_HANDLE create_valid_transport_handle(IOTHUBTRANSPORT_CONFIG* config)
{
	test_reset();
	set_exp_calls_for_IoTHubTransportAMQP_Create(config);
	TRANSPORT_LL_HANDLE handle = config->upperConfig->protocol()->IoTHubTransport_Create(config);
	return handle;
}


BEGIN_TEST_SUITE(iothubtransportamqp_ut)

TEST_SUITE_INITIALIZE(TestClassInitialize)
{
    TEST_INITIALIZE_MEMORY_DEBUG(g_dllByDll);
    g_testByTest = TEST_MUTEX_CREATE();
    ASSERT_IS_NOT_NULL(g_testByTest);

    umock_c_init(on_umock_c_error);

	int result = umocktypes_charptr_register_types();
	ASSERT_ARE_EQUAL(int, 0, result);
	result = umocktypes_stdint_register_types();
	ASSERT_ARE_EQUAL(int, 0, result);

	REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_MESSAGE_HANDLE, void*);
	REGISTER_UMOCK_ALIAS_TYPE(IOTHUBMESSAGE_CONTENT_TYPE, int);
	REGISTER_UMOCK_ALIAS_TYPE(IOTHUB_MESSAGE_RESULT, void*);
	REGISTER_UMOCK_ALIAS_TYPE(MESSAGE_HANDLE, void*);
	REGISTER_UMOCK_ALIAS_TYPE(PROPERTIES_HANDLE, void*);
	REGISTER_UMOCK_ALIAS_TYPE(BINARY_DATA, void*);
	REGISTER_UMOCK_ALIAS_TYPE(MAP_HANDLE, void*);
	REGISTER_UMOCK_ALIAS_TYPE(AMQP_VALUE, void*);
	REGISTER_UMOCK_ALIAS_TYPE(MAP_RESULT, int);
	REGISTER_UMOCK_ALIAS_TYPE(AMQP_TYPE, int);
	REGISTER_UMOCK_ALIAS_TYPE(STRING_HANDLE, void*);
	REGISTER_UMOCK_ALIAS_TYPE(VECTOR_HANDLE, void*);


	REGISTER_GLOBAL_MOCK_HOOK(malloc, TEST_malloc);
	REGISTER_GLOBAL_MOCK_FAIL_RETURN(malloc, NULL);

	REGISTER_GLOBAL_MOCK_RETURN(STRING_construct, TEST_STRING_HANDLE);
	REGISTER_GLOBAL_MOCK_FAIL_RETURN(STRING_construct, NULL);

	REGISTER_GLOBAL_MOCK_RETURN(VECTOR_create, TEST_VECTOR_HANDLE);
	REGISTER_GLOBAL_MOCK_FAIL_RETURN(VECTOR_create, NULL);
}

TEST_SUITE_CLEANUP(TestClassCleanup)
{
    umock_c_deinit();

    TEST_MUTEX_DESTROY(g_testByTest);
    TEST_DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_FUNCTION_INITIALIZE(TestMethodInitialize)
{
    if (TEST_MUTEX_ACQUIRE(g_testByTest))
    {
        ASSERT_FAIL("our mutex is ABANDONED. Failure in test framework");
    }

    umock_c_reset_all_calls();
}

TEST_FUNCTION_CLEANUP(TestMethodCleanup)
{
    TEST_MUTEX_RELEASE(g_testByTest);
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_09_005: [If parameter config or config->upperConfig are NULL then IoTHubTransportAMQP_Create shall fail and return NULL.]
TEST_FUNCTION(AMQP_Create_NULL_config)
{
    // arrange
	test_reset();
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();

    // act
	TRANSPORT_LL_HANDLE tr_hdl = provider->IoTHubTransport_Create(NULL);

    // assert
	ASSERT_IS_NULL_WITH_MSG(tr_hdl, "Expected NULL transport handle");

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_09_005: [If parameter config or config->upperConfig are NULL then IoTHubTransportAMQP_Create shall fail and return NULL.]
TEST_FUNCTION(AMQP_Create_NULL_upperConfig)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();
	IOTHUBTRANSPORT_CONFIG config;
	config.upperConfig = NULL;

	test_reset();

	// act
	TRANSPORT_LL_HANDLE tr_hdl = provider->IoTHubTransport_Create(&config);

	// assert
	ASSERT_IS_NULL(tr_hdl);

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_09_006: [IoTHubTransportAMQP_Create shall fail and return NULL if any fields of the config->upperConfig structure are NULL.]
TEST_FUNCTION(AMQP_Create_upperConfig_protocol_NULL)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();
	IOTHUBTRANSPORT_CONFIG* config = create_transport_config(NULL, true);

	test_reset();

	// act
	TRANSPORT_LL_HANDLE handle = provider->IoTHubTransport_Create(config);

	// assert
	ASSERT_IS_NULL(handle);

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_09_006: [IoTHubTransportAMQP_Create shall fail and return NULL if any fields of the config->upperConfig structure are NULL.]
TEST_FUNCTION(AMQP_Create_upperConfig_iotHubName_NULL)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();

	IOTHUB_CLIENT_CONFIG client_config;
	client_config.protocol = AMQP_Protocol;
	client_config.deviceId = TEST_DEVICE_ID;
	client_config.deviceKey = TEST_DEVICE_KEY;
	client_config.deviceSasToken = TEST_DEVICE_SAS_TOKEN;
	client_config.iotHubName = NULL;
	client_config.iotHubSuffix = TEST_IOT_HUB_SUFFIX;
	client_config.protocolGatewayHostName = TEST_PROT_GW_HOSTNAME;

	IOTHUBTRANSPORT_CONFIG config;
	config.upperConfig = &client_config;
	config.waitingToSend = TEST_WAIT_TO_SEND_LIST;

	test_reset();

	// act
	TRANSPORT_LL_HANDLE handle = provider->IoTHubTransport_Create(&config);

	// assert
	ASSERT_IS_NULL(handle);

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_09_006: [IoTHubTransportAMQP_Create shall fail and return NULL if any fields of the config->upperConfig structure are NULL.]
TEST_FUNCTION(AMQP_Create_upperConfig_iotHubSuffix_NULL)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();

	IOTHUB_CLIENT_CONFIG client_config;
	client_config.protocol = AMQP_Protocol;
	client_config.deviceId = TEST_DEVICE_ID;
	client_config.deviceKey = TEST_DEVICE_KEY;
	client_config.deviceSasToken = TEST_DEVICE_SAS_TOKEN;
	client_config.iotHubName = TEST_IOT_HUB_NAME;
	client_config.iotHubSuffix = NULL;
	client_config.protocolGatewayHostName = TEST_PROT_GW_HOSTNAME;

	IOTHUBTRANSPORT_CONFIG config;
	config.upperConfig = &client_config;
	config.waitingToSend = TEST_WAIT_TO_SEND_LIST;

	test_reset();

	// act
	TRANSPORT_LL_HANDLE handle = provider->IoTHubTransport_Create(&config);

	// assert
	ASSERT_IS_NULL(handle);

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_09_134: [IoTHubTransportAMQP_Create shall fail and return NULL if the combined length of config->iotHubName and config->iotHubSuffix exceeds 254 bytes (RFC1035)]
TEST_FUNCTION(AMQP_Create_config_hubFqdn_too_long)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();

	IOTHUB_CLIENT_CONFIG client_config;
	client_config.protocol = AMQP_Protocol;
	client_config.deviceId = TEST_DEVICE_ID;
	client_config.deviceKey = TEST_DEVICE_KEY;
	client_config.deviceSasToken = TEST_DEVICE_SAS_TOKEN;
	client_config.iotHubName = "0123456789012345678901234567890123456789";
	client_config.iotHubSuffix = "01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234";
	client_config.protocolGatewayHostName = TEST_PROT_GW_HOSTNAME;

	IOTHUBTRANSPORT_CONFIG config;
	config.upperConfig = &client_config;
	config.waitingToSend = TEST_WAIT_TO_SEND_LIST;

	test_reset();

	// act
	TRANSPORT_LL_HANDLE handle = provider->IoTHubTransport_Create(&config);

	// assert
	ASSERT_IS_NULL(handle);

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_09_009: [IoTHubTransportAMQP_Create shall fail and return NULL if memory allocation of the transport's internal state structure fails.]
TEST_FUNCTION(AMQP_Create_malloc_TRANSPORT_LL_HANDLE_fails)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();
	IOTHUBTRANSPORT_CONFIG* config = create_transport_config(AMQP_Protocol, true);

	test_reset();
	EXPECTED_CALL(malloc(0)).IgnoreArgument_size().SetReturn(NULL);

	// act
	TRANSPORT_LL_HANDLE handle = provider->IoTHubTransport_Create(config);

	// assert
	ASSERT_IS_NULL(handle);

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_09_255: [If VECTOR_create fails, IoTHubTransportAMQP_Create shall fail and return.]
// Tests_SRS_IOTHUBTRANSPORTAMQP_09_236: [If IoTHubTransportAMQP_Create fails it shall free any memory it allocated (iotHubHostFqdn, transport state).]
TEST_FUNCTION(AMQP_Create_VECTOR_create_fails)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();
	IOTHUBTRANSPORT_CONFIG* config = create_transport_config(AMQP_Protocol, true);

	test_reset();

	EXPECTED_CALL(malloc(0)).IgnoreArgument_size();
	STRICT_EXPECTED_CALL(malloc(strlen(config->upperConfig->iotHubName) + strlen(config->upperConfig->iotHubSuffix) + 2));
	EXPECTED_CALL(STRING_construct(0)).SetReturn(TEST_IOTHUB_FQDN_STRING_HANDLE);
	STRICT_EXPECTED_CALL(VECTOR_create(sizeof(IOTHUB_DEVICE_HANDLE))).SetReturn((VECTOR_HANDLE)NULL);
	STRICT_EXPECTED_CALL(STRING_delete(TEST_IOTHUB_FQDN_STRING_HANDLE));
	STRICT_EXPECTED_CALL(free(saved_malloc_results[0]));

	// act
	TRANSPORT_LL_HANDLE handle = provider->IoTHubTransport_Create(config);

	// assert
	ASSERT_IS_NULL(handle);

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_20_001: [If config->upperConfig->protocolGatewayHostName is not NULL, IoTHubTransportAMQP_Create shall use it as iotHubHostFqdn]
// Tests_SRS_IOTHUBTRANSPORTAMQP_09_023: [If IoTHubTransportAMQP_Create succeeds it shall return a non-NULL pointer to the structure that represents the transport.]
// Tests_SRS_IOTHUBTRANSPORTAMQP_09_254: [IoTHubTransportAMQP_Create shall initialize the transport state registered device list with a VECTOR instance.]
TEST_FUNCTION(AMQP_Create_iothub_fqdn_succeeds)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();
	IOTHUBTRANSPORT_CONFIG* config = create_transport_config(AMQP_Protocol, true);

	test_reset();
	set_exp_calls_for_IoTHubTransportAMQP_Create(config);

	// act
	TRANSPORT_LL_HANDLE handle = provider->IoTHubTransport_Create(config);

	// assert
	ASSERT_ARE_EQUAL(void_ptr, saved_malloc_results[0], (void_ptr)handle);

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_09_010: [If config->upperConfig->protocolGatewayHostName is NULL, IoTHubTransportAMQP_Create shall create an immutable string, referred to as iotHubHostFqdn, from the following pieces: config->iotHubName + "." + config->iotHubSuffix.]
// Tests_SRS_IOTHUBTRANSPORTAMQP_09_023: [If IoTHubTransportAMQP_Create succeeds it shall return a non-NULL pointer to the structure that represents the transport.]
// Tests_SRS_IOTHUBTRANSPORTAMQP_09_254: [IoTHubTransportAMQP_Create shall initialize the transport state registered device list with a VECTOR instance.]
TEST_FUNCTION(AMQP_Create_protocol_gw_fqdn_succeeds)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();
	IOTHUBTRANSPORT_CONFIG* config = create_transport_config(AMQP_Protocol, false);

	test_reset();
	set_exp_calls_for_IoTHubTransportAMQP_Create(config);

	// act
	TRANSPORT_LL_HANDLE handle = provider->IoTHubTransport_Create(config);

	// assert
	ASSERT_ARE_EQUAL(void_ptr, saved_malloc_results[0], (void_ptr)handle);

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_02_001: [If parameter `handle` is NULL then `IoTHubTransportAMQP_GetHostname` shall return NULL.]
TEST_FUNCTION(AMQP_GetHostName_NULL_transport_handle)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();

	test_reset();

	// act
	STRING_HANDLE hostname = provider->IoTHubTransport_GetHostname(NULL);

	// assert
	ASSERT_IS_NULL(hostname);

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_02_002: [Otherwise IoTHubTransportAMQP_GetHostname shall return the target IoT Hub FQDN as a STRING_HANDLE.]
TEST_FUNCTION(AMQP_GetHostName_IoTHub_fqdn_succeeds)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();
	IOTHUBTRANSPORT_CONFIG* config = create_transport_config(AMQP_Protocol, true);

	test_reset();
	set_exp_calls_for_IoTHubTransportAMQP_Create(config);

	TRANSPORT_LL_HANDLE handle = provider->IoTHubTransport_Create(config);

	// act
	STRING_HANDLE hostname = provider->IoTHubTransport_GetHostname(handle);

	// assert
	ASSERT_ARE_EQUAL(STRING_HANDLE, hostname, TEST_IOTHUB_FQDN_STRING_HANDLE);

	// cleanup
	test_cleanup();
}

// Tests_SRS_IOTHUBTRANSPORTAMQP_02_002: [Otherwise IoTHubTransportAMQP_GetHostname shall return the target IoT Hub FQDN as a STRING_HANDLE.]
TEST_FUNCTION(AMQP_GetHostName_gw_protocol_fqdn_succeeds)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();
	IOTHUBTRANSPORT_CONFIG* config = create_transport_config(AMQP_Protocol, false);

	test_reset();
	set_exp_calls_for_IoTHubTransportAMQP_Create(config);

	TRANSPORT_LL_HANDLE handle = provider->IoTHubTransport_Create(config);

	// act
	STRING_HANDLE hostname = provider->IoTHubTransport_GetHostname(handle);

	// assert
	ASSERT_ARE_EQUAL(void_ptr, hostname, TEST_PROT_GW_FQDN_STRING_HANDLE);

	// cleanup
	test_cleanup();
}

TEST_FUNCTION(AMQP_Register_success)
{
	// arrange
	TRANSPORT_PROVIDER* provider = (TRANSPORT_PROVIDER*)AMQP_Protocol();
	IOTHUBTRANSPORT_CONFIG* config = create_transport_config(AMQP_Protocol, true);
	
	IOTHUB_DEVICE_CONFIG device_config;
	device_config.deviceId = TEST_DEVICE_ID;
	device_config.deviceKey = TEST_DEVICE_KEY;

	TRANSPORT_LL_HANDLE transport_handle = create_valid_transport_handle(config);

	test_reset();

	STRICT_EXPECTED_CALL(VECTOR_find_if(TEST_VECTOR_HANDLE, NULL, TEST_DEVICE_ID)).SetReturn((void*)TEST_DEVICE_STATE_HANDLE);
	EXPECTED_CALL(malloc(0));
	EXPECTED_CALL(DList_InitializeListHead(NULL));
	STRICT_EXPECTED_CALL(STRING_construct(TEST_DEVICE_ID)).SetReturn(TEST_DEVICE_ID_STRING_HANDLE);

	// devicesPath
	STRICT_EXPECTED_CALL(STRING_c_str(TEST_IOTHUB_FQDN_STRING_HANDLE)).SetReturn(TEST_IOTHUB_FQDN_CHAR_PTR);
	STRICT_EXPECTED_CALL(STRING_c_str(TEST_DEVICE_ID_STRING_HANDLE)).SetReturn(TEST_DEVICE_ID);

	EXPECTED_CALL(malloc(0));
	EXPECTED_CALL(STRING_construct(NULL)).SetReturn(TEST_DEVICES_PATH_STRING_HANDLE);
	STRICT_EXPECTED_CALL(free(saved_malloc_results[1]));

	// targetAddress
	STRICT_EXPECTED_CALL(STRING_c_str(TEST_DEVICES_PATH_STRING_HANDLE)).SetReturn(TEST_DEVICES_PATH_CHAR_PTR);

	EXPECTED_CALL(malloc(0));
	EXPECTED_CALL(STRING_construct(NULL)).SetReturn(TEST_TARGET_ADDRESS_STRING_HANDLE);
	STRICT_EXPECTED_CALL(free(saved_malloc_results[2]));

	// messageReceiveAddress
	STRICT_EXPECTED_CALL(STRING_c_str(TEST_DEVICES_PATH_STRING_HANDLE)).SetReturn(TEST_DEVICES_PATH_CHAR_PTR);

	EXPECTED_CALL(malloc(0));
	EXPECTED_CALL(STRING_construct(NULL)).SetReturn(TEST_MSG_RCV_ADDRESS_STRING_HANDLE);
	STRICT_EXPECTED_CALL(free(saved_malloc_results[3]));

	STRICT_EXPECTED_CALL(VECTOR_push_back(TEST_VECTOR_HANDLE, saved_malloc_results[0], 1)).SetReturn(0);

	STRICT_EXPECTED_CALL(STRING_c_str(TEST_DEVICE_ID_STRING_HANDLE)).SetReturn(TEST_DEVICE_ID);
	STRICT_EXPECTED_CALL(STRING_c_str(TEST_IOTHUB_FQDN_STRING_HANDLE)).SetReturn(TEST_IOTHUB_FQDN_CHAR_PTR);

	//STRICT_EXPECTED_CALL(authentication_create(NULL)).SetReturn(TEST_AUTHENTICATION_STATE_HANDLE);
	STRICT_EXPECTED_CALL(VECTOR_size(TEST_VECTOR_HANDLE)).SetReturn(0);


	// act
	IOTHUB_DEVICE_HANDLE device_handle = provider->IoTHubTransport_Register(transport_handle, &device_config, TEST_IOTHUB_CLIENT_LL_HANDLE, TEST_WAIT_TO_SEND_LIST);



	// assert
	//ASSERT_ARE_EQUAL(void_ptr, hostname, TEST_PROT_GW_FQDN_STRING_HANDLE);
	ASSERT_IS_NOT_NULL(device_handle);

	// cleanup
	test_cleanup();
	free(transport_handle);
}

END_TEST_SUITE(iothubtransportamqp_ut)
