djvudigital, a part of [DjVuLibre](http://djvu.sourceforge.net/), is another PDF to DjVu converter.
The list below outlines some differences between the converters.

# Superiorities of djvudigital #

djvudigital uses a [sophisticated layer separation algorithm](http://leon.bottou.org/papers/bottou-2001). pdf2djvu uses a simple one.

djvudigital is typically much faster than pdf2djvu.

djvudigital can convert PostScript files, too.

# Superiorities of pdf2djvu #

djvudigital internally relies on a [Ghostscript driver](http://djvu.sourceforge.net/gsdjvu.html), which has a license incompatibility issue. pdf2djvu is not affected by any license problem.

djvudigital cannot extract hidden text.

djvudigital cannot extract document metadata.

For some documents, djvudigital cannot extract document outline. See the [bug report](http://sourceforge.net/p/djvu/bugs/109/) for details.

djvudigital cannot produce indirect multi-page document by itself.

djvudigital always uses lossless compression for monochrome images.

With djvudigital, there is no obvious way to convert only some pages of a document.

With djvudigital, there is no way to customize page identifiers.
