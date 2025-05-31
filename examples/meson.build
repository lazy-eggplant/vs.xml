mio_dep = dependency('mio')
gtl_dep = dependency('gtl')

executable(
    'readme',
    './src/readme.cpp',
    install: false,
    dependencies: [
        vs_xml_dep
    ],
)

executable(
    'tree-building',
    './src/build-tree.cpp',
    install: false,
    dependencies: [
        vs_xml_dep
    ],
)

executable(
    'document-building',
    './src/build-doc.cpp',
    install: false,
    dependencies: [
        vs_xml_dep
    ],
)


executable(
    'archive-building',
    './src/build-archive.cpp',
    install: false,
    dependencies: [
        vs_xml_dep
    ],
)

executable(
    'from-xml',
    './src/from-xml.cpp',
    install: false,
    dependencies: [
        vs_xml_dep,
        mio_dep
    ],
)

executable(
    'to-xml',
    './src/to-xml.cpp',
    install: false,
    dependencies: [
        vs_xml_dep,
        mio_dep
    ],
)

executable(
    'mmap',
    './src/mmap.cpp',
    install: false,
    dependencies: [
        vs_xml_dep,
        mio_dep
    ],
)

executable(
    'annotations',
    './src/annotations.cpp',
    install: false,
    dependencies: [
        vs_xml_dep,
        mio_dep,
        gtl_dep
    ],
)