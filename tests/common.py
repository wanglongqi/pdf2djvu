#!/usr/bin/python
# encoding=UTF-8

# Copyright © 2009-2015 Jakub Wilk
#
# This package is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 dated June, 1991.

import inspect
import os
import re
import subprocess as ipc
import xml.etree.cElementTree as etree

re.compile.M = re.M
re = re.compile
re.type = type(re(''))

from nose import SkipTest

from nose.tools import (
    assert_true,
    assert_equal,
    assert_not_equal,
)

try:
    from nose.tools import assert_multi_line_equal
except ImportError:
    assert_multi_line_equal = assert_equal
else:
    assert_multi_line_equal.im_class.maxDiff = None

try:
    from nose.tools import assert_regexp_matches as assert_grep
except ImportError:
    def assert_grep(text, expected_regexp, msg=None):
        '''Fail the test unless the text matches the regular expression.'''
        if not isinstance(expected_regexp, re.type):
            expected_regexp = re(expected_regexp)
        if expected_regexp.search(text):
            return
        if msg is None:
            msg = "Regexp didn't match"
        msg = '%s: %r not found in %r' % (msg, expected_regexp.pattern, text)
        raise AssertionError(msg)

def assert_well_formed_xml(xml):
    try:
        etree.fromstring(xml)
    except SyntaxError, ex:
        raise AssertionError(ex)

class ipc_result(object):

    def __init__(self, stdout, stderr, rc):
        self.stdout = stdout
        self.stderr = stderr
        self.rc = rc

    def assert_(self, stdout='', stderr='', rc=0):
        if stderr is None:
            pass
        elif isinstance(stderr, re.type):
            assert_grep(self.stderr, stderr)
        else:
            assert_multi_line_equal(self.stderr, stderr)
        if rc is not None:
            assert_equal(self.rc, rc)
        if stdout is None:
            pass
        elif isinstance(stdout, re.type):
            assert_grep(self.stdout, stdout)
        else:
            assert_multi_line_equal(self.stdout, stdout)

class case(object):

    _pdf2djvu_command = os.getenv('pdf2djvu') or 'pdf2djvu'

    def get_pdf2djvu_command(self):
        return self._pdf2djvu_command

    def get_source_file(self, strip_py=False):
        result = inspect.getsourcefile(type(self))
        if strip_py and result.endswith('.py'):
            return result[:-3]
        else:
            return result

    def get_pdf_file(self):
        return self.get_source_file(strip_py=True) + '.pdf'

    def get_djvu_file(self):
        return self.get_source_file(strip_py=True) + '.djvu'

    def run(self, *commandline):
        env = dict(os.environ,
            MALLOC_CHECK_='3',
            MALLOC_PERTURB_=str(0xA5),
        )
        child = ipc.Popen(list(commandline),
            stdout=ipc.PIPE,
            stderr=ipc.PIPE,
            env=env,
        )
        stdout, stderr = child.communicate()
        stderr = re('^(?:  \S+ --> \S+ \(\d+ bytes\)\n)+$').sub('', stderr) # strip djvuextract cruft
        return ipc_result(stdout, stderr, child.returncode)

    def _pdf2djvu(self, *args):
        return self.run(
            self.get_pdf2djvu_command(),
            '-q',
            self.get_pdf_file(),
            *args
        )

    def pdf2djvu(self, *args):
        return self._pdf2djvu('-o', self.get_djvu_file(), *args)

    def pdf2djvu_indirect(self, *args):
        return self._pdf2djvu('-i', self.get_djvu_file(), *args)

    def djvudump(self, *args):
        return self.run('djvudump', self.get_djvu_file(), *args)

    def djvused(self, expr):
        return self.run(
            'djvused',
            '-e', expr,
            self.get_djvu_file()
        )

    def print_text(self):
        return self.run('djvutxt', self.get_djvu_file())

    def print_outline(self):
        return self.djvused('print-outline')

    def print_ant(self, page):
        return self.djvused('select %d; print-ant' % page)

    def print_meta(self):
        return self.djvused('print-meta')

    def decode(self):
        return self.run(
            'ddjvu',
            self.get_djvu_file()
        )

    def extract_xmp(self):
        r = self.djvused('output-ant')
        assert_equal(r.stderr, '')
        assert_equal(r.rc, 0)
        xmp_lines = [line for line in r.stdout.splitlines() if line.startswith('(xmp "')]
        if len(xmp_lines) == 0:
            return
        [xmp_line] = xmp_lines
        assert_equal(xmp_line[-2:], '")')
        xmp = xmp_line[5:-1]
        xmp = eval(xmp)
        return xmp

    def require_feature(self, feature):
        r = self.pdf2djvu('--version')
        r.assert_(stderr=re('^pdf2djvu '), rc=1)
        if feature not in r.stderr:
            raise SkipTest(feature + ' support missing')

def rainbow(width, height):
    from PIL import Image
    from PIL import ImageColor
    image = Image.new('RGB', (width, height))
    pixels = image.load()
    for x in xrange(width):
        for y in xrange(height):
            hue = 255 * x // (width - 1)
            luminance = 100 * y // height
            color = ImageColor.getrgb('hsl(%(hue)d, 100%%, %(luminance)d%%)' % locals())
            pixels[x, y] = color
    return image

def checkboard(width, height):
    from PIL import Image
    image = Image.new('1', (width, height))
    pixels = image.load()
    for x in xrange(width):
        for y in xrange(height):
            color = 0xff * ((x ^ y) & 1)
            pixels[x, y] = color
    return image

# vim:ts=4 sts=4 sw=4 et
