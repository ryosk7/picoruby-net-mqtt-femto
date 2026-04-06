# picoruby-net-mqtt-femto

An external mrbgem for PicoRuby that provides an lwIP-based MQTT client for low-memory environments.

This library is maintained outside the main `picoruby/picoruby` repository.

`picoruby-net-mqtt-femto` is not intended to compete with the pure Ruby `picoruby-net-mqtt` implementation. The two libraries target different constraints and different execution environments.

This gem focuses on:

- low-memory, lwIP-oriented deployments
- a `picoruby-socket`-based integration model
- cooperative-multitasking-friendly behavior
- experimental but practical use on constrained boards such as RP2040/Pico W

In other words, `picoruby-net-mqtt` is the more portable and Ruby-centric option, while `picoruby-net-mqtt-femto` exists as a specialized option for environments where memory pressure and lwIP integration matter more than feature completeness.

| gem | implementation | target |
|---|---|---|
| picoruby-net-mqtt | pure Ruby | boards with more memory |
| picoruby-net-mqtt-femto | lwIP-based | low-memory environments |

## Features

- Native lwIP MQTT implementation for better performance
- MQTT 3.1.1 protocol support
- QoS 0 and QoS 1
- CONNECT, PUBLISH, SUBSCRIBE, PING, DISCONNECT
- Keep-alive with automatic PING
- Small fixed receive queue for bursty incoming messages
- API compatible with picoruby-net-mqtt (see support status below)
- Optimized for RP2040 (pico_w) boards

## Installation

This repository is intended to be maintained outside `picoruby/picoruby`.

If you want to use it from another gem or external build config, add it as a GitHub dependency:

```ruby
spec.add_dependency 'picoruby-net-mqtt-femto', 'ryosk7/picoruby-net-mqtt-femto'
```

## Usage

The API is compatible with picoruby-net-mqtt:

### Connect and Publish

```ruby
require 'net/mqtt'

# Connect to MQTT broker
client = Net::MQTT::Client.new("test.mosquitto.org", 1883)
client.connect

# Publish a message
client.publish("sensors/temperature", "25.5")

# Disconnect
client.disconnect
```

### Reconnect With Backoff

```ruby
require 'net/mqtt'

client = Net::MQTT::Client.new("test.mosquitto.org", 1883)
client.connect

begin
  client.publish("sensors/temperature", "25.5", qos: 1)
rescue Net::MQTT::ConnectionError
  client.reconnect(max_attempts: 5, base_delay_ms: 200, max_delay_ms: 2_000)
  retry
end
```

### Wait For Connection State

```ruby
require 'net/mqtt'

client = Net::MQTT::Client.new("test.mosquitto.org", 1883)
client.connect

client.wait_until_connected(timeout_ms: 3_000)
client.disconnect
client.wait_until_disconnected(timeout_ms: 3_000)
```

### Wrap Operations With Reconnect

```ruby
require 'net/mqtt'

client = Net::MQTT::Client.new("test.mosquitto.org", 1883)
client.connect

client.with_reconnect(max_attempts: 3, base_delay_ms: 200) do |mqtt|
  mqtt.publish("sensors/temperature", "25.5", qos: 1)
end
```

### Restore Subscriptions After Reconnect

```ruby
require 'net/mqtt'

client = Net::MQTT::Client.new("test.mosquitto.org", 1883)
client.connect
client.subscribe("sensors/#", qos: 1)

client.reconnect
# previously subscribed topics are subscribed again automatically
```

### Receive With Reconnect

```ruby
require 'net/mqtt'

client = Net::MQTT::Client.new("test.mosquitto.org", 1883)
client.connect
client.subscribe("sensors/#", qos: 1)

client.wait_until_message_available(timeout_ms: 3_000)
topic, payload = client.receive_with_reconnect(timeout: 5, max_attempts: 3)
puts "#{topic}: #{payload}"
```

### Publish And Subscribe With Reconnect

```ruby
require 'net/mqtt'

client = Net::MQTT::Client.new("test.mosquitto.org", 1883)
client.connect

client.subscribe_with_reconnect("sensors/#", qos: 1, max_attempts: 3)
client.publish_with_reconnect("sensors/temperature", "25.5", qos: 1, max_attempts: 3)
client.unsubscribe_with_reconnect("sensors/#", max_attempts: 3)
```

### Inspect Native State

```ruby
require 'net/mqtt'

client = Net::MQTT::Client.new("test.mosquitto.org", 1883)
client.connect

puts client.native_state
puts client.connecting?
puts client.active?
puts client.disconnecting?
puts client.timed_out?
puts client.publishing?
puts client.subscribing?
puts client.connection_status
puts client.connection_status_name
puts client.connection_error?
puts client.receive_queue_size
puts client.message_available?
puts client.receive_queue_empty?
puts client.pending_operation?
puts client.pending_publish?
puts client.pending_publish_topic
puts client.pending_publish_qos
puts client.pending_subscribe?
puts client.pending_subscribe_topic
pp client.pending_topics
puts client.pending_topic_count
pp client.stats
pp client.subscriptions
client.each_subscription do |topic, qos|
  puts "#{topic}: #{qos}"
end
puts client.drain_each_subscription { |topic, qos| puts "#{topic}: #{qos}" }
puts client.subscribed?("sensors/#")
puts client.subscription_qos("sensors/#")
pp client.subscription_topics
puts client.subscription_count
client.each_message do |topic, payload|
  puts "#{topic}: #{payload}"
end
pp client.message_topics
puts client.drain_each_message { |topic, payload| puts "#{topic}: #{payload}" }
pp client.drain_messages
```

### Subscribe to Topics

```ruby
require 'net/mqtt'

client = Net::MQTT::Client.new("test.mosquitto.org", 1883)
client.connect

# Subscribe to topics
client.subscribe("sensors/#", "alerts/#")
client.unsubscribe("alerts/#")

# Receive messages
5.times do
  topic, message = client.get
  puts "#{topic}: #{message}"
end

client.disconnect
```

## When to Use

### Use picoruby-net-mqtt-femto when:
- Running on RP2040-based boards (pico_w)
- Performance is critical
- Using mruby/c VM
- Need low memory footprint

## Requirements

- RP2040 board with WiFi (pico_w)
- picoruby-socket with lwIP MQTT support enabled
- mruby/c VM (RP2040 build only)

## API Compatibility

This gem provides the same `Net::MQTT` module and client API surface as picoruby-net-mqtt. Feature support differs:
- QoS 0 and QoS 1 are supported
- TLS/SSL is not supported
- `clean_session` option is currently ignored
- `ping` raises "not supported"

## Support Status

### Supported
- CONNECT / CONNACK
- PUBLISH / SUBSCRIBE
- PINGREQ / PINGRESP (automatic keep-alive)
- DISCONNECT
- QoS 0
- QoS 1

### Planned
- QoS 2

### Not Planned
- TLS/SSL
- `clean_session` option

## Example: IoT Sensor

```ruby
require 'net/mqtt'

def read_temperature
  # Read from sensor
  22.5 + rand * 3
end

client = Net::MQTT::Client.new("mqtt.example.com", 1883)
client.connect

# Publish sensor data every 5 seconds
loop do
  temp = read_temperature
  client.publish("sensors/temperature", temp.to_s)
  sleep 5
end
```

## License

MIT
