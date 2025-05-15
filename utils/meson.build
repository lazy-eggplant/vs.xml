mio_dep = dependency('mio')

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
    'vs-xml.decode',
    'decode.cpp',
    install: true,
    dependencies: [
        vs_xml_dep,
        mio_dep,
    ],
)