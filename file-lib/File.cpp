// Copyright (C) 2008, 2009, 2010 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the TightVNC software.  Please visit our Web site:
//
//                       http://www.tightvnc.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#include "File.h"

#include "util/DateTime.h"
#include <crtdbg.h>

TCHAR File::s_separatorChar = _T('\\');

File::File(const TCHAR *pathname)
{
  m_pathName.setString(pathname);
}

File::File(const TCHAR *parent, const TCHAR *child)
{
  m_pathName.setString(parent);

  if (!m_pathName.endsWith(File::s_separatorChar)) {

    TCHAR separator[2];
    separator[0] = File::s_separatorChar;
    separator[1] = '\0';

    m_pathName.appendString(&separator[0]);
  }

  m_pathName.appendString(child);
}

File::~File()
{
}

bool File::canRead() const
{
  return tryCreateFile(GENERIC_READ, OPEN_EXISTING);
}

bool File::canWrite() const
{
  return tryCreateFile(GENERIC_WRITE, OPEN_EXISTING);
}

bool File::createNewFile() const
{
  return tryCreateFile(GENERIC_READ | GENERIC_WRITE, CREATE_NEW);
}

bool File::remove() const
{
  if (!exists()) {
    return false;
  }
  if ((isDirectory()) && (RemoveDirectory(m_pathName.getString()) != 0)) {
    return true;
  }
  if (isFile() && DeleteFile(m_pathName.getString()) != 0) {
    return true;
  }
  return false;
}

bool File::exists() const
{
  WIN32_FIND_DATA fileInfo;
  return getFileInfo(&fileInfo);
}

void File::getName(StringStorage *name) const
{
  if (m_pathName.getLength() == 0) {
    name->setString(_T(""));
    return ;
  }

  const TCHAR *buffer = m_pathName.getString();
  bool separatorFound = false;
  size_t i = 0;

  for (i = m_pathName.getLength() - 1; i >= 0; i--) {
    if (buffer[i] == File::s_separatorChar) {
      separatorFound = true;
      break;
    } 
  } 

  m_pathName.getSubstring(name, i + 1, m_pathName.getLength() - 1);
}

void File::getFileExtension(StringStorage *ext) const
{
  StringStorage fileName;

  getName(&fileName);

  int pointPos = fileName.findLast(_T('.'));

  if (pointPos == -1) {
    ext->setString(_T(""));
  } else {
    fileName.getSubstring(ext, pointPos + 1, fileName.getLength() - 1);
  }
}

bool File::isFile() const
{
  if (exists() && !isDirectory()) {
    return true;
  }
  return false;
}

bool File::isDirectory() const
{
  WIN32_FIND_DATA fileInfo;
  if (getFileInfo(&fileInfo)) {
    if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      return true;
    } 
  } 
  return false;
}

void File::getPath(StringStorage *pathname) const
{
  *pathname = m_pathName;
}

UINT64 File::lastModified() const
{
  WIN32_FIND_DATA fileInfo;

  if (!getFileInfo(&fileInfo)) {
    return 0;
  }

  DateTime dt(fileInfo.ftLastWriteTime);

  return dt.getTime();
}

UINT64 File::length() const
{
  WIN32_FIND_DATA fileInfo;

  if (!getFileInfo(&fileInfo)) {
    return 0;
  }

  INT64 maxDWORDPlusOne = 1 + (INT64)MAXDWORD;

  return fileInfo.nFileSizeHigh * maxDWORDPlusOne + fileInfo.nFileSizeLow;
}

bool File::list(StringStorage *fileList, UINT32 *filesCount) const
{
  StringStorage folderPath(m_pathName.getString());
  folderPath.appendString(_T("\\*"));

  UINT32 index = 0;

  HANDLE hfile;
  WIN32_FIND_DATA findFileData;

  UINT savedErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

  hfile = FindFirstFile(folderPath.getString(), &findFileData);

  SetErrorMode(savedErrorMode);

  if (hfile == INVALID_HANDLE_VALUE) {
    return false;
  }

  do {

    if (_tcscmp(findFileData.cFileName, _T(".")) == 0 ||
        _tcscmp(findFileData.cFileName, _T("..")) == 0) {
      continue;
    }

    if (fileList != NULL) {
      fileList[index].setString(findFileData.cFileName);
    }

    index++;

  } while (FindNextFile(hfile, &findFileData));

  FindClose(hfile);

  if (fileList == NULL) {
    *filesCount = index;
  }

  return true;
}

bool File::listRoots(StringStorage *rootList, UINT32 *rootsCount)
{
  TCHAR drivesList[256];

  if (GetLogicalDriveStrings(255, drivesList) == 0) {
    return false;
  }

  UINT32 count = 0;
  int i = 0;

  while (drivesList[i] != '\0') {

    TCHAR *drive = _tcsdup(&drivesList[i]);
    TCHAR *backslash = _tcsrchr(drive, '\\');

    if (backslash != NULL) {
      *backslash = '\0';
    }

    if (rootList != NULL) {
      rootList[count].setString(drive);
    }

    free(drive);

    i += _tcscspn(&drivesList[i], _T("\0")) + 1;
    count++;
  } 

  if (rootList == NULL) {
    *rootsCount = count;
  }

  return true;
}

bool File::mkdir() const
{
  if (CreateDirectory(m_pathName.getString(), NULL) == 0) {
    return false;
  }
  return true;
}

bool File::renameTo(const TCHAR *destPathName)
{
  File destFile(destPathName);
  return renameTo(&destFile);
}

bool File::renameTo(File *dest)
{
  StringStorage destPathName;
  dest->getPath(&destPathName);
  if (MoveFile(m_pathName.getString(), destPathName.getString()) == 0) {
    return false;
  }
  return true;
}

bool File::setLastModified(INT64 time)
{
  _ASSERT(time >= 0);

  HANDLE hfile = CreateFile(m_pathName.getString(),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
  if (hfile == INVALID_HANDLE_VALUE) {
    return false;
  }

  FILETIME ft;

  DateTime dt((UINT64)time);

  dt.toFileTime(&ft);

  if (SetFileTime(hfile, NULL, NULL, &ft) == FALSE) {
    CloseHandle(hfile);
    return false;
  }

  CloseHandle(hfile);

  return true;
}

bool File::truncate()
{
  if (exists()) {
    if (!remove()) {
      return false;
    }
  }
  return createNewFile();
}

bool File::getFileInfo(WIN32_FIND_DATA *fileInfo) const
{
  HANDLE fileHandle;

  UINT savedErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

  fileHandle = FindFirstFile(m_pathName.getString(), fileInfo);

  SetErrorMode(savedErrorMode);

  if (fileHandle == INVALID_HANDLE_VALUE) {
    return false;
  } 

  FindClose(fileHandle);
  return true;
}

bool File::tryCreateFile(DWORD desiredAccess, DWORD creationDisposition) const
{
  HANDLE hfile = CreateFile(m_pathName.getString(),
                            desiredAccess,
                            0,
                            NULL,
                            creationDisposition,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

  if (hfile == INVALID_HANDLE_VALUE) {
    return false;
  }

  CloseHandle(hfile);

  return true;
}
