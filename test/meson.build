# Ignore these these tests if noexcept is set.
if get_option('noexcept') == false

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

    test('query-builder',executable(
        'query-builder',
        './src/query-builder.cpp',
        install: false,
        cpp_args: [],
        link_args: [],
        dependencies: [
            vs_xml_dep,
        ],
    ))

    test('binary-format',executable(
        'binary-format',
        './src/binary-format.cpp',
        install: false,
        cpp_args: [],
        link_args: [],
        dependencies: [
            vs_xml_dep,
        ],
    ))

    test('tree-iterator',executable(
        'tree-iterator',
        './src/tree-iterator.cpp',
        install: false,
        cpp_args: [],
        link_args: [],
        dependencies: [
            vs_xml_dep,
        ],
    ))
endif

