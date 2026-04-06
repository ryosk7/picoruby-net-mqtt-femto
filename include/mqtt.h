#ifndef MQTT_H_
#define MQTT_H_

#include <stdint.h>
#include <stdbool.h>

#include "picoruby.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MQTT_TOPIC_MAX_LEN
#define MQTT_TOPIC_MAX_LEN 64
#endif

#ifndef MQTT_PAYLOAD_MAX_LEN
#define MQTT_PAYLOAD_MAX_LEN 256
#endif

#ifndef MQTT_CLIENT_ID_MAX_LEN
#define MQTT_CLIENT_ID_MAX_LEN 64
#endif

#ifndef MQTT_RECEIVE_QUEUE_LEN
#define MQTT_RECEIVE_QUEUE_LEN 4
#endif

typedef enum {
  MQTT_STATE_IDLE,
  MQTT_STATE_CONNECTING,
  MQTT_STATE_CONNACK_WAIT,
  MQTT_STATE_ACTIVE,
  MQTT_STATE_SUBSCRIBING,
  MQTT_STATE_PUBLISHING,
  MQTT_STATE_DISCONNECTING,
  MQTT_STATE_ERROR,
  MQTT_STATE_TIMEOUT
} mqtt_fsm_state_t;

typedef struct {
  char topic[MQTT_TOPIC_MAX_LEN];
  char payload[MQTT_PAYLOAD_MAX_LEN];
} mqtt_message_t;

typedef struct {
  void *client;  // Platform-specific client pointer
  void *tls_config; // Platform-specific TLS config pointer
  mqtt_fsm_state_t fsm_state;

  char topic_to_sub[MQTT_TOPIC_MAX_LEN];
  int subscribe_qos;
  char topic_to_pub[MQTT_TOPIC_MAX_LEN];
  char payload_to_pub[MQTT_PAYLOAD_MAX_LEN];
  int payload_to_pub_len;
  int publish_qos;
  int publish_retain;

  char recv_topic[MQTT_TOPIC_MAX_LEN];
  char recv_payload[MQTT_PAYLOAD_MAX_LEN];
  int recv_payload_len;
  mqtt_message_t recv_queue[MQTT_RECEIVE_QUEUE_LEN];
  int recv_queue_head;
  int recv_queue_tail;
  int recv_queue_count;
  char client_id[MQTT_CLIENT_ID_MAX_LEN];
  int connection_status;

  void *vm;           // VM pointer (platform-specific)
  void *callback_proc; // Callback procedure (platform-specific)
} mqtt_context_t;

int MQTT_connect_impl(const char *host, int port, const char *client_id,
                      int keep_alive, const char *username,
                      const char *password, const char *will_topic,
                      const char *will_message, int will_qos,
                      int will_retain, int ssl);
void MQTT_poll_impl(void);
void MQTT_poll_sleep_impl(int ms);
int MQTT_is_connected_impl(void);
int MQTT_connection_status_impl(void);
int MQTT_fsm_state_impl(void);
int MQTT_receive_queue_size_impl(void);
const char *MQTT_pending_publish_topic_impl(void);
int MQTT_pending_publish_qos_impl(void);
const char *MQTT_pending_subscribe_topic_impl(void);
int MQTT_subscribe_impl(const char *topic, int qos);
int MQTT_unsubscribe_impl(const char *topic);
int MQTT_publish_impl(const char *topic, const char *payload, int len,
                      int qos, int retain);
void MQTT_disconnect_impl(void);
int MQTT_get_message_impl(char **topic, char **payload);

void MQTT_init_context(void *vm);
void MQTT_set_callback(void *proc);

void mrb_picoruby_net_mqtt_femto_gem_init(void* mrb);
void mrb_picoruby_net_mqtt_femto_gem_final(void* mrb);
void mrbc_net_mqtt_femto_init(void *vm);

#ifdef __cplusplus
}
#endif

#endif
