mio_dep = dependency('mio')
nanobench_dep = dependency('nanobench')
pugi_dep = dependency('pugixml')

benchmark('serialize-big',executable(
    'serialize-big',
    './src/serialize-big.cpp',
    install: false,
    cpp_args: [],
    link_args: [],
    dependencies: [
        vs_xml_dep,
        mio_dep,
        pugi_dep
    ],
))