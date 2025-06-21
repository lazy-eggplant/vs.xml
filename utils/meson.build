mio_dep = dependency('mio')
linenoise_dep = dependency('linenoise')

executable(
    'vs-xml.encode',
    'encode.cpp',
    install: true,
    dependencies: [
        vs_xml_dep,
        mio_dep,
    ],
)

executable(
    'vs-xml.encode-archive',
    'encode-archive.cpp',
    install: true,
    dependencies: [
        vs_xml_dep,
        mio_dep,
    ],
)

executable(
    'vs-xml.decode',
    'decode.cpp',
    install: true,
    dependencies: [
        vs_xml_dep,
        mio_dep,
    ],
)

executable(
    'vs-xml.decode-archive',
    'decode-archive.cpp',
    install: true,
    dependencies: [
        vs_xml_dep,
        mio_dep,
    ],
)

executable(
    'vs-xml.query',
    'query.cpp',
    install: true,
    dependencies: [
        vs_xml_dep,
        mio_dep,
        linenoise_dep,
    ],
)