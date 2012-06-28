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

#ifndef _DESKTOP_WINDOW_H_
#define _DESKTOP_WINDOW_H_

#include "win-system/WinClipboard.h"
#include "gui/DibFrameBuffer.h"
#include "region/Rect.h"
#include "region/Dimension.h"
#include "ScaleManager.h"
#include "client-config-lib/ConnectionConfig.h"
#include "gui/PaintWindow.h"
#include "gui/ScrollBar.h"
#include "gui/drawing/SolidBrush.h"
#include "gui/drawing/Graphics.h"
#include "rfb/RfbKeySym.h"
#include "viewer-core/RemoteViewerCore.h"

class DesktopWindow : public PaintWindow,
                      protected RfbKeySymListener 
{
public:
  DesktopWindow(LogWriter *logWriter, ConnectionConfig *conConf);
  virtual ~DesktopWindow();

  void setClipboardData(const StringStorage *strText);
  void updateFramebuffer(const FrameBuffer *framebuffer,
                         const Rect *dstRect);
  // this function must be called if size of image was changed
  // or the number of bits per pixel
  void setNewFramebuffer(const FrameBuffer *framebuffer);

  // set scale of image, can -1 = Auto, in percent
  void setScale(int scale);
  // it returns the image width and height considering scale
  Rect getViewerGeometry();
  // it returns the image width, height and number of bits per pixel
  void getServerGeometry(int *width, int *height, int *pixelsize);

  void setViewerCore(RemoteViewerCore *pViewerCore);

  // this function sends to remote viewer core
  // key what is pressed or unpressed
  void sendKey(WCHAR key, bool pressed);
  // this function sends to remote viewer core the combination
  // Ctrl + Alt + Del
  void sendCtrlAltDel();
  // inform the class we in full-screen mode or not
  // the difference is that in full-screen don't draw the scrollbars
  void setFullScreen(bool isFullScreen);

protected:
  bool onMessage(UINT message, WPARAM wParam, LPARAM lParam);
  void onPaint(DeviceContext *dc, PAINTSTRUCT *paintStruct);
  bool onCreate(LPCREATESTRUCT pcs);
  bool onDrawClipboard();
  bool onEraseBackground(HDC hdc);
  bool onDeadChar(WPARAM wParam, LPARAM lParam);
  bool onHScroll(WPARAM wParam, LPARAM lParam);
  bool onVScroll(WPARAM wParam, LPARAM lParam);
  bool onKey(WPARAM wParam, LPARAM lParam);
  bool onChar(WPARAM wParam, LPARAM lParam);
  bool onMouse(unsigned char msg, unsigned short wspeed, POINTS pt);
  bool onSize(WPARAM wParam, LPARAM lParam);
  bool onDestroy();
  POINTS getViewerCoord(long xPos, long yPos);
  void onRfbKeySymEvent(unsigned int rfbKeySym, bool down);
  void calculateWndSize(bool isChanged);
  void applyScrollbarChanges(bool isChanged, bool isVert, bool isHorz, int wndWidth, int wndHeight);

  LogWriter *m_logWriter;

  // keyboard support
  std::auto_ptr<RfbKeySym> m_rfbKeySym;

  // scroll bars: vertical and horizontal
  ScrollBar m_sbar;

  bool m_winResize;
  bool m_showVert;
  bool m_showHorz;

  // for scaling
  ScaleManager m_scManager;
  Rect m_clientArea;
  int m_fbWidth;
  int m_fbHeight;
  SolidBrush m_brush;

  // frame buffer
  LocalMutex m_bufferLock;
  DibFrameBuffer m_framebuffer;

  // clipboard
  WinClipboard m_clipboard;
  StringStorage m_strClipboard;

  RemoteViewerCore *m_pViewerCore;
  ConnectionConfig *m_conConf;
  bool m_isBackgroundDirty;
  bool m_isFullScreen;

private:
  void doDraw(DeviceContext *dc);
  void scrollProcessing(int fbWidth, int fbHeight);
  void drawBackground(DeviceContext *dc, const RECT *rcMain, const RECT *rcImage);
  void drawImage(const RECT *src, const RECT *dst);
  void repaint(const Rect *repaintRect);
  void calcClientArea();
};

#endif
