# Name #

**pdf2djvu** — creates DjVu files from PDF files

# Synopsis #

`pdf2djvu` _[_{`-o` _|_ `--output`_} output-djvu-file]_ _[option…]_ _pdf-file_

`pdf2djvu` _{_`-i` _|_ `--indirect`_} index-djvu-file_  _[option…]_ _pdf-file_

`pdf2djvu` _{_`--version` _|_ `--help` _|_ `-h`_}_

# Description #

This program creates a DjVu file from the Portable Document Format file _pdf-file_.

# Options #

**pdf2djvu** accepts the following options:

> Document type, file names:

> `-o`, `--output=`_output-djvu-file_
> > Generate a bundled multi-page document. Write the file into _output-djvu-file_ instead of standard output.


> `-i`, `--indirect=`_index-djvu-file_
> > Generate an indirect multi-page document. Use _index-djvu-file_ as the index file name; put the component files into the same directory. The directory must exist and be writable.


> `--pageid-template=`_template_
> > Specifies the naming scheme for page identifiers. Consult the “Template language” section for the template language description.
> > The default template is “`p{page:04*}.djvu`”.
> > For portability reasons, page identifiers:
      * must consist only of lowercase ASCII letters, digits, `_`, `+`, `-` and dot,
      * cannot start with a dot,
      * cannot contain two consecutive dots,
      * must end with the `.djvu` or the `.djv` extension.


> `--pageid-prefix=`_prefix_
> > Equivalent to “`--pageid-template=`_prefix_`{page:04*}.djvu`”.


> `--page-title-template=`_template_
> > Specifies the template for page titles. Consult the “Template language” section for the template language description.
> > The default is to set no page titles.


> Resolution, page size:

> `-d`, `--dpi=`_resolution_
> > Specifies the desired resolution to _resolution_ dots per inch. The default is 300 dpi. The allowed range is: 72 ≤ _resolution_ ≤ 6000.


> `--media-box`
> > Use _MediaBox_ to determine page size. _CropBox_ is used by default.


> `--page-size=`_width_`x`_height_
> > Specifies the preferred page size to _width_ pixels × _height_ pixels.  The actual page size may be altered in order to respect aspect ratio and DjVu limitations on resolution. (This option takes precedence over `-d`/`--dpi`.)


> `--guess-dpi`
> > Try to guess native resolution by inspecting embedded images. Use with care.


> Image quality:

> `--bg-slices=`_n_`+`_…_`+`_n_, `--bg-slices=`_n_`,`_…_`,n`
> > Specifies the encoding quality of the IW44 background layer. This option is similar to the `-slice` option of `c44`. Consult the [c44](http://djvu.sf.net/doc/man/c44.html)`(1)` manual page for details. The default is `72+11+10+10`.


> `--bg-subsample=`_n_
> > Specifies the background subsampling ratio. The default is 3. Valid values are integers between 1 and 12, inclusive.


> `--fg-colors=default`
> > Try to preserve all the foreground layer colors.
> > This is the default.


> `--fg-colors=web`
> > Reduce foreground layer colors to the web palette (216 colors).
> > This option is not recommended.


> `--fg-colors=`_n_
> > Use GraphicsMagick to reduce number of distinct colors in the foreground layer to _n_. Valid values are integers between 1 and 4080. This option is not recommended.


> `--fg-colors=black`
> > Discard any color information from the foreground layer.


> `--monochrome`
> > Render pages as monochrome bitmaps. With this option, `--bg-`_…_ and `--fg-`_…_ options are not respected.


> `--loss-level=`_n_
> > Specifies the aggressiveness of the lossy compression. The default is 0 (lossless). Valid values are integers between 0 and 200, inclusive. This option is similar to the `-losslevel` option of `cjb2`; consult the [cjb2](http://djvu.sourceforge.net/doc/man/cjb2.html)(1) manual page for details. This option is respected only along with the `--monochrome` option.


> `--lossy`
> > Synonym for `--loss-level=100`.


> `--anti-alias`
> > Enable font and vector anti-aliasing. This option is not recommended.


> Extraction:

> `--no-metadata`
> > Don't extract the metadata.
> > By default:
      * The following entries of the document information dictionary are extracted: `Title`, `Author`, `Subject`, `Creator`, `Producer`, `CreationDate`, `ModDate`. Timestamps are formatted according to [RFC 3999](https://www.ietf.org/rfc/rfc3339), with date and time components separated by a single space.
      * The XMP metadata is extracted (or created) and updated accordingly.


> `--verbatim-metadata`
> > Keep the original metadata intact.


> `--no-outline`
> > Don't extract the document outline.


> `--hyperlinks=border-avis`
> > Make hyperlink borders always visible.
> > By default, a hyperlink border is visible only when the mouse is over the hyperlink.


> `--hyperlinks=#`_RRGGBB_
> > Force the specified border color for hyperlinks.


> `--no-hyperlinks`, `--hyperlinks=none`
> > Don't extract hyperlinks.


> `--no-text`
> > Don't extract the text.


> `--words`
> > Extract the text. Record the location of every word. This is the default.


> `--lines`
> > Extract the text. Record the location of every line, rather that every word.


> `--crop-text`
> > Extract no text outside the page boundary.


> `--no-nfkc`
> > Don't [NFKC](http://unicode.org/reports/tr15/)-normalize the text.


> `--filter-text=`_command-line_
> > Filter the text through the _command-line_. The provided filter must preserve whitespace, control characters and decimal digits.
> > This option implies `--no-nfkc`.


> `-p`, `--pages=`_page-range_
> > Specifies pages to convert. _page-range_ is a comma-separated list of sub-ranges. Each sub-range is either a single page (e.g. `17`) or a contiguous range of pages (e.g. `37-42`). Pages are numbered from 1.
> > The default is to convert all pages.


> Performance:

> `-j`, `--jobs=`_n_
> > Use _n_ threads to perform conversion. The default is to use one thread.


> `-j0`, `--jobs=0`
> > Determine automatically how many threads to use to perform conversion.


> Verbosity, help:

> `-v`, `--verbose`
> > Display more informational messages while converting the file.


> `-q`, `--quiet`
> > Don't display informational messages while converting the file.


> `--version`
> > Output version information and exit.


> `-h`, `--help`
> > Display help and exit.

# Environment #


> `OMP_`**> > Details of runtime behaviour with respect to parallelism can be controlled by several environment variables. Please refer to the [OpenMP API specification](http://openmp.org/wp/openmp-specifications) for details.**

# Template language #

## Template syntax ##

The template language is roughly modelled on the [Python string formatting syntax](https://docs.python.org/library/string.html#format-string-syntax).

A template is a piece of text which contains fields, surrounded by curly braces `{``}`. Fields are replaced with appropriately formatted values when the template is evaluated. Moreover, `{{` is replaced with a single `{` and `}}` is replaced with a single `}`.

## Field syntax ##

Each field consists of a variable name, optionally followed by a shift, optionally followed by a format specification.

The shift is a signed (i.e. starting with a `+` or `-` character) integer.

The format specification consists of a colon, followed by a width specification.

The width specification is a decimal integer defining the minimum field width. If not specified, then the field width will be determined by the content. Preceding the width specification with a zero (`0`) character enables zero-padding.

The width specification is optionally followed by an asterisk (`*`) character, which increases the minimum field width to the width of the longest possible content of the variable.

## Available variables ##

`page`, `spage`

> Page number in the PDF document.

`dpage`
> Page number in the DjVu document.

# Implementation defails #

## Layer separation algorithm ##

Unless the `--monochrome` option is on, pdf2djvu uses the following naïve layer separation algorithm:

<ol>
<li>For each page, do the following:<br>
<ol>
<li>Raster the page into a pixmap, in the usual manner.</li>
<li>Raster the page into another pixmap, omitting the following page elements:<br>
<ul>
<li>text,</li>
<li>1 bit-per-pixel raster images,</li>
<li>vector elements (except fills of large areas).</li>
</ul>
</li>
<li>Compare both pixmaps, pixel by pixel:<br>
<ol>
<li>If their colors match, classify the pixel as a part of the background layer.</li>
<li>2. Otherwise, classify the pixel as a part of the foreground layer.</li>
</ol>
</li>
</ol>
</li>
</ol>

# Bug reports #

If you find a bug in **pdf2djvu**, please report it at [the issue tracker](https://code.google.com/p/pdf2djvu/issues/).

# See also #

[djvu](http://djvu.sf.net/doc/man/djvu.html)`(1)`,
[djvudigital](http://djvu.sf.net/doc/man/djvudigital.html)`(1)`,
[csepdjvu](http://djvu.sf.net/doc/man/csepdjvu.html)`(1)`

# Author #

[Jakub Wilk](mailto:jwilk@jwilk.net)

# Copyright #

Copyright © 2007, 2008, 2009, 2010 Jakub Wilk