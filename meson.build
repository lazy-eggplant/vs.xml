project(
  'vs-xml',
  ['cpp'],
  version: '0.2.2',
  meson_version: '>= 1.1',
  default_options: ['cpp_std=c++23'],
)

include_dirs = include_directories('include')

add_project_arguments(['-Wno-c23-extensions'], language: 'cpp')

conf = configuration_data()
version_components = meson.project_version().split('.')
major_version = version_components[0]
minor_version = version_components[1]
rev_version = version_components[2]

conf = configuration_data()
conf.set('VS_XML_VERSION_MAJOR', major_version)
conf.set('VS_XML_VERSION_MINOR', minor_version)
conf.set('VS_XML_VERSION_REV', rev_version)
conf.set_quoted('VS_XML_VERSION', major_version + '.' + minor_version + '.' + rev_version)

if get_option('use_fmt')
  fmt_dep = dependency('fmt')
  conf.set('VS_XML_USE_FMT','true')
else 
  fmt_dep = []
endif

if get_option('use_gtl')
  gtl_dep = dependency('gtl')
  conf.set('VS_XML_USE_GTL','true')
else 
  gtl_dep = []
endif

configure_file(output : 'config.hpp',
  configuration : conf,
)

cfgfile = custom_target('make-config',
  input : './build/config.hpp',
  output : ['config.hpp.fake-meson'],
  command : ['cp', '@INPUT0@', '../include/vs-xml/config.hpp'],
  capture : true,
  install : false,
  build_always_stale: true
)

vs_xml_lib = library(
    'vs-xml',
    [
      'lib/parser.cpp',
      'lib/serializer.cpp',
      'lib/tree.cpp',
      'lib/builder.cpp',
      'lib/inplace-builder.cpp',
      'lib/query.cpp'
    ],
    cpp_args: [],
    install: true,
    dependencies: [fmt_dep, gtl_dep],
    link_depends: cfgfile,
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