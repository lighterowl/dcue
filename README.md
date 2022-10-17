# dcue

DCue is a tool for generating CUE sheets from Discogs data.

The original project was developed by Fluxtion on [Sourceforge](http://sourceforge.net/projects/dcue/). It originally used its own implementation of the HTTP protocol based on raw sockets in order to remain dependency-free. However, the Discogs' API HTTP endpoint has been switched off and the API is now accessible only via HTTPS. Since providing a custom HTTPS implementation would require using a SSL library anyway, I went ahead and ported the code to use libcurl (and later wininet). The custom HTTP implementation was removed.

Please report bugs and ideas via the GitHub issue tracker. Thanks!

# Compiling

Please note that due to image URLs being available only when the Discogs API is accessed via a registered application identified by its own key and secret, fetching images is only possible when running the official builds. Thus, you will not be able to use this functionality if you compile the application yourself (`--cover` and `--cover-file` will be ignored and won't even appear in the help output). If you need a build for another platform, please file an issue.

Also remember to include submodules in your clone : the easiest way to do that is to add `--recurse-submodules` to the `git clone` commandline.

## Linux

Make sure you have the libcurl headers. Most distributions ship these as a separate package, for example `libcurl-dev` on Debian and `libcurl-devel` on Red Hat.

```
mkdir build
cd build
cmake ..
make
```

## Windows

Just use CMake to generate the build files and build. If you use Visual Studio, chances are everything will work out of the box.

Windows builds use [WinInet](https://docs.microsoft.com/en-us/windows/win32/wininet/portal) to access the Discogs API and as such have no curl dependency.
