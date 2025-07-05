project(
  'vs-xml',
  ['cpp'],
  version: files('VERSION'),
  meson_version: '>= 1.3',
  default_options: ['cpp_std=c++23'],
)

add_project_arguments(['-Wno-c23-extensions'], language: 'cpp')

conf = configuration_data()
version_components = meson.project_version().split('.')
major_version = version_components[0]
minor_version = version_components[1]
rev_version = version_components[2]

subdir('./include/vs-xml')

if get_option('use_fmt')
  fmt_dep = dependency('fmt') #default_options : ['header-only=true']
else 
  fmt_dep = []
endif

if get_option('use_gtl')
  gtl_dep = dependency('gtl')
else 
  gtl_dep = []
endif


vs_xml_lib = library(
    'vs-xml',
    [
      'lib/parser.cpp',
      'lib/serializer.cpp',
      'lib/archive.cpp',
      'lib/tree.cpp',
      'lib/document.cpp',
      'lib/tree-builder.cpp',
      'lib/query.cpp',
      'lib/query-builder.cpp',
      'lib/node.cpp',
      'lib/wrp-node.cpp',
    ],
    cpp_args: [],
    install: true,
    dependencies: [fmt_dep, gtl_dep],
    include_directories: ['include'],
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