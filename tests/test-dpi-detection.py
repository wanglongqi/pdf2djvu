# encoding=UTF-8

# Copyright © 2009-2015 Jakub Wilk
#
# This package is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 dated June, 1991.

import re

from common import (
    case,
)

class test(case):
    def test(self):
        self.pdf2djvu('--guess-dpi').assert_()
        r = self.djvudump()
        r.assert_(stdout=re.compile('INFO .* DjVu 100x200,'))

# vim:ts=4 sts=4 sw=4 et
