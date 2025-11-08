#ifndef MOSQUITTO_MOCK_H
#define MOSQUITTO_MOCK_H

#include <mosquitto.h>
#include <gmock/gmock.h>

class MosquittoMock;
extern MosquittoMock* gMosqMock;

class MosquittoMock
{
public:
    MosquittoMock() {gMosqMock = this;};
    ~MosquittoMock() {gMosqMock = nullptr;};
    MOCK_METHOD(int, mosquitto_lib_init, ()), (override);
    MOCK_METHOD(struct mosquitto *, mosquitto_new,(const char *id, bool clean_session, void *obj));
    MOCK_METHOD(int, mosquitto_int_option, (struct mosquitto *mosq, int option, int value));
    MOCK_METHOD(void, mosquitto_connect_v5_callback_set, (struct mosquitto *mosq, void (*on_connect)(struct mosquitto *, void *, int, int, const mosquitto_property *)));
    MOCK_METHOD(void, mosquitto_disconnect_v5_callback_set, (struct mosquitto *mosq, void (*on_disconnect)(struct mosquitto *, void *, int, const mosquitto_property *)));
    MOCK_METHOD(void, mosquitto_message_v5_callback_set, (struct mosquitto *mosq, void (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *, const mosquitto_property *)));
    MOCK_METHOD(int, mosquitto_tls_set, (struct mosquitto* mosq, const char* cafile, const char* capath,
                          const char* certfile, const char* keyfile, int (*cb)(char *buf, int size, int rwflag, void *userdata)));
    MOCK_METHOD(const char *, mosquitto_strerror, (int));
    MOCK_METHOD(void, mosquitto_destroy, (struct mosquitto *mosq));
    MOCK_METHOD(int, mosquitto_tls_insecure_set, (struct mosquitto *mosq, bool value));
    MOCK_METHOD(int, mosquitto_connect_bind_v5, (struct mosquitto *mosq, const char *host, int port, int keepalive, const char *bind_address, const mosquitto_property *properties));
    MOCK_METHOD(int, mosquitto_publish_v5, (struct mosquitto *mosq, int *mid, const char *topic, int payloadlen, const void *payload, int qos, bool retain, const mosquitto_property *properties), ());
    MOCK_METHOD(int, mosquitto_subscribe_v5, (struct mosquitto *mosq, int *mid, const char *sub, int qos, int options, const mosquitto_property *properties));
    MOCK_METHOD(int, mosquitto_disconnect_v5, (struct mosquitto *mosq, int reason_code, const mosquitto_property *properties));
    MOCK_METHOD(int, mosquitto_loop_start, (struct mosquitto *mosq));
    MOCK_METHOD(int, mosquitto_loop_stop, (struct mosquitto *mosq, bool force));
    MOCK_METHOD(int, mosquitto_lib_cleanup, ());
};

#endif