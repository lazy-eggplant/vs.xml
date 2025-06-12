project(
  'vs-xml',
  ['cpp'],
  version: '0.2.10',
  meson_version: '>= 1.3',
  default_options: ['cpp_std=c++23'],
)

include_dirs = include_directories('include')

add_project_arguments(['-Wno-c23-extensions'], language: 'cpp')

conf = configuration_data()
version_components = meson.project_version().split('.')
major_version = version_components[0]
minor_version = version_components[1]
rev_version = version_components[2]

subdir('./include/vs-xml')

vs_xml_lib = library(
    'vs-xml',
    [
      'lib/archive.cpp',
      'lib/parser.cpp',
      'lib/serializer.cpp',
      'lib/tree.cpp',
      'lib/builder.cpp',
      'lib/inplace-builder.cpp',
      'lib/query.cpp',
      'lib/query-builder.cpp',
    ],
    cpp_args: [],
    install: true,
    dependencies: [fmt_dep, gtl_dep],
    include_directories: include_dirs,
)

vs_xml_dep = declare_dependency(
  link_with: vs_xml_lib,
  include_directories: ['include'],
)

# Ignore these targets if not the root project
if meson.is_subproject() == false

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

endif

pconf = import('pkgconfig')
pconf.generate(
  vs_xml_lib,
  description: 'High performance XML parser, tree builder an query system for (huge) trees with limited mutability',
  url: 'https://github.com/lazy-eggplant/vs-xml',
  version: meson.project_version(),
)