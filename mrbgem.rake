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
    pico_sdk_dir = "#{MRUBY_ROOT}/mrbgems/picoruby-r2p2/lib/pico-sdk"
    spec.cc.flags << '-std=gnu11'
    spec.cc.include_paths << "#{MRUBY_ROOT}/mrbgems/picoruby-socket/include"
    spec.cc.include_paths << "#{lwip_dir}/src/include"
    spec.cc.include_paths << "#{lwip_dir}/contrib/ports/unix/port/include"
    spec.cc.include_paths << "#{lwip_dir}/src/apps/altcp_tls"
    spec.cc.defines << 'PICO_CYW43_ARCH_POLL=1'
    spec.cc.defines << 'PICO_RP2040=1'
    spec.cc.defines << 'PICO_BOARD="pico_w"'

    if File.directory?(pico_sdk_dir)
      pico_include_paths = Dir.glob(
        "#{pico_sdk_dir}/src/{boards,common,rp2_common,rp2040}/**/include"
      ).select { |path| File.directory?(path) }
      pico_lib_include_paths = Dir.glob(
        "#{pico_sdk_dir}/lib/**/include"
      ).select { |path| File.directory?(path) }
      pico_lib_include_paths << "#{pico_sdk_dir}/lib/cyw43-driver/src"
      spec.cc.include_paths.concat((pico_include_paths + pico_lib_include_paths).sort.uniq)
    end

    obj = "#{dir}/ports/rp2040/mqtt.c".relative_path_from(dir).pathmap("#{build_dir}/%X.o")
    spec.objs << obj
    file obj => "#{dir}/ports/rp2040/mqtt.c" do |t|
      cc.run t.name, t.prerequisites.first
    end
  end
end
