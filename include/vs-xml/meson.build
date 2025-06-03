conf = configuration_data()
conf.set('VS_XML_VERSION_MAJOR', major_version)
conf.set('VS_XML_VERSION_MINOR', minor_version)
conf.set('VS_XML_VERSION_REV', rev_version)
conf.set_quoted('VS_XML_VERSION', major_version + '.' + minor_version + '.' + rev_version)
if get_option('noexcept')
  conf.set('VS_XML_NO_EXCEPT', 'true')
  #TODO: explicit check for other flags to notify they are incompatible with noexcept
  if get_option('utils') or get_option('benchmarks') or get_option('tests') or get_option('examples')
    error('Only the base library can be built if using `noexcept` set true')
  endif
endif

if get_option('noassert')
  conf.set('VS_XML_NO_ASSERT', 'true')
endif

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

cfgfile = configure_file(output : 'config.hpp',
  configuration : conf,
)

install_headers(cfgfile, subdir:'vs-xml')
install_subdir('.', install_dir : 'include/vs-xml', strip_directory: false, follow_symlinks: true, exclude_files: ['meson.build'] )
