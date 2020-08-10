# dcue

DCue is a tool for generating CUE sheets from Discogs data.

The original project was developed by Fluxtion on
[Sourceforge](http://sourceforge.net/projects/dcue/). It originally used its own
implementation of the HTTP protocol based on raw sockets in order to remain
dependency-free. However, the Discogs' API HTTP endpoint has been switched off
and the API is now accessible only via HTTPS. Since providing a custom HTTPS
implementation would require using a SSL library anyway, I went ahead and ported
the code to use libcurl. The custom HTTP implementation was removed.

Please report bugs and ideas via the GitHub issue tracker. Thanks!

# Compiling
## Linux

Make sure you have the libcurl headers. Most distributions ship these as a
separate package, for example `libcurl-dev` on Debian and `libcurl-devel` on Red
Hat.

```
mkdir build
cd build
cmake ..
make
```

## Windows

Get [vcpkg](https://github.com/microsoft/vcpkg) and use it to compile and install libcurl :

```
vcpkg install curl:x86-windows
```

Then, use CMake to generate the build files and build. If you use Visual Studio
, chances are everything will work out of the box if you took the extra steps
to integrate VS with vcpkg, as described in vcpkg's readme.

The official builds use a statically linked libcurl for convenience.
