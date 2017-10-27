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

Windows isn't my main development platform, so there might be a few things that
will probably seem a bit off to more seasoned developers. dcue is compiled with
Visual Studio 2015 on Windows 7, and that's where the official release builds
come from. The libcurl DLL is shipped alongside the main application.

1. Choose a directory where the libcurl build files will be stored. This
directory will be referred to as `CURL_PATH` from now on.
2. Download the [curl source distribution](https://curl.haxx.se/download/curl-7.56.1.zip)
and extract the `include` directory to `CURL_PATH`, thus placing the headers
into `CURL_PATH/include/curl`.
3. Download the [Windows binary distribution](https://skanthak.homepage.t-online.de/download/curl-7.56.1.cab)
and extract the contents of the `I386` directory into `CURL_PATH/lib`.
4. Run CMake and set `CURL_PATH` to your directory via the appropriate `-D`
option if you're using the commandline, or in the GUI after configuring.
5. Launch the *VS2015 x86 Native Tools Command Prompt*, go into the CMake build
directory and run `msbuild dcue.sln`.
