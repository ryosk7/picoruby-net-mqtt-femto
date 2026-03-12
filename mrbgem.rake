MRuby::Gem::Specification.new('picoruby-net-mqtt-femto') do |spec|
  if build.vm_mruby?
    raise "picoruby-net-mqtt-femto is mruby/c (RP2040) only"
  end

  spec.license = 'MIT'
  spec.authors = ['Ryosuke Uchida']
  spec.summary = 'lwIP native MQTT client for PicoRuby (RP2040 optimized)'
  spec.require_name = 'net/mqtt'

  spec.add_dependency 'picoruby-socket'
  spec.add_dependency 'picoruby-pack'
  spec.add_dependency 'picoruby-time'

  spec.cc.include_paths << "#{dir}/include"

  unless build.posix?
    lwip_dir = "#{MRUBY_ROOT}/mrbgems/picoruby-socket/lib/lwip"
    spec.cc.include_paths << "#{MRUBY_ROOT}/mrbgems/picoruby-socket/include"
    spec.cc.include_paths << "#{lwip_dir}/src/include"
    spec.cc.include_paths << "#{lwip_dir}/contrib/ports/unix/port/include"
    spec.cc.include_paths << "#{lwip_dir}/src/apps/altcp_tls"

    spec.objs << "#{dir}/ports/rp2040/mqtt.o"
    file "#{dir}/ports/rp2040/mqtt.o" => "#{dir}/ports/rp2040/mqtt.c" do |t|
      cc.run t.name, t.prerequisites.first
    end
  end
end
