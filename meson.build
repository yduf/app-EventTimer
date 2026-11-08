project(
  'event-timer',
  'cpp',
  version: '0.1.0',
  meson_version: '>=1.1.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=2',
    'buildtype=debugoptimized',
  ],
)

fs = import('fs')
cpp = meson.get_compiler('cpp')

slint_dir = get_option('slint_dir')
if slint_dir == ''
  slint_dir = meson.project_source_root() / 'subprojects' / 'slint-cpp'
  if not fs.exists(slint_dir / 'lib' / 'libslint_cpp.so')
    ruby = find_program('ruby')
    run_command(ruby, meson.project_source_root() / 'scripts' / 'fetch_slint.rb', check: true)
  endif
endif

slint_compiler = find_program(slint_dir / 'bin' / 'slint-compiler')
slint_lib_dir = slint_dir / 'lib'

slint_dep = declare_dependency(
  compile_args: ['-isystem', slint_dir / 'include' / 'slint'],
  dependencies: [cpp.find_library('slint_cpp', dirs: slint_lib_dir, required: true)],
)

nlohmann_json_dep = dependency('nlohmann_json', fallback: ['nlohmann_json', 'nlohmann_json_dep'])
x11_dep = dependency('x11')

xdotool = find_program('xdotool', required: false)
if not xdotool.found()
  warning('xdotool not found: event sending will be disabled at runtime')
endif

ui_generated = custom_target(
  'slint-ui',
  input: files('ui/app.slint'),
  output: ['app.h', 'app.cpp'],
  depfile: 'app.d',
  command: [
    slint_compiler,
    '@INPUT@',
    '-f', 'cpp',
    '-o', '@OUTPUT0@',
    '--cpp-file=@OUTPUT1@',
    '--depfile', '@DEPFILE@',
    '--style', 'fluent',
    '--embed-resources=embed-files',
    '--translation-domain=event-timer',
  ],
)

event_timer = executable(
  'event-timer',
  [
    'src/main.cc',
    'src/countdown_timer.cc',
    'src/event_key.cc',
    'src/target_window.cc',
    'src/config.cc',
    'src/key_sender.cc',
    'src/x11_window.cc',
    'src/window_picker.cc',
    ui_generated,
  ],
  include_directories: [include_directories('.'), include_directories('src')],
  dependencies: [slint_dep, nlohmann_json_dep, x11_dep],
  build_rpath: slint_lib_dir,
)

if get_option('tests')
  doctest_dir = get_option('myfunc_dir')
  if doctest_dir == ''
    warning('myfunc_dir is not set, unit tests disabled (pass -Dmyfunc_dir=/path/to/cpp)')
  elif not fs.exists(doctest_dir / 'myFunc' / 'doctest.h')
    warning('doctest.h not found under ' + doctest_dir + ', unit tests disabled')
  else
    countdown_test = executable(
      'countdown-timer-test',
      ['src/countdown_timer_test.cc', 'src/countdown_timer.cc'],
      include_directories: [include_directories('src'), include_directories(doctest_dir)],
    )
    test('countdown-timer', countdown_test)

    event_key_test = executable(
      'event-key-test',
      [
        'src/event_key_test.cc',
        'src/target_window_test.cc',
        'src/config_test.cc',
        'src/event_key.cc',
        'src/target_window.cc',
        'src/config.cc',
      ],
      include_directories: [include_directories('src'), include_directories(doctest_dir)],
      dependencies: [nlohmann_json_dep],
    )
    test('event-key', event_key_test)
  endif
endif
