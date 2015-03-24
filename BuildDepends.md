# DjVuLibre #
[DjVuLibre](http://djvu.sourceforge.net/) is an implementation of [DjVu](http://djvu.org/), including viewers, browser plugins, decoders, simple encoders, and utilities.

## Compatibility ##
DjVuLibre 3.5.21 or newer is required.

# poppler #
[Poppler](http://poppler.freedesktop.org/) is a PDF rendering library derived from xpdf.

## Header files ##
You will need some `*.h` files which are not installed by default (`make install`) unless you use the `--enable-xpdf-headers` configure option. Alternatively, you may use the `tools/install-poppler-h-files` script to install them afterwards.

Note that some Linux distributions are already shipping those files.

## Compatibility ##

Poppler 0.11.1 or newer is required.

# PStreams (recommended) #
[PStreams](http://pstreams.sourceforge.net/) allows you to run another program from your C++ application and to transfer data between the two programs, similar to shell pipelines.

# GNOME XSLT library (recommended) #
[Libxslt](http://xmlsoft.org/xslt/) is the XSLT C library developed for the GNOME project.

This library is required to correctly deal with XMP metadata.

# GraphicsMagick (optional) #
[GraphicsMagick](http://www.graphicsmagick.org/) is a library to read, write, and manipulate images.

# an OpenMP implementation (recommended) #
[http://openmp.org/](OpenMP.md) is API for multi-platform shared-memory parallel programming.
