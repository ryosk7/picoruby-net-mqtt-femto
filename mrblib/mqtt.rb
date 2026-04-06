module Net
  module MQTT
    class MQTTError < StandardError; end
    class ConnectionError < MQTTError; end
    class ProtocolError < MQTTError; end

    CONNECT     = 1
    CONNACK     = 2
    PUBLISH     = 3
    PUBACK      = 4
    PUBREC      = 5
    PUBREL      = 6
    PUBCOMP     = 7
    SUBSCRIBE   = 8
    SUBACK      = 9
    UNSUBSCRIBE = 10
    UNSUBACK    = 11
    PINGREQ     = 12
    PINGRESP    = 13
    DISCONNECT  = 14

    STATE_NAMES = {
      0 => "idle",
      1 => "connecting",
      2 => "connack_wait",
      3 => "active",
      4 => "subscribing",
      5 => "publishing",
      6 => "disconnecting",
      7 => "error",
      8 => "timeout"
    }

    CONNECTION_ERRORS = {
      1 => "Connection refused: unsupported protocol version",
      2 => "Connection refused: identifier rejected",
      3 => "Connection refused: server unavailable",
      4 => "Connection refused: invalid username or password",
      5 => "Connection refused: not authorized",
      256 => "Disconnected by broker",
      257 => "Connection timed out"
    }

    class Client
      attr_reader :host, :port
      attr_accessor :client_id, :keep_alive, :clean_session
      attr_accessor :username, :password
      attr_accessor :will_topic, :will_message, :will_qos, :will_retain
      attr_accessor :auto_resubscribe
      attr_accessor :ssl, :ca_file, :cert_file, :key_file

      def initialize(host, port = 1883, **options)
        @host = host
        @port = port
        @client_id = options[:client_id] || "picoruby-#{Time.now.to_i}"
        @keep_alive = options[:keep_alive] || 60
        @username = options[:username] # not work
        @password = options[:password] # not work
        @will_topic = options[:will_topic]
        @will_message = options[:will_message]
        @will_qos = options[:will_qos] || 0
        @will_retain = options[:will_retain] || false
        @auto_resubscribe = options.key?(:auto_resubscribe) ? options[:auto_resubscribe] : true
        @ssl = options[:ssl] || false # not work
        @ca_file = options[:ca_file] # not work
        @cert_file = options[:cert_file] # not work
        @key_file = options[:key_file] # not work
        @connected = false
        @subscriptions = {}
      end

      def self.connect(host, port = 1883, **options, &block)
        client = new(host, port, **options)
        client.connect
        if block
          begin
            block.call(client)
          ensure
            client.disconnect
          end
        else
          client
        end
      end

      def connect
        if @keep_alive < 0 || @keep_alive > 65_535
          raise MQTTError.new("keep_alive must be between 0 and 65535")
        end
        if @will_qos < 0 || @will_qos > 2
          raise MQTTError.new("will_qos must be between 0 and 2")
        end
        if @will_topic.nil? != @will_message.nil?
          raise MQTTError.new("will_topic and will_message must be set together")
        end

        # Initiate non-blocking connection
        result = _connect_impl(@host, @port, @client_id, @keep_alive,
                               @username, @password, @will_topic,
                               @will_message, @will_qos, @will_retain, @ssl)
        raise ConnectionError.new(connection_error_message) unless result

        # Short test loop (~3 seconds timeout)
        300.times do |i|
          _poll_impl if respond_to?(:_poll_impl)
          if _is_connected_impl
            @connected = true
            return true
          end

          poll_sleep_ms(10)
        end

        @connected = false
        raise ConnectionError.new(connection_error_message("Connection timeout"))
      end

      def poll_sleep_ms(ms)
        if respond_to?(:_poll_sleep_impl)
          _poll_sleep_impl(ms)
        elsif respond_to?(:_poll_impl)
          ms.times do
            _poll_impl
            sleep_ms 1
          end
        else
          sleep_ms(ms)
        end
      end

      def disconnect
        return unless connected?
        _disconnect_impl
        @connected = false
      end

      def reconnect(max_attempts: nil, base_delay_ms: 100, max_delay_ms: 5_000)
        raise MQTTError.new("base_delay_ms must be positive") if base_delay_ms <= 0
        raise MQTTError.new("max_delay_ms must be positive") if max_delay_ms <= 0

        disconnect if connected?

        attempts = 0
        delay_ms = base_delay_ms

        loop do
          attempts += 1

          begin
            connect
            restore_subscriptions if @auto_resubscribe
            return true
          rescue ConnectionError
            raise if max_attempts && attempts >= max_attempts
          end

          poll_sleep_ms(delay_ms)
          delay_ms = [delay_ms * 2, max_delay_ms].min
        end
      end

      def wait_until_connected(timeout_ms: 3_000, poll_interval_ms: 10)
        raise MQTTError.new("timeout_ms must be non-negative") if timeout_ms < 0
        raise MQTTError.new("poll_interval_ms must be positive") if poll_interval_ms <= 0

        waited_ms = 0
        while waited_ms <= timeout_ms
          return true if connected?
          poll_sleep_ms(poll_interval_ms)
          waited_ms += poll_interval_ms
        end

        false
      end

      def wait_until_disconnected(timeout_ms: 3_000, poll_interval_ms: 10)
        raise MQTTError.new("timeout_ms must be non-negative") if timeout_ms < 0
        raise MQTTError.new("poll_interval_ms must be positive") if poll_interval_ms <= 0

        waited_ms = 0
        while waited_ms <= timeout_ms
          return true unless connected?
          poll_sleep_ms(poll_interval_ms)
          waited_ms += poll_interval_ms
        end

        false
      end

      def wait_until_message_available(timeout_ms: 3_000, poll_interval_ms: 10)
        raise MQTTError.new("timeout_ms must be non-negative") if timeout_ms < 0
        raise MQTTError.new("poll_interval_ms must be positive") if poll_interval_ms <= 0

        waited_ms = 0
        while waited_ms <= timeout_ms
          return true if message_available?
          return false unless connected?
          poll_sleep_ms(poll_interval_ms)
          waited_ms += poll_interval_ms
        end

        false
      end

      def with_reconnect(max_attempts: nil, base_delay_ms: 100, max_delay_ms: 5_000)
        raise ArgumentError, "block required" unless block_given?

        attempts = 0

        loop do
          begin
            return yield self
          rescue ConnectionError
            attempts += 1
            raise if max_attempts && attempts > max_attempts

            reconnect(max_attempts: 1, base_delay_ms: base_delay_ms,
                      max_delay_ms: max_delay_ms)
          end
        end
      end

      def receive_with_reconnect(timeout: nil, max_attempts: nil,
                                 base_delay_ms: 100, max_delay_ms: 5_000, &block)
        if block_given?
          loop do
            message = with_reconnect(max_attempts: max_attempts,
                                     base_delay_ms: base_delay_ms,
                                     max_delay_ms: max_delay_ms) do |mqtt|
              mqtt.receive(timeout: timeout)
            end

            next unless message

            yield(message[0], message[1])
          end
        else
          with_reconnect(max_attempts: max_attempts,
                         base_delay_ms: base_delay_ms,
                         max_delay_ms: max_delay_ms) do |mqtt|
            mqtt.receive(timeout: timeout)
          end
        end
      end

      def publish_with_reconnect(topic, payload, retain: false, qos: 0,
                                 max_attempts: nil, base_delay_ms: 100, max_delay_ms: 5_000)
        with_reconnect(max_attempts: max_attempts,
                       base_delay_ms: base_delay_ms,
                       max_delay_ms: max_delay_ms) do |mqtt|
          mqtt.publish(topic, payload, retain: retain, qos: qos)
        end
      end

      def subscribe_with_reconnect(*topics, qos: 0, max_attempts: nil,
                                   base_delay_ms: 100, max_delay_ms: 5_000)
        with_reconnect(max_attempts: max_attempts,
                       base_delay_ms: base_delay_ms,
                       max_delay_ms: max_delay_ms) do |mqtt|
          mqtt.subscribe(*topics, qos: qos)
        end
      end

      def unsubscribe_with_reconnect(*topics, max_attempts: nil,
                                     base_delay_ms: 100, max_delay_ms: 5_000)
        with_reconnect(max_attempts: max_attempts,
                       base_delay_ms: base_delay_ms,
                       max_delay_ms: max_delay_ms) do |mqtt|
          mqtt.unsubscribe(*topics)
        end
      end

      def connected?
        return false unless @connected

        @connected = false unless _is_connected_impl
        @connected
      end

      def connection_status
        _connection_status_impl
      end

      def connection_status_name
        connection_error_message("ok")
      end

      def connection_error?
        connection_status != 0
      end

      def native_state
        STATE_NAMES[_fsm_state_impl] || "unknown"
      end

      def receive_queue_size
        _receive_queue_size_impl
      end

      def message_available?
        receive_queue_size > 0
      end

      def drain_messages
        messages = []

        while (message = _get_message_impl)
          messages << message
        end

        messages
      end

      def pending_publish_topic
        _pending_publish_topic_impl
      end

      def pending_publish?
        !pending_publish_topic.nil?
      end

      def pending_publish_qos
        _pending_publish_qos_impl
      end

      def pending_subscribe_topic
        _pending_subscribe_topic_impl
      end

      def pending_subscribe?
        !pending_subscribe_topic.nil?
      end

      def subscriptions
        @subscriptions.dup
      end

      def stats
        {
          connected: connected?,
          native_state: native_state,
          connecting: native_state == "connecting" || native_state == "connack_wait",
          active: native_state == "active",
          disconnecting: native_state == "disconnecting",
          timed_out: native_state == "timeout",
          publishing: native_state == "publishing",
          subscribing: native_state == "subscribing",
          connection_status: connection_status,
          connection_status_name: connection_status_name,
          connection_error: connection_error?,
          receive_queue_size: receive_queue_size,
          message_available: message_available?,
          receive_queue_empty: receive_queue_size == 0,
          pending_operation: pending_publish? || pending_subscribe?,
          pending_publish: pending_publish?,
          pending_publish_topic: pending_publish_topic,
          pending_publish_qos: pending_publish_qos,
          pending_subscribe: pending_subscribe?,
          pending_subscribe_topic: pending_subscribe_topic,
          subscriptions: @subscriptions.length,
          auto_resubscribe: @auto_resubscribe,
        }
      end

      def publish(topic, payload, retain: false, qos: 0)
        raise MQTTError.new("Not connected") unless connected?
        raise MQTTError.new("QoS must be 0 or 1") unless [0, 1].include?(qos)
        _publish_impl(topic, payload.to_s, retain, qos)
      end

      def subscribe(*topics, qos: 0)
        raise MQTTError.new("Not connected") unless connected?
        raise MQTTError.new("QoS must be 0 or 1") unless [0, 1].include?(qos)
        topics.each do |topic|
          @subscriptions[topic] = qos
          _subscribe_impl(topic, qos)
        end
      end

      def unsubscribe(*topics)
        raise MQTTError.new("Not connected") unless connected?
        topics.each do |topic|
          @subscriptions.delete(topic)
          _unsubscribe_impl(topic)
        end
      end

      def receive(timeout: nil)
        raise MQTTError.new("Not connected") unless connected?

        deadline = timeout ? Time.now.to_f + timeout : nil
        while true
          message = _get_message_impl
          return message if message
          unless connected?
            raise ConnectionError.new(connection_error_message("Connection lost"))
          end
          return nil if deadline && Time.now.to_f > deadline
          sleep_ms 10
        end
      end

      def ping
        raise MQTTError.new("Not connected") unless connected?
        raise MQTTError.new("ping not supported")
      end

      def get(&block)
        return unless connected?

        if block_given?
          # Non-Blocking
          loop do
            message = _get_message_impl
            if message
              yield(message[0], message[1])
            elsif !connected?
              raise ConnectionError.new(connection_error_message("Connection lost"))
            end
            sleep_ms 10
          end
        else
          # Blocking
          _get_message_impl
        end
      end

      private

      def connection_error_message(default = "Connection failed")
        CONNECTION_ERRORS[_connection_status_impl] || default
      end

      def restore_subscriptions
        @subscriptions.each do |topic, qos|
          _subscribe_impl(topic, qos)
        end
      end
    end
  end
end
