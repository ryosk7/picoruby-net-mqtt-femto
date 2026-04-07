/*
 * MQTT mrubyc bindings
 * lwIP-independent layer - only handles Ruby to C interface
 */

#include <mrubyc.h>

// External function declarations (implementations in ports/rp2040/mqtt.c)
extern int MQTT_connect_impl(const char *host, int port, const char *client_id,
                             int keep_alive, const char *username,
                             const char *password, const char *will_topic,
                             const char *will_message, int will_qos,
                             int will_retain, int ssl,
                             uintptr_t ca_addr, int ca_size);
extern void MQTT_poll_impl(void);
extern void MQTT_poll_sleep_impl(int ms);
extern int MQTT_is_connected_impl(void);
extern int MQTT_connection_status_impl(void);
extern int MQTT_fsm_state_impl(void);
extern int MQTT_receive_queue_size_impl(void);
extern int MQTT_clear_timeout_impl(void);
extern const char *MQTT_pending_publish_topic_impl(void);
extern int MQTT_pending_publish_qos_impl(void);
extern const char *MQTT_pending_subscribe_topic_impl(void);
extern int MQTT_publish_impl(const char *topic, const char *payload, int len,
                             int qos, int retain);
extern int MQTT_subscribe_impl(const char *topic, int qos);
extern int MQTT_unsubscribe_impl(const char *topic);
extern int MQTT_get_message_impl(char **topic, char **payload);
extern void MQTT_disconnect_impl(void);

// mrubyc C API bindings

static void
c_mqtt_connect(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if (argc != 13) {
    SET_FALSE_RETURN();
    return;
  }

  if (v[1].tt != MRBC_TT_STRING) {
    SET_FALSE_RETURN();
    return;
  }

  if (v[2].tt != MRBC_TT_INTEGER) {
    SET_FALSE_RETURN();
    return;
  }

  if (v[3].tt != MRBC_TT_STRING) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[4].tt != MRBC_TT_INTEGER) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[5].tt != MRBC_TT_NIL && v[5].tt != MRBC_TT_STRING) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[6].tt != MRBC_TT_NIL && v[6].tt != MRBC_TT_STRING) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[7].tt != MRBC_TT_NIL && v[7].tt != MRBC_TT_STRING) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[8].tt != MRBC_TT_NIL && v[8].tt != MRBC_TT_STRING) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[9].tt != MRBC_TT_INTEGER) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[10].tt != MRBC_TT_TRUE && v[10].tt != MRBC_TT_FALSE) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[11].tt != MRBC_TT_TRUE && v[11].tt != MRBC_TT_FALSE) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[12].tt != MRBC_TT_NIL && v[12].tt != MRBC_TT_INTEGER) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[13].tt != MRBC_TT_INTEGER) {
    SET_FALSE_RETURN();
    return;
  }

  const char *host = (const char *)GET_STRING_ARG(1);
  int port = GET_INT_ARG(2);
  const char *client_id = (const char *)GET_STRING_ARG(3);
  int keep_alive = GET_INT_ARG(4);
  const char *username = (v[5].tt == MRBC_TT_STRING) ? (const char *)GET_STRING_ARG(5) : NULL;
  const char *password = (v[6].tt == MRBC_TT_STRING) ? (const char *)GET_STRING_ARG(6) : NULL;
  const char *will_topic = (v[7].tt == MRBC_TT_STRING) ? (const char *)GET_STRING_ARG(7) : NULL;
  const char *will_message = (v[8].tt == MRBC_TT_STRING) ? (const char *)GET_STRING_ARG(8) : NULL;
  int will_qos = GET_INT_ARG(9);
  int will_retain = (v[10].tt == MRBC_TT_TRUE) ? 1 : 0;
  int ssl = (v[11].tt == MRBC_TT_TRUE) ? 1 : 0;
  uintptr_t ca_addr = (v[12].tt == MRBC_TT_INTEGER) ? (uintptr_t)GET_INT_ARG(12) : (uintptr_t)0;
  int ca_size = GET_INT_ARG(13);

  int result = MQTT_connect_impl(host, port, client_id, keep_alive, username,
                                 password, will_topic, will_message,
                                 will_qos, will_retain, ssl, ca_addr, ca_size);

  if (result == 0) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}

static void
c_mqtt_publish(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if (argc != 4) {
    SET_FALSE_RETURN();
    return;
  }

  if (v[1].tt != MRBC_TT_STRING || v[2].tt != MRBC_TT_STRING) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[3].tt != MRBC_TT_TRUE && v[3].tt != MRBC_TT_FALSE) {
    SET_FALSE_RETURN();
    return;
  }
  if (v[4].tt != MRBC_TT_INTEGER) {
    SET_FALSE_RETURN();
    return;
  }

  const char *topic = (const char *)GET_STRING_ARG(1);
  const char *payload = (const char *)GET_STRING_ARG(2);
  int retain = (v[3].tt == MRBC_TT_TRUE) ? 1 : 0;
  int qos = GET_INT_ARG(4);

  int result = MQTT_publish_impl(topic, payload, 0, qos, retain);

  if (result == 0) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}

static void
c_mqtt_subscribe(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if (argc != 2) {
    SET_FALSE_RETURN();
    return;
  }

  if (v[1].tt != MRBC_TT_STRING || v[2].tt != MRBC_TT_INTEGER) {
    SET_FALSE_RETURN();
    return;
  }

  const char *topic = (const char *)GET_STRING_ARG(1);
  int qos = GET_INT_ARG(2);

  int result = MQTT_subscribe_impl(topic, qos);

  if (result == 0) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}

static void
c_mqtt_get_message(mrbc_vm *vm, mrbc_value v[], int argc)
{
  char *topic, *payload;
  int result = MQTT_get_message_impl(&topic, &payload);

  if (result == 0 && topic && payload) {
    // Return array [topic, payload]
    mrbc_value array = mrbc_array_new(vm, 2);
    mrbc_value topic_val = mrbc_string_new_cstr(vm, topic);
    mrbc_value payload_val = mrbc_string_new_cstr(vm, payload);

    mrbc_array_set(&array, 0, &topic_val);
    mrbc_array_set(&array, 1, &payload_val);

    SET_RETURN(array);
  } else {
    SET_NIL_RETURN();
  }
}

static void
c_mqtt_unsubscribe(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if (argc != 1) {
    SET_FALSE_RETURN();
    return;
  }

  if (v[1].tt != MRBC_TT_STRING) {
    SET_FALSE_RETURN();
    return;
  }

  const char *topic = (const char *)GET_STRING_ARG(1);

  int result = MQTT_unsubscribe_impl(topic);

  if (result == 0) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}

static void
c_mqtt_is_connected(mrbc_vm *vm, mrbc_value v[], int argc)
{
  int connected = MQTT_is_connected_impl();
  if (connected) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}

static void
c_mqtt_connection_status(mrbc_vm *vm, mrbc_value v[], int argc)
{
  SET_INT_RETURN(MQTT_connection_status_impl());
}

static void
c_mqtt_fsm_state(mrbc_vm *vm, mrbc_value v[], int argc)
{
  SET_INT_RETURN(MQTT_fsm_state_impl());
}

static void
c_mqtt_receive_queue_size(mrbc_vm *vm, mrbc_value v[], int argc)
{
  SET_INT_RETURN(MQTT_receive_queue_size_impl());
}

static void
c_mqtt_clear_timeout(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if (MQTT_clear_timeout_impl()) {
    SET_TRUE_RETURN();
  } else {
    SET_FALSE_RETURN();
  }
}

static void
c_mqtt_pending_publish_topic(mrbc_vm *vm, mrbc_value v[], int argc)
{
  const char *topic = MQTT_pending_publish_topic_impl();
  if (topic) {
    SET_RETURN(mrbc_string_new_cstr(vm, topic));
  } else {
    SET_NIL_RETURN();
  }
}

static void
c_mqtt_pending_publish_qos(mrbc_vm *vm, mrbc_value v[], int argc)
{
  SET_INT_RETURN(MQTT_pending_publish_qos_impl());
}

static void
c_mqtt_pending_subscribe_topic(mrbc_vm *vm, mrbc_value v[], int argc)
{
  const char *topic = MQTT_pending_subscribe_topic_impl();
  if (topic) {
    SET_RETURN(mrbc_string_new_cstr(vm, topic));
  } else {
    SET_NIL_RETURN();
  }
}

static void
c_mqtt_poll(mrbc_vm *vm, mrbc_value v[], int argc)
{
  MQTT_poll_impl();
  SET_NIL_RETURN();
}

static void
c_mqtt_poll_sleep(mrbc_vm *vm, mrbc_value v[], int argc)
{
  if (argc != 1 || v[1].tt != MRBC_TT_INTEGER) {
    SET_NIL_RETURN();
    return;
  }
  MQTT_poll_sleep_impl(GET_INT_ARG(1));
  SET_NIL_RETURN();
}

static void
c_mqtt_disconnect(mrbc_vm *vm, mrbc_value v[], int argc)
{
  MQTT_disconnect_impl();
  SET_NIL_RETURN();
}

void mrbc_net_mqtt_femto_init(mrbc_vm *vm) {
  mrbc_define_method(0, mrbc_class_object, "_connect_impl", c_mqtt_connect);
  mrbc_define_method(0, mrbc_class_object, "_is_connected_impl", c_mqtt_is_connected);
  mrbc_define_method(0, mrbc_class_object, "_connection_status_impl",
                     c_mqtt_connection_status);
  mrbc_define_method(0, mrbc_class_object, "_fsm_state_impl", c_mqtt_fsm_state);
  mrbc_define_method(0, mrbc_class_object, "_receive_queue_size_impl",
                     c_mqtt_receive_queue_size);
  mrbc_define_method(0, mrbc_class_object, "_clear_timeout_impl",
                     c_mqtt_clear_timeout);
  mrbc_define_method(0, mrbc_class_object, "_pending_publish_topic_impl",
                     c_mqtt_pending_publish_topic);
  mrbc_define_method(0, mrbc_class_object, "_pending_publish_qos_impl",
                     c_mqtt_pending_publish_qos);
  mrbc_define_method(0, mrbc_class_object, "_pending_subscribe_topic_impl",
                     c_mqtt_pending_subscribe_topic);
  mrbc_define_method(0, mrbc_class_object, "_poll_impl", c_mqtt_poll);
  mrbc_define_method(0, mrbc_class_object, "_poll_sleep_impl", c_mqtt_poll_sleep);
  mrbc_define_method(0, mrbc_class_object, "_publish_impl", c_mqtt_publish);
  mrbc_define_method(0, mrbc_class_object, "_subscribe_impl", c_mqtt_subscribe);
  mrbc_define_method(0, mrbc_class_object, "_unsubscribe_impl", c_mqtt_unsubscribe);
  mrbc_define_method(0, mrbc_class_object, "_get_message_impl", c_mqtt_get_message);
  mrbc_define_method(0, mrbc_class_object, "_disconnect_impl", c_mqtt_disconnect);
}
