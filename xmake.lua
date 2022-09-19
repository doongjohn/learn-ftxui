add_rules('mode.debug', 'mode.release')

set_languages('cxx20')
set_defaultplat('mingw')
set_defaultarchs('x86_64')

-- set_toolchains('clang')

package('ftxui')
  add_deps('cmake')
  set_sourcedir(path.join(os.scriptdir(), 'vendor/FTXUI'))
  on_install(function (package)
    local configs = {}
    import('package.tools.cmake').install(package, configs)
  end)
package_end()

add_requires(
  'ftxui',
  'fmt'
)

target('cpp')
  set_kind('binary')
  add_packages(
    'ftxui',
    'fmt'
  )
  add_files('src/*.cpp')
  add_ldflags(
    '-static',
    '-static-libgcc',
    '-static-libstdc++',
    '-lpthread'
  )
  add_includedirs(
    'vendor/FTXUI/include'
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
