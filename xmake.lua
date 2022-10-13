add_rules('mode.debug', 'mode.release')

set_defaultplat('mingw')
set_defaultarchs('x86_64')

set_languages('cxx20')

if not is_plat('mingw') then
  set_toolchains('clang')
end

-- build ftxui via cmake
package('ftxui')
  add_deps('cmake')
  set_sourcedir(path.join(os.scriptdir(), 'vendor/FTXUI'))
  on_install(function (package)
    local configs = {}
    table.insert(configs, '-DFTXUI_MICROSOFT_TERMINAL_FALLBACK=OFF')
    import('package.tools.cmake').install(package, configs)
  end)
package_end()

add_requires(
  'ftxui',
  'fmt'
)

target('learn_ftxui')
  set_kind('binary')
  add_ldflags('-lpthread')
  if is_plat('mingw') then
    add_ldflags(
      '-static',
      '-static-libgcc',
      '-static-libstdc++'
    )
  end
  add_files('src/*.cpp')
  add_includedirs(
    'vendor/FTXUI/include'
  )
  add_packages(
    'ftxui',
    'fmt'
  )
  -- set_optimize('fastest')
target_end()

task('install_vendor')
  set_menu {
    usage = 'xmake install_vendor',
    description = 'downloads vendor library',
    options = {}
  }
  on_run(function ()
    if not os.exists('vendor') then
      os.mkdir('vendor')
    end
    if os.exists('vendor') then
      os.cd('vendor')
      os.exec('git clone https://github.com/ArthurSonzogni/FTXUI')
    end
  end)
task_end()
