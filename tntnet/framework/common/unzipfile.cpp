/* unzipfile.cpp
   Copyright (C) 2003-2006 Tommi Maekitalo

This file is part of tntnet.

Tntnet is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Tntnet is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with tntnet; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330,
Boston, MA  02111-1307  USA
*/

#include "tnt/unzipfile.h"
#include <sstream>
#include "unzip.h"

namespace tnt
{
  namespace
  {
    int checkError(int ret, const char* function)
    {
      if (ret < 0)
      {
        switch (ret)
        {
          case UNZ_END_OF_LIST_OF_FILE:
            throw unzipEndOfListOfFile(function);

          case UNZ_PARAMERROR:
            throw unzipParamError(function);

          case UNZ_BADZIPFILE:
            throw unzipBadZipFile(function);

          case UNZ_INTERNALERROR:
            throw unzipInternalError(function);

          case UNZ_CRCERROR:
            throw unzipCrcError(function);
        }

        throw unzipError(ret, "unknown error", function);
      }
      return ret;
    }

  }

  std::string unzipError::formatMsg(int err, const char* msg, const char* function)
  {
    std::ostringstream s;
    s << "unzip-error " << err;
    if (function && function[0])
      s << " in function \"" << function << '"';
    s << ": " << msg;
    return s.str();
  }

  unzipEndOfListOfFile::unzipEndOfListOfFile(const char* function)
    : unzipError(UNZ_END_OF_LIST_OF_FILE, "end of list of file", function)
    { }

  unzipParamError::unzipParamError(const char* function)
    : unzipError(UNZ_PARAMERROR, "parameter error", function)
    { }

  unzipBadZipFile::unzipBadZipFile(const char* function)
    : unzipError(UNZ_PARAMERROR, "bad zip file", function)
    { }

  unzipInternalError::unzipInternalError(const char* function)
    : unzipError(UNZ_PARAMERROR, "internal error", function)
    { }

  unzipCrcError::unzipCrcError(const char* function)
    : unzipError(UNZ_PARAMERROR, "crc error", function)
    { }

  //////////////////////////////////////////////////////////////////////
  // unzipFile
  //

  struct unzipFile::unzFileStruct
  {
    unzFile file;
  };

  void unzipFile::open(const std::string& path)
  {
    close();
    file = new unzFileStruct;
    if (!(file->file = ::unzOpen(path.c_str())))
    {
      delete file;
      file = 0;
      throw std::runtime_error("error opening " + path);
    }
  }

  void unzipFile::close()
  {
    if (file)
    {
      unzClose(file->file);
      delete file;
      file = 0;
    }
  }

  unzipFile::~unzipFile()
  {
    if (file)
    {
      unzClose(file->file);
      delete file;
    }
  }

  void unzipFile::goToFirstFile()
  {
    checkError(::unzGoToFirstFile(file->file), "unzGoToFirstFile");
  }

  void unzipFile::goToNextFile()
  {
    checkError(::unzGoToNextFile(file->file), "unzGoToNextFile");
  }

  void unzipFile::locateFile(const std::string& fileName, bool caseSensitivity)
  {
    checkError(::unzLocateFile(file->file, fileName.c_str(), caseSensitivity ? 1 : 0), "unzLocateFile");
  }

  void unzipFile::openCurrentFile()
  {
    checkError(::unzOpenCurrentFile(file->file), "unzOpenCurrentFile");
  }

  void unzipFile::openCurrentFile(const std::string& pw)
  {
    checkError(::unzOpenCurrentFilePassword(file->file, pw.c_str()), "unzOpenCurrentFilePassword");
  }

  void unzipFile::closeCurrentFile()
  {
    checkError(::unzCloseCurrentFile(file->file), "unzCloseCurrentFile");
  }

  int unzipFile::readCurrentFile(void* buf, unsigned len)
  {
    return checkError(::unzReadCurrentFile(file->file, buf, len), "unzReadCurrentFile");
  }

  //////////////////////////////////////////////////////////////////////
  // unzipFileStreamBuf
  //

  unzipFileStreamBuf::int_type unzipFileStreamBuf::overflow(unzipFileStreamBuf::int_type c)
  { return traits_type::eof(); }

  unzipFileStreamBuf::int_type unzipFileStreamBuf::underflow()
  {
    int n = file.readCurrentFile(buffer, sizeof(buffer));
    if (n == 0)
      return traits_type::eof();
    setg(buffer, buffer, buffer + n);
    return traits_type::to_int_type(buffer[0]);
  }

  int unzipFileStreamBuf::sync()
  {
    return 0;
  }
}
