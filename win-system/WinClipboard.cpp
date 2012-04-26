// Copyright (C) 2012 GlavSoft LLC.
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

#include "WinClipboard.h"

WinClipboard::WinClipboard(HWND hwnd) 
  : m_hndClipboard(0), m_hWnd(hwnd)
{
}

WinClipboard::~WinClipboard() 
{
  if (m_hndClipboard) {
    EmptyClipboard();
    GlobalFree(m_hndClipboard);
  }
}

void WinClipboard::setHWnd(HWND hwnd)
{
  m_hWnd = hwnd;
  SetClipboardViewer(m_hWnd);
}

bool WinClipboard::getString(StringStorage * str) 
{
  UINT strType = CF_UNICODETEXT;

  if (sizeof(TCHAR) == 1) {
    strType = CF_TEXT;
  }
  int uFormat = GetPriorityClipboardFormat(&strType, sizeof(UINT)); 

  if (!uFormat || uFormat == -1) {
     return false;
  }
  if (OpenClipboard(m_hWnd)) {
     HANDLE hndData = GetClipboardData(uFormat); 

     if (hndData) {
        TCHAR *szData = (TCHAR *)GlobalLock(hndData); 
        str->setString(szData);
        GlobalUnlock(hndData); 
        CloseClipboard();

        return true;
      }
    CloseClipboard();
    return false;
  }
  return false;
}

bool WinClipboard::setString(const StringStorage * str) 
{
  int dataType = CF_UNICODETEXT;

  if (sizeof(TCHAR) == 1) {
     dataType = CF_TEXT;
  }
  int strLength = static_cast<int>(str->getLength()) + 1;
  int dataSize = strLength * sizeof(TCHAR);

  if (OpenClipboard(m_hWnd)) {
     if (m_hndClipboard) {
         EmptyClipboard();
         GlobalFree(m_hndClipboard);
      }
      m_hndClipboard = GlobalAlloc(GMEM_MOVEABLE, dataSize);
      CopyMemory(GlobalLock(m_hndClipboard), str->getString(), dataSize);
      GlobalUnlock(m_hndClipboard);
      SetClipboardData(dataType, m_hndClipboard);
      CloseClipboard();
      return true;
  }
  return false;
}
