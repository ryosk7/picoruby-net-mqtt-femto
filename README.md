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
- QoS 0 (At most once delivery)
- CONNECT, PUBLISH, SUBSCRIBE, PING, DISCONNECT
- Keep-alive with automatic PING
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

### Subscribe to Topics

```ruby
require 'net/mqtt'

client = Net::MQTT::Client.new("test.mosquitto.org", 1883)
client.connect

# Subscribe to a topic
client.subscribe("sensors/#")

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
- QoS 0 only (higher QoS not supported yet)
- TLS/SSL is not supported
- username/password authentication is not supported yet
- `keep_alive` and `clean_session` options are currently ignored (keep-alive is fixed at 60s)
- `unsubscribe` and `ping` raise "not supported"

## Support Status

### Supported
- CONNECT / CONNACK
- PUBLISH / SUBSCRIBE
- PINGREQ / PINGRESP (automatic keep-alive)
- DISCONNECT
- QoS 0

### Planned
- `keep_alive` option (set keep-alive value from Ruby)
- `username` / `password` authentication
- `retain` flag
- `unsubscribe`
- QoS 1
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
