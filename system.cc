/* Copyright © 2007, 2008 Jakub Wilk
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991.
 */

#include "system.hh"
#include "debug.hh"

#include <cerrno>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iconv.h>

#ifndef WIFEXITED
#define WIFEXITED(x) 0
#endif

#ifndef WEXITSTATUS
#define WEXITSTATUS(x) (-1)
#endif

#ifdef WIN32

#include "windows.h"

#endif

/* constants
 * =========
 */

#ifndef WIN32
#define TEMPORARY_PATH_TEMPLATE "/tmp/pdf2djvu.XXXXXX"
#define PATH_SEPARATOR "/"
#define USE_UNIX_PATH_SEPARATOR 1
#else
#define TEMPORARY_PATH_TEMPLATE "pdf2djvu"
#define PATH_SEPARATOR "\\"
#define USE_UNIX_PATH_SEPARATOR 0
#endif


/* class OSError : Error
 * =====================
 */

std::string OSError::__error_message__(const std::string &context)
{
  std::string message = strerror(errno);
  if (context.length())
    message = context + ": " + message;
  return message;
}

void throw_os_error(const std::string &context)
{
  switch (errno)
  {
  case ENOTDIR:
    throw NotADirectory(context);
  case ENOENT:
    throw NoSuchFileOrDirectory(context);
  default:
    throw OSError(context);
  }
}

void warn_os_error(const std::string &context)
{
  try
  {
    throw_os_error(context); 
  }
  catch (const OSError &e)
  {
    std::cerr << "[Warning] " << e.what() << std::endl;
  }
}


/* class Command
 * =============
 */

Command::Command(const std::string& command) : command(command)
{
  if (this->argv.size() == 0 && !USE_UNIX_PATH_SEPARATOR)
  {
    /* Convert path separators: */
    std::ostringstream stream;
    for (std::string::const_iterator it = command.begin(); it != command.end(); it++)
    {
      if (*it == '/')
        stream << PATH_SEPARATOR;
      else
        stream << *it;
    }
    this->argv.push_back(stream.str());
  }
  else
    this->argv.push_back(command);
}

Command &Command::operator <<(const std::string& arg)
{
  this->argv.push_back(arg);
  return *this;
}

Command &Command::operator <<(int i)
{
  std::ostringstream stream;
  stream << i;
  return *this << stream.str();
}

void Command::operator()(std::ostream &my_stdout, bool quiet)
{
  this->call(&my_stdout, quiet);
}

void Command::operator()(bool quiet)
{
  this->call(NULL, quiet);
}

#ifndef HAVE_PSTREAMS
static const std::string argv_to_command_line(const std::vector<std::string> &argv)
/* Translate a sequence of arguments into a command line string. */
{
  std::ostringstream buffer;
#ifdef WIN32
  /* Using the same rules as the MS C runtime:
   *
   * 1) Arguments are delimited by white space, which is either a space or a
   *    tab.
   *
   * 2) A string surrounded by double quotation marks is interpreted as a
   *    single argument, regardless of white space contained within. A
   *    quoted string can be embedded in an argument.
   *
   * 3) A double quotation mark preceded by a backslash is interpreted as a
   *    literal double quotation mark.
   *
   * 4) Backslashes are interpreted literally, unless they immediately
   *    precede a double quotation mark.
   *
   * 5) If backslashes immediately precede a double quotation mark, every
   *    pair of backslashes is interpreted as a literal backslash.  If the
   *    number of backslashes is odd, the last backslash escapes the next
   *    double quotation mark as described in rule 3.
   *
   * See <http://msdn.microsoft.com/library/en-us/vccelng/htm/progs_12.asp>.
   */
  for (std::vector<std::string>::const_iterator parg = argv.begin(); parg != argv.end(); parg++)
  {
    int backslashed = 0;
    bool need_quote = parg->find_first_of(" \t") != std::string::npos;
    if (need_quote)
      buffer << '"';
    for (std::string::const_iterator pch = parg->begin(); pch != parg->end(); pch++)
    {
      if (*pch == '\\')
      {
        backslashed++;
      }
      else if (*pch == '"')
      {
        for (int i = 0; i < backslashed; i++)
          buffer << "\\\\";
        backslashed = 0;
        buffer << "\\\"";
      }
      else
      {
        for (int i = 0; i < backslashed; i++)
          buffer << '\\';
        backslashed = 0;
        buffer << *pch;
      }
    }
    for (int i = 0; i < backslashed; i++)
      buffer << '\\';
    if (need_quote)
      buffer << '"';
    buffer << ' ';
  }
#else
  /* Assume POSIX shell. */
  for (std::vector<std::string>::const_iterator parg = argv.begin(); parg != argv.end(); parg++)
  {
    buffer << "'";
    if (parg->find_first_of("\\\'") == std::string::npos) 
      buffer << *parg;
    else
      for (std::string::const_iterator pch = parg->begin(); pch != parg->end(); pch++)
      {
        if (*pch == '\\' || *pch == '\'')
          buffer << "'\"\\" << *pch << "\"'";
        else
          buffer << *pch;
      }
    buffer << "' ";
  }
#endif
  return buffer.str();
}
#endif

void Command::call(std::ostream *my_stdout, bool quiet)
{
  int status;
#ifdef HAVE_PSTREAMS
  redi::ipstream xsystem(this->command, this->argv, redi::pstream::pstdout | redi::pstream::pstderr);
  if (!xsystem.rdbuf()->error())
  {
    if (my_stdout != NULL)
    {
      xsystem.out();
      copy_stream(xsystem, *my_stdout, false);
    }
    {
      xsystem.err();
      copy_stream(xsystem, quiet ? dev_null : std::cerr, false);
    }
    xsystem.close();
  }
  status = xsystem.rdbuf()->status();
#else
  FILE *file;
  {
    const std::string &command_line = argv_to_command_line(this->argv);
    file = ::popen(command_line.c_str(), "r");
  }
  if (file != NULL)
  {
    char buffer[BUFSIZ];
    size_t nbytes;
    status = 0;
    while (!feof(file))
    {
      nbytes = fread(buffer, 1, sizeof buffer, file);
      if (ferror(file))
      {
        status = -1;
        break;
      }
      if (my_stdout != NULL)
        my_stdout->write(buffer, nbytes);
    }
    status = status || pclose(file);
  }
  else
    status = -1;
#endif
  if (status != 0)
  {
    std::ostringstream message;
    message << "system(\"";
    message << this->command;
    message << " ...\") failed";
    if (WIFEXITED(status))
      message << " with exit code " << WEXITSTATUS(status);
    throw CommandFailed(message.str());
  }
}


/* class Directory
 * ===============
 */

Directory::Directory(const std::string &name)
: name(name), posix_dir(NULL)
{ 
  this->open(name.c_str());
}

Directory::~Directory() throw ()
{
  this->close();
}

void Directory::open(const char* path)
{
  this->posix_dir = opendir(path);
  if (this->posix_dir == NULL)
    throw_os_error(path);
}

void Directory::close(void)
{
  if (this->posix_dir == NULL)
    return;
  if (closedir(static_cast<DIR*>(this->posix_dir)) != 0)
    throw_os_error(this->name);
}


/* class TemporaryDirectory : Directory
 * ====================================
 */

TemporaryDirectory::TemporaryDirectory() : Directory()
{
#ifndef WIN32
  char path_buffer[] = TEMPORARY_PATH_TEMPLATE;
  if (mkdtemp(path_buffer) == NULL)
    throw_os_error(path_buffer);
#else
  char base_path_buffer[PATH_MAX];
  char path_buffer[PATH_MAX];
  if (GetTempPath(PATH_MAX, base_path_buffer) == 0)
    throw_os_error("GetTempPath");
  if (GetTempFileNameA(base_path_buffer, TEMPORARY_PATH_TEMPLATE, 0, path_buffer) == 0)
    throw_os_error("GetTempFileNameA");
  if (unlink(path_buffer) < 0)
    throw_os_error(path_buffer);
  if (mkdir(path_buffer) < 0)
    throw_os_error(path_buffer);
#endif
  this->name += path_buffer;
}

TemporaryDirectory::~TemporaryDirectory() throw ()
{
  if (rmdir(this->name.c_str()) == -1)
    warn_os_error(this->name);
}


/* class File : std::fstream
 * =========================
 */

void File::open(const char* path)
{
  this->exceptions(std::ifstream::failbit | std::ifstream::badbit);
  if (path == NULL)
    this->std::fstream::open(this->name.c_str(), std::fstream::in | std::fstream::out | std::fstream::binary);
  else
  {
    this->name = path;
    this->std::fstream::open(path, std::fstream::in | std::fstream::out | std::fstream::trunc | std::fstream::binary);
  }
  this->exceptions(std::ifstream::badbit);
}

File::File(const std::string &name)
{
  this->open(name.c_str());
}

File::File(const Directory& directory, const std::string &name)
{
  std::ostringstream stream;
  stream << directory << PATH_SEPARATOR << name;
  this->open(stream.str().c_str());
}

size_t File::size()
{
  this->seekg(0, std::ios::end);
  return this->tellg();
}

void File::reopen()
{
  if (this->is_open())
    this->close();
  this->open(NULL);
}

File::operator const std::string& () const
{
  return this->name;
}

std::ostream &operator<<(std::ostream &out, const Directory &directory)
{
  return out << directory.name;
}

std::ostream &operator<<(std::ostream &out, const File &file)
{
  return out << file.name;
}


/* class TemporaryFile : File
 * ==========================
 */

void TemporaryFile::construct()
{
#ifndef WIN32
  char path_buffer[] = TEMPORARY_PATH_TEMPLATE;
  int fd = mkstemp(path_buffer);
#else
  char base_path_buffer[PATH_MAX];
  char path_buffer[PATH_MAX];
  if (GetTempPath(PATH_MAX, base_path_buffer) == 0)
    throw_os_error("GetTempPath");
  if (GetTempFileNameA(base_path_buffer, TEMPORARY_PATH_TEMPLATE, 0, path_buffer) == 0)
    throw_os_error("GetTempFileNameA");
  int fd = ::open(path_buffer, O_RDWR | O_BINARY);
#endif
  if (fd == -1)
    throw_os_error(path_buffer);
  if (::close(fd) == -1)
    throw_os_error(path_buffer);
  this->open(path_buffer);
}

TemporaryFile::TemporaryFile()
{
  this->construct();
}

TemporaryFile::~TemporaryFile() throw ()
{
  if (this->is_open())
    this->close();
  if (unlink(this->name.c_str()) == -1)
    warn_os_error(this->name);
}


/* class ExistingFile : File
 * =========================
 */

ExistingFile::ExistingFile(const std::string &name) 
: File()
{
  this->name = name;
  this->open(NULL);
}

ExistingFile::ExistingFile(const Directory& directory, const std::string &name)
: File()
{
  std::ostringstream stream;
  stream << directory << "/" << name;
  this->name = stream.str();
  this->open(NULL);
}


/* utility functions
 * =================
 */

void utf16_to_utf8(const char *inbuf, size_t inbuf_len, std::ostream &stream)
{
  char outbuf[BUFSIZ];
  char *outbuf_ptr = outbuf;
  size_t outbuf_len = sizeof outbuf;
  iconv_t cd = iconv_open("UTF-8", "UTF-16");
  if (cd == reinterpret_cast<iconv_t>(-1))
    throw_os_error("iconv_open()");
  while (inbuf_len > 0)
  {
    struct iconv_adapter 
    {
      // http://wang.yuxuan.org/blog/2007/7/9/deal_with_2_versions_of_iconv_h
      iconv_adapter(const char** s) : s(s) {}
      iconv_adapter(char** s) : s(const_cast<const char**>(s)) {}
      operator char**() const
      {
        return const_cast<char**>(s);
      }
      operator const char**() const
      {
        return const_cast<const char**>(s);
      }
      const char** s;
    };

    size_t n = iconv(cd, iconv_adapter(&inbuf), &inbuf_len, &outbuf_ptr, &outbuf_len);
    if (n == (size_t) -1 && errno == E2BIG)
    {
      stream.write(outbuf, outbuf_ptr - outbuf);
      outbuf_ptr = outbuf;
      outbuf_len = sizeof outbuf;
    }
    else if (n == (size_t) -1)
      throw IconvError();
  }
  stream.write(outbuf, outbuf_ptr - outbuf);
  if (iconv_close(cd) == -1)
    throw_os_error("iconv_close()");
}

void copy_stream(std::istream &istream, std::ostream &ostream, bool seek)
{
  if (seek)
    istream.seekg(0, std::ios::beg);
  char buffer[BUFSIZ];
  while (!istream.eof())
  {
    istream.read(buffer, sizeof buffer);
    ostream.write(buffer, istream.gcount());
  }
}

void copy_stream(std::istream &istream, std::ostream &ostream, bool seek, std::streamsize limit)
{
  if (seek)
    istream.seekg(0, std::ios::beg);
  char buffer[BUFSIZ];
  while (!istream.eof() && limit > 0)
  {
    std::streamsize chunk_size = std::min(static_cast<std::streamsize>(sizeof buffer), limit);
    istream.read(buffer, chunk_size);
    ostream.write(buffer, istream.gcount());
    limit -= chunk_size;
  }
}

bool is_stream_a_tty(const std::ostream &ostream)
{
  if (&ostream == &std::cout)
    return isatty(STDOUT_FILENO);
  else
  {
    // Not implemented for `ostream != std::count`.
    // See <http://www.ginac.de/~kreckel/fileno/> for a more general
    // (although terribly unportable) solution.
    throw std::invalid_argument("is_a_tty(const std::ostream &)");
  }
}

// vim:ts=2 sw=2 et
