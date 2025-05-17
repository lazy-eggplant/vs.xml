project(
  'vs-xml',
  ['cpp'],
  version: '0.1.14',
  meson_version: '>= 1.1',
  default_options: ['cpp_std=c++23'],
)

include_dirs = include_directories('include')

add_project_arguments(['-Wno-c23-extensions'], language: 'cpp')

if get_option('use_fmt')
  fmt_dep = dependency('fmt')
  use_fmt = ['-D', 'USE_FMT']
else 
  fmt_dep = []
  use_fmt = []
endif

vs_xml_lib = library(
    'vs-xml',
    [
      'lib/parser.cpp',
      'lib/serializer.cpp',
      'lib/tree.cpp',
      'lib/builder.cpp',
      'lib/document.cpp',
      'lib/world.cpp'
    ],
    cpp_args: [use_fmt],
    install: true,
    dependencies: [fmt_dep],
    include_directories: include_dirs,
)

vs_xml_dep = declare_dependency(
  link_with: vs_xml_lib,
  include_directories: ['include'],
)

install_subdir('include/vs-xml', install_dir : 'include', strip_directory: false )

if get_option('tests')
  subdir(['./test/'])
endif

if get_option('benchmarks')
  subdir(['./benchmark/'])
endif

if get_option('examples')
  subdir(['./examples/'])
endif

if get_option('utils')
  subdir(['./utils/'])
endif

pconf = import('pkgconfig')
pconf.generate(
  vs_xml_lib,
  description: 'XML parser and tree builder for limited mutability scenarios',
  url: 'https://github.com/lazy-eggplant/vs-xml',
  version: meson.project_version(),
)