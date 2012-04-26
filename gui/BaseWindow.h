// Copyright (C) 2011,2012 GlavSoft LLC.
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

#ifndef _BASE_WINDOW_H_
#define _BASE_WINDOW_H_

#include "util/CommonHeader.h"

class BaseWindow
{
public:
  BaseWindow();

  virtual ~BaseWindow();

  // getWindow()
  // Get a handle of the window 
  HWND getHWnd();
  void setHWnd(HWND hwnd);

  // createWindow()
  // Create window with windowName and setted style
  // other parameters can by changed
  bool createWindow(const StringStorage *windowName, DWORD style, HWND hWndParent = 0,
                    int xPos = CW_USEDEFAULT, int yPos = CW_USEDEFAULT,
                    int width = CW_USEDEFAULT, int height = CW_USEDEFAULT);
  bool destroyWindow();

  // regClass()
  // Register class with className name for creating a window
  bool regClass(const StringStorage *className);

  // setClass()
  // Set a class name only to the new window created by createWindow
  void setClass(const StringStorage *className);

  // basic window manipulation procedures
  void show();
  void hide();
  void enableWindow(bool bEnable);
  void updateWindow();
  bool setSize(int width, int height);
  bool setPosition(int xPos, int yPos);
  void setWindowText(const StringStorage *text);

  // loadIcon()
  // Set the icon of application where id can be from resource or handle HICON
  void loadIcon(DWORD id);

  // setParent()
  // Making child window by changing parent of the window
  void setParent(HWND hwnd);

  // for changing registered class parameters of created window
  void setClassStyle(DWORD style);
  void setClassCursor(HCURSOR hcursor);
  void setClassBackground(HBRUSH hbrush);
  void setClassMenu(LONG menu);

  // for changing or get style and exstyle of window
  LONG getStyle();
  void setStyle(DWORD style);
  void setExStyle(DWORD exstyle);

  // processing window messages
  void run(bool processAll);

  // full redraw of window area
  void redraw(const RECT *rcArea = 0);

  // set or kill timer, with identifactor ident
  // and time in milliseconds
  void setTimer(UINT_PTR ident, UINT time);
  void killTimer(UINT_PTR ident);

  // set foreground window
  void setForegroundWindow();

  // working with accelerator
  void setAccTable(HACCEL hAccTable);

  // post message to this window
  void postMessage(UINT Msg, WPARAM wParam = 0, LPARAM lParam = 0);

  // quit from message loop
  void quit();

  void setChildDialog(HWND hwnd);
  void getClientRect(RECT *rc);
  void getBorderSize(int *width, int *height);

  static const int MOUSE_LDOWN  = 1;
  static const int MOUSE_MDOWN  = 2;
  static const int MOUSE_RDOWN  = 4;
  static const int MOUSE_WUP    = 8;
  static const int MOUSE_WDOWN  = 16;

private:
  // virtual functions
  virtual bool onCommand(WPARAM wParam, LPARAM lParam);
  virtual bool onNotify(int idCtrl, LPNMHDR pnmh);
  virtual bool onSysCommand(WPARAM wParam, LPARAM lParam);
  virtual bool onMessage(UINT message, WPARAM wParam, LPARAM lParam);
  virtual bool onMouse(unsigned char msg, unsigned short wspeed, POINTS pt);

protected:
  virtual bool wndProc(UINT message, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT message,
                                       WPARAM wParam, LPARAM lParam);

  HWND m_hWnd, m_hWndDialog;
  StringStorage m_className;
  StringStorage m_windowName;
  HICON m_hicon;
  bool m_bclassReg;
  bool m_bWndCreated;
  HACCEL m_hAccTable;
};

#endif
