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

#include "WindowsClipboard.h"
#include "util/Log.h"

const TCHAR WindowsClipboard::m_clipbWinClassName[] = _T("Tvnserver.Clipboard.Window");
const HINSTANCE WindowsClipboard::m_hinst = GetModuleHandle(0);

WindowsClipboard::WindowsClipboard(ClipboardListener *clipboardListener)
: Window(m_hinst, m_clipbWinClassName),
  m_hwndNextViewer(0),
  m_clipboardListener(clipboardListener)
{
  resume();
}

WindowsClipboard::~WindowsClipboard()
{
  terminate();
  wait();
}

bool WindowsClipboard::writeToClipBoard(const TCHAR *text)
{
  StringStorage clipboard;
  convertFromRfbFormat(text, &clipboard);
  if (OpenClipboard(m_hwnd)) {
    EmptyClipboard();

    size_t clipSize = clipboard.getSize();
    HGLOBAL hglb = GlobalAlloc(GMEM_MOVEABLE, clipSize);
    if (hglb) {
      TCHAR *buff = (TCHAR *)GlobalLock(hglb);
      memcpy(buff, clipboard.getString(), clipSize);
      GlobalUnlock(hglb);

#ifdef _UNICODE
      SetClipboardData(CF_UNICODETEXT, hglb);
#else
      SetClipboardData(CF_TEXT, hglb);
#endif
    }

    CloseClipboard();
  }

  return false;
}

void WindowsClipboard::readFromClipBoard(StringStorage *clipDest) const
{
  clipDest->setString(_T(""));
  if (!IsClipboardFormatAvailable(CF_TEXT) || !OpenClipboard(m_hwnd)) {
    return;
  }

  HANDLE hglb = GetClipboardData(CF_TEXT);
  if (hglb != NULL) {
    LPSTR lpstr = (LPSTR)GlobalLock(hglb);
    if (lpstr !=0 ) {
      clipDest->fromAnsiString(lpstr);
    }
  }
  CloseClipboard();
}

bool WindowsClipboard::wndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
  int fake = 3;
  switch (message)
  {
  case WM_CREATE:
    m_hwndNextViewer = SetClipboardViewer((HWND)wParam);
    break;

  case WM_CHANGECBCHAIN:
    if ((HWND) wParam == m_hwndNextViewer) {
      m_hwndNextViewer = (HWND) lParam;
    }
    else if (m_hwndNextViewer != NULL) {
      SendMessage(m_hwndNextViewer, message, wParam, lParam);
    }

    break;

  case WM_DESTROY:
    ChangeClipboardChain(m_hwnd, m_hwndNextViewer);
    break;

  case WM_DRAWCLIPBOARD:  
    {
      StringStorage winClip, rfbClip;
      readFromClipBoard(&winClip);
      convertToRfbFormat(&winClip, &rfbClip);

      m_clipboardListener->onClipboardUpdate(&rfbClip);
    }
    SendMessage(m_hwndNextViewer, message, wParam, lParam);
    break;

  default:
    return false; 
  }

  return true;
}

void WindowsClipboard::onTerminate()
{
  if (m_hwnd != 0) {
    PostMessage(m_hwnd, WM_QUIT, 0, 0);
  }
}

void WindowsClipboard::execute()
{
  Log::info(_T("clipboard thread id = %d"), getThreadId());

  if (!createWindow()) {
    return;
  }

  MSG msg;
  while (!isTerminating()) {
    if (GetMessage(&msg, m_hwnd, 0, 0)) {
      DispatchMessage(&msg);
    } else {
      break;
    }
  }

  destroyWindow();
}

void WindowsClipboard::convertToRfbFormat(const StringStorage *source,
                                          StringStorage *dest)
{
  const TCHAR *srcText = source->getString();
  size_t length = source->getLength();
  TCHAR *rfbText = new TCHAR[length + 1];

  size_t j = 0;
  for (size_t i = 0; i < length; i++) {
    if (!(srcText[i] == 0x0d && srcText[i + 1] == 0x0a)) {
      rfbText[j] = srcText[i];
      j++;
    }
  }
  rfbText[j] = 0;
  dest->setString(rfbText);
  delete[] rfbText;
}

void WindowsClipboard::convertFromRfbFormat(const TCHAR *source,
                                            StringStorage *dest)
{
  size_t lfCount = 0;
  size_t sourceLen = _tcslen(source);
  for (size_t i = 0; i < sourceLen; i++) {
    if (source[i] == 0x0a) {
      lfCount++;
    }
  }

  size_t destLen = sourceLen + lfCount;
  TCHAR *destText = new TCHAR[destLen + 1];
  int j = 0;
  for (size_t i = 0; i < sourceLen; i++) {
    if (source[i] == 0x0a) {
      destText[j] = 0x0d;
      j++;
    }
    destText[j] = source[i];
    j++;
  }
  destText[j] = 0;

  dest->setString(destText);
  delete[] destText;
}
