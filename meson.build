project(
  'vs-xml',
  ['cpp'],
  version: '0.1.4',
  meson_version: '>= 1.1',
  default_options: ['cpp_std=c++23'],
)

include_dirs = include_directories('include')

add_project_arguments(['-Wno-c23-extensions'], language: 'cpp')

vs_xml_lib = library(
    'vs-xml',
    [
      'src/parser.cpp',
      'src/serializer.cpp',
      'src/tree.cpp',
      'src/builder.cpp',
      'src/wrp-tree.cpp'
    ],
    install: true,
    dependencies: [],
    include_directories: include_dirs,
)

vs_xml_dep = declare_dependency(
  link_with: vs_xml_lib,
  include_directories: ['include'],
)

install_subdir('include/vs-xml', install_dir : 'include/vs-xml', strip_directory: false )

if get_option('tests')
  subdir(['./test/'])
endif

if get_option('benchmarks')
  subdir(['./benchmark/'])
endif

pconf = import('pkgconfig')
pconf.generate(
  vs_xml_lib,
  description: 'XML parser and tree builder for limited mutability scenarios',
  url: 'https://github.com/lazy-eggplant/vs-xml',
  version: meson.project_version(),
)