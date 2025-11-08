#include "MosquittoMock.h"
MosquittoMock* gMosqMock = nullptr;

extern "C" {
    int mosquitto_lib_init(void) {
        return gMosqMock ? gMosqMock->mosquitto_lib_init() : MOSQ_ERR_SUCCESS;
    }

    struct mosquitto* mosquitto_new(const char* id, bool clean_session, void* userdata) {
        return gMosqMock ? gMosqMock->mosquitto_new(id, clean_session, userdata)
                          : nullptr;
    }

    int mosquitto_int_option(struct mosquitto* mosq, enum mosq_opt_t option, int value) {
        return gMosqMock ? gMosqMock->mosquitto_int_option(mosq, option, value)
                          : MOSQ_ERR_SUCCESS;
    }

    int mosquitto_tls_set(struct mosquitto* mosq, const char* cafile, const char* capath,
                          const char* certfile, const char* keyfile, int (*cb)(char *buf, int size, int rwflag, void *userdata)) {
        return gMosqMock ? gMosqMock->mosquitto_tls_set(mosq, cafile, capath, certfile, keyfile, cb)
                          : MOSQ_ERR_SUCCESS;
    }

    int mosquitto_tls_insecure_set(struct mosquitto* mosq, bool value) {
        return gMosqMock ? gMosqMock->mosquitto_tls_insecure_set(mosq, value)
                          : MOSQ_ERR_SUCCESS;
    }

    int mosquitto_connect_bind_v5(struct mosquitto* mosq, const char* host, int port, int keepalive,
                                  const char* bind_address, const mosquitto_property* props) {
        return gMosqMock ? gMosqMock->mosquitto_connect_bind_v5(mosq, host, port, keepalive, bind_address, props)
                          : MOSQ_ERR_SUCCESS;
    }
    int mosquitto_publish_v5(struct mosquitto* mosq, int* mid, const char* topic, int payloadlen,
                             const void* payload, int qos, bool retain, const mosquitto_property* props) {
        return gMosqMock ? gMosqMock->mosquitto_publish_v5(mosq, mid, topic, payloadlen, payload, qos, retain, props)
                          : MOSQ_ERR_SUCCESS;
    }
    const char* mosquitto_strerror(int err) {
        return gMosqMock ? gMosqMock->mosquitto_strerror(err)
                          : "Success";
    }
    void mosquitto_destroy(struct mosquitto* mosq) {
        if (gMosqMock) {
            gMosqMock->mosquitto_destroy(mosq);
        }
    }

    int mosquitto_subscribe_v5(struct mosquitto *mosq, int *mid, const char *sub, int qos, int options, const mosquitto_property *properties) {
        return gMosqMock ? gMosqMock->mosquitto_subscribe_v5(mosq, mid, sub, qos, options, properties)
                          : MOSQ_ERR_SUCCESS;
    }

    int mosquitto_disconnect_v5(struct mosquitto *mosq, int reason_code, const mosquitto_property *properties) {
        return gMosqMock ? gMosqMock->mosquitto_disconnect_v5(mosq, reason_code, properties)
                          : MOSQ_ERR_SUCCESS;
    }

    void mosquitto_connect_v5_callback_set(struct mosquitto *mosq, void (*on_connect)(struct mosquitto *, void *, int, int, const mosquitto_property *)) {
        if (gMosqMock) {
            gMosqMock->mosquitto_connect_v5_callback_set(mosq, on_connect);
        }
    }

    void mosquitto_disconnect_v5_callback_set(struct mosquitto *mosq, void (*on_disconnect)(struct mosquitto *, void *, int, const mosquitto_property *)) {
        if (gMosqMock) {
            gMosqMock->mosquitto_disconnect_v5_callback_set(mosq, on_disconnect);
        }
    }

    void mosquitto_message_v5_callback_set(struct mosquitto *mosq, void (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *, const mosquitto_property *)) {
        if (gMosqMock) {
            gMosqMock->mosquitto_message_v5_callback_set(mosq, on_message);
        }
    }

    int mosquitto_loop_start(struct mosquitto *mosq) {
        return gMosqMock ? gMosqMock->mosquitto_loop_start(mosq)
                          : MOSQ_ERR_SUCCESS;
    }

    int mosquitto_loop_stop(struct mosquitto *mosq, bool force) {
        return gMosqMock ? gMosqMock->mosquitto_loop_stop(mosq, force)
                          : MOSQ_ERR_SUCCESS;
    }

    int mosquitto_lib_cleanup() {
        if (gMosqMock) {
            gMosqMock->mosquitto_lib_cleanup();
        }
    }
}
