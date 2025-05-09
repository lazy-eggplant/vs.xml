## Installing

Use it as a meson dependency, be it a subproject of after installing it on your system.  

The process is basically as usual:

```sh
meson setup build           #Add more flags to setup release, lto optimizations etc based on your needs
meson install -C build      #You might have to define DESTDIR to perform a dry run or if you have a strange location
```

A simplified makefile is made available, which has some reasonable defaults.  

Tarballs are also offered for some distributions in releases.  

## Usage

TBW
