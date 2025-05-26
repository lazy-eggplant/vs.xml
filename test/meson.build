mio_dep = dependency('mio')

test('base-checks',executable(
    'base-checks',
    './src/base-checks.cpp',
    install: false,
    cpp_args: [],
    link_args: [],
    dependencies: [
        vs_xml_dep
    ],
))

test('parse-demo',executable(
    'parse-demo',
    './src/parse-demo.cpp',
    install: false,
    cpp_args: [],
    link_args: [],
    dependencies: [
        vs_xml_dep,
        mio_dep
    ],
))

test('escape',executable(
    'escape',
    './src/escape.cpp',
    install: false,
    cpp_args: [],
    link_args: [],
    dependencies: [
        vs_xml_dep,
    ],
))

test('query',executable(
    'query',
    './src/query.cpp',
    install: false,
    cpp_args: [],
    link_args: [],
    dependencies: [
        vs_xml_dep,
    ],
))

