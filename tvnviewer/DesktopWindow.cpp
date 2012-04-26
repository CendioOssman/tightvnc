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

#include "DesktopWindow.h"

DesktopWindow::DesktopWindow(ConnectionConfig *conConf)
 : m_clipboard(0), 
   m_showVert(false), 
   m_showHorz(false), 
   m_scale(100), 
   m_fbWidth(1), 
   m_fbHeight(1), 
   m_winResize(false),
   m_conConf(conConf), 
   m_brush(RGB(0, 0, 0)),
   m_pViewerCore(0),
   m_isBackgroundDirty(false),
   m_isFullScreen(false)
{
  m_rfbKeySym = std::auto_ptr<RfbKeySym>(new RfbKeySym(this));
}

DesktopWindow::~DesktopWindow()
{
}

void DesktopWindow::setFullScreen(bool isFullScreen)
{
  m_isFullScreen = isFullScreen;
  m_sbar.setVirtualScroll(m_isFullScreen);
}

void DesktopWindow::setViewerCore(RemoteViewerCore *pViewerCore)
{
  m_pViewerCore = pViewerCore;
}

bool DesktopWindow::onCreate(LPCREATESTRUCT pcs)
{
  m_sbar.setWindow(getHWnd());
  m_clipboard.setHWnd(getHWnd());
  return true;
}

void DesktopWindow::onPaint(DeviceContext *dc, PAINTSTRUCT *paintStruct)
{
  AutoLock al(&m_bufferLock);
  Rect paintRect(&paintStruct->rcPaint);

  if (paintRect.area() != 0) {
    try {
      m_framebuffer.setTargetDC(paintStruct->hdc);
      if (!m_clientArea.isEmpty()) {
        doDraw(dc);
      }
    } catch (Exception &ex) {
      Log::error(_T("Error in onPaint: %s"), ex.getMessage());
    }
  }
}

bool DesktopWindow::onMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_HSCROLL:
      return onHScroll(wParam, lParam);
    case WM_VSCROLL:
      return onVScroll(wParam, lParam);
    case WM_ERASEBKGND:
      return onEraseBackground(reinterpret_cast<HDC>(wParam));
    case WM_DEADCHAR:
    case WM_SYSDEADCHAR:
      return onDeadChar(wParam, lParam);
    case WM_DRAWCLIPBOARD:
      return onDrawClipboard();
    case WM_CREATE:
      return onCreate(reinterpret_cast<LPCREATESTRUCT>(lParam));
    case WM_SIZE:
      return onSize(wParam, lParam);
    case WM_DESTROY:
      return onDestroy();
    case WM_CHAR:
    case WM_SYSCHAR:
      return onChar(wParam, lParam);
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
      return onKey(wParam, lParam);
    case WM_SETFOCUS:
      m_rfbKeySym->processFocusRestoration();
      return true;
    case WM_KILLFOCUS:
      m_rfbKeySym->processFocusLoss();
      return true;
  }
  return false;
}

bool DesktopWindow::onEraseBackground(HDC hdc) {
  return true;
}

bool DesktopWindow::onHScroll(WPARAM wParam, LPARAM lParam) 
{
  switch(LOWORD(wParam)) {
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
      m_sbar.setHorzPos(HIWORD(wParam));
      redraw();
      break;
    case SB_LINELEFT:
      m_sbar.moveLeftHorz(ScrollBar::SCROLL_STEP);
      redraw();
      break;
    case SB_PAGELEFT:   
      m_sbar.moveLeftHorz();
      redraw();
      break;
    case SB_LINERIGHT:
      m_sbar.moveRightHorz(ScrollBar::SCROLL_STEP);
      redraw();
      break;
    case SB_PAGERIGHT:    
      m_sbar.moveRightHorz();
      redraw();
      break;
  }
  return true;
}

bool DesktopWindow::onVScroll(WPARAM wParam, LPARAM lParam) 
{
  switch(LOWORD(wParam)) {
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
      m_sbar.setVertPos(HIWORD(wParam));
      redraw();
      break;
    case SB_LINEUP:
      m_sbar.moveDownVert(ScrollBar::SCROLL_STEP);
      redraw();
      break;
    case SB_PAGEUP:
      m_sbar.moveDownVert();
      redraw();
      break;
    case SB_LINEDOWN:
      m_sbar.moveUpVert(ScrollBar::SCROLL_STEP);
      redraw();
      break;
    case SB_PAGEDOWN:
      m_sbar.moveUpVert();
      redraw();
      break;
  }
  return true;
}

bool DesktopWindow::onMouse(unsigned char msg, unsigned short wspeed, POINTS pt) 
{
  POINT p;

  p.x = pt.x;
  p.y = pt.y;
  if (m_conConf->isViewOnly()) {
    return true;
  }

  if (m_conConf->isMouseSwapEnabled() && msg) {
    bool bSecond = !!(msg & (1 << 1));
    bool bThird  = !!(msg & (1 << 2));
    msg &= ~((1 << 1) | (1 << 2));
    if (bSecond) {  
      msg |= 1 << 2;
    }
    if (bThird) {
      msg |= 1 << 1;
    }
  }

  int mask = MOUSE_WUP | MOUSE_WDOWN;
  // we need to translate screen coordinate to client
  if (msg & mask) { 
    if (!ScreenToClient(getHWnd(), &p)) {
      p.x = -1; 
      p.y = -1;
    } 
  }
  POINTS mousePos = getViewerCoord(p.x, p.y); 
  Point pos;
  pos.x = mousePos.x;
  pos.y = mousePos.y;
  if (pos.y >= 0 && pos.x >= 0) {
     UINT8 keys;
     
     keys = msg;
     m_pViewerCore->sendPointerEvent(keys, &pos);
     if (msg & mask) {
        int i;

        m_pViewerCore->sendPointerEvent(keys & ~mask, &pos);
        for (i=0; i< wspeed-1; i++) {
          m_pViewerCore->sendPointerEvent(keys, &pos);
          m_pViewerCore->sendPointerEvent(keys & ~mask, &pos); 
        }
     }
  }
  return true;
}

bool DesktopWindow::onKey(WPARAM wParam, LPARAM lParam) 
{
  if (!m_conConf->isViewOnly()) {
    m_rfbKeySym->processKeyEvent(static_cast<unsigned short>(wParam), 
                                 static_cast<unsigned int>(lParam));   
  }
  return true;
}

bool DesktopWindow::onChar(WPARAM wParam, LPARAM lParam) 
{
  if (!m_conConf->isViewOnly()) {
    m_rfbKeySym->processCharEvent(static_cast<WCHAR>(wParam), 
                                  static_cast<unsigned int>(lParam));
  }
  return true;
}

bool DesktopWindow::onDeadChar(WPARAM wParam, LPARAM lParam) 
{
  return true;
}

bool DesktopWindow::onDrawClipboard()
{
  if (!IsWindowVisible(getHWnd()) || !m_conConf->isClipboardEnabled()) {
    return false;
  }
  StringStorage strTmp;
  if (m_clipboard.getString(&strTmp)) {

    if (strTmp == m_strClipboard) {
      m_strClipboard = _T("");
      return true;
    }
    m_strClipboard = _T("");
    m_pViewerCore->sendCutTextEvent(&strTmp);
  }
  return true;
}

void DesktopWindow::setClipboardData(const StringStorage * strText)
{
  if (m_conConf->isClipboardEnabled()) {
    m_clipboard.setString(strText);
    m_strClipboard.setString(strText->getString());
  }
}

void DesktopWindow::doDraw(DeviceContext *dc)
{
  int fbWidth  = m_framebuffer.getDimension().width;
  int fbHeight = m_framebuffer.getDimension().height;

  if (!fbWidth || !fbHeight) {
    Graphics graphics(dc);

    graphics.fillRect(m_clientArea.left, m_clientArea.top, 
                      m_clientArea.right, m_clientArea.bottom, &m_brush);
    return;
  }

  scrollProcessing(fbWidth, fbHeight);

  int iHorzPos = 0; 
  int iVertPos = 0; 

  if (m_showHorz) { 
    iHorzPos = m_sbar.getHorzPos();
  }

  if (m_showVert) {
    iVertPos = m_sbar.getVertPos();
  }

  m_scManager.setStartPoint(iHorzPos, iVertPos);

  Rect src, dst;
  m_scManager.getSourceRect(&src);
  m_scManager.getDestinationRect(&dst);

  int iWidth = m_clientArea.getWidth() - dst.getWidth();
  int iHeight = m_clientArea.getHeight() - dst.getHeight();

  if (iWidth || iHeight) {
    drawBackground(dc, &m_clientArea.toWindowsRect(), &dst.toWindowsRect());
  }

  drawImage(&src.toWindowsRect(), &dst.toWindowsRect());
}

void DesktopWindow::applyScrollbarChanges(bool isChanged, bool isVert, bool isHorz, int wndWidth, int wndHeight)
{
  if (m_showVert != isVert) {
    m_showVert = isVert;
    m_sbar.showVertScroll(m_showVert);
    isChanged = true;
  }

  if (m_showHorz != isHorz) {
    m_showHorz = isHorz;
    m_sbar.showHorzScroll(m_showHorz);
    isChanged = true;
  }

  if (isChanged) {
    m_scManager.setWindow(&Rect(0, 0, wndWidth, wndHeight));
    if (m_showVert) {
      m_sbar.setVertRange(0, m_scManager.getVertPoints(), wndHeight);
    }
    if (m_showHorz) {
      m_sbar.setHorzRange(0, m_scManager.getHorzPoints(), wndWidth);
    }
    calcClientArea();
  }
}

void DesktopWindow::calculateWndSize(bool isChanged)
{
  long hScroll = 0;
  long vScroll = 0;
  if (m_showVert) {
    vScroll = m_sbar.getVerticalSize();
  }
  if (m_showHorz) {
    hScroll = m_sbar.getHorizontalSize();
  }

  int wndWidth  = vScroll + m_clientArea.getWidth();
  int wndHeight = hScroll + m_clientArea.getHeight();
  bool isVert   = m_scManager.getVertPages(wndHeight);
  bool isHorz   = m_scManager.getHorzPages(wndWidth);

  if (isHorz) {
    wndHeight -= m_sbar.getHorizontalSize();
  }
  if (isVert) {
    wndWidth -= m_sbar.getVerticalSize();
  }

  if (isVert != isHorz) {
     // need to recalculate bHorz and bVert
    isVert = m_scManager.getVertPages(wndHeight);
    isHorz = m_scManager.getHorzPages(wndWidth);
    if (isHorz) {
      wndHeight -= m_sbar.getHorizontalSize();
    }
    if (isVert) {
      wndWidth -= m_sbar.getVerticalSize();
    }
  }
  applyScrollbarChanges(isChanged, isVert, isHorz, wndWidth, wndHeight);
}

void DesktopWindow::scrollProcessing(int fbWidth, int fbHeight)
{
  bool bChanged = false;

  if (m_winResize) {
    m_winResize = false;
    bChanged = true;
  }

  if (fbWidth != m_fbWidth || fbHeight != m_fbHeight) {
    bChanged = true;
    m_fbWidth = fbWidth;
    m_fbHeight = fbHeight;
    m_scManager.setScreenResolution(fbWidth, fbHeight);
  }

  calculateWndSize(bChanged);
}

void DesktopWindow::drawBackground(DeviceContext *dc, const RECT *rcMain, const RECT *rcImage) {
  Graphics graphics(dc);

  // top rectangle
  graphics.fillRect(rcMain->left, rcMain->top, rcMain->right, rcImage->top, &m_brush);
  // left rectangle
  graphics.fillRect(rcMain->left, rcImage->top, rcImage->left, rcImage->bottom, &m_brush);
  // bottom rectangle
  graphics.fillRect(rcMain->left, rcImage->bottom, rcMain->right, rcMain->bottom, &m_brush);
  // right rectangle
  graphics.fillRect(rcImage->right, rcImage->top, rcMain->right, rcImage->bottom, &m_brush);
}

void DesktopWindow::calcClientArea() {
  if (getHWnd()) {
    RECT rc;

    getClientRect(&rc);
    m_clientArea.fromWindowsRect(&rc);
    m_scManager.setWindow(&m_clientArea);
  }
}

void DesktopWindow::drawImage(const RECT *src, const RECT *dst)
{
  Rect rc_src(src);
  Rect rc_dest(dst);

  AutoLock al(&m_bufferLock);
  m_framebuffer.stretchFromDibSection(&rc_dest, &rc_src);
}

bool DesktopWindow::onSize(WPARAM wParam, LPARAM lParam) 
{
  calcClientArea();
  m_winResize = true; 
  return true;
}

bool DesktopWindow::onDestroy()
{
  return true;
}

void DesktopWindow::updateFramebuffer(const FrameBuffer *framebuffer,
                                     const Rect *dstRect)
{
  AutoLock al(&m_bufferLock);

  if (!m_framebuffer.copyFrom(dstRect, framebuffer, dstRect->left, dstRect->top)) {
    Log::error(_T("Possible invalide region. (%d, %d), (%d, %d)"),
               dstRect->left, dstRect->top, dstRect->right, dstRect->bottom);
    Log::interror(_T("Error in updateFramebuffer (ViewerWindow)"));
  }

  repaint(dstRect);
}

void DesktopWindow::setNewFramebuffer(const FrameBuffer *framebuffer)
{
  Dimension dimension = framebuffer->getDimension();
  Dimension olddimension = m_framebuffer.getDimension();

  bool isBackgroundDirty = dimension.width < olddimension.width || 
                           dimension.height < olddimension.height;

  Log::detail(_T("Desktop size: %d, %d"),  dimension.width, dimension.height);
  {
    AutoLock al(&m_bufferLock);
    if (!dimension.isEmpty()) {
      // the width and height should be aligned to 4
      int alignWidth = (dimension.width + 3) / 4;
      dimension.width = alignWidth * 4;
      int alignHeight = (dimension.height + 3) / 4;
      dimension.height = alignHeight * 4;
      m_framebuffer.setProperties(&dimension, 
                                  &framebuffer->getPixelFormat(), 
                                  getHWnd());
      m_framebuffer.setColor(0, 0, 0);
    }
  }
  if (dimension.isEmpty()) {
    repaint(&dimension.getRect());
  } else {
    m_isBackgroundDirty = isBackgroundDirty;
  }
}

void DesktopWindow::repaint(const Rect *repaintRect)
{
  Rect rect;
  m_scManager.getSourceRect(&rect);
  Rect paint = repaintRect;
  paint.intersection(&rect);

  // checks what we getted a valid rectangle
  if (paint.getWidth() <= 1 || paint.getHeight() <= 1 || m_isBackgroundDirty) {
    m_isBackgroundDirty = false;
    redraw();
    return;
  }
  Rect wnd;
  m_scManager.getWndFromScreen(&paint, &wnd);
  m_scManager.getDestinationRect(&rect);
  if (wnd.left) {
    --wnd.left;
  }
  if (wnd.top) {
    --wnd.top;
  }
  if (wnd.right < rect.right) {
    ++wnd.right;
  }
  if (wnd.bottom < rect.bottom) {
    ++wnd.bottom;
  }
  wnd.intersection(&rect);
  redraw(&wnd.toWindowsRect());
}

void DesktopWindow::setScale(int scale)
{
  AutoLock al(&m_bufferLock);
  m_scale = scale;
  m_scManager.setScale(m_scale);
  m_winResize = true;
}

POINTS DesktopWindow::getViewerCoord(long xPos, long yPos)
{
  Rect rect;
  POINTS p;

  m_scManager.getDestinationRect(&rect);
  // it checks this point in the rect
  if (!rect.isPointInRect(xPos, yPos)) {
    p.x = -1;
    p.y = -1;
    return p;
  }
  p = m_scManager.transformDispToScr(xPos, yPos);
  return p;
}

void DesktopWindow::getServerGeometry(int *width, int *height, int *pixelsize)
{
  AutoLock al(&m_bufferLock);
  if (width) {
    *width = m_framebuffer.getDimension().width;
  }
  if (height) {
    *height = m_framebuffer.getDimension().height;
  }
  if (pixelsize) {
    *pixelsize = m_framebuffer.getBitsPerPixel();
  }
}

void DesktopWindow::onRfbKeySymEvent(unsigned int rfbKeySym, bool down)
{
  if (m_pViewerCore) {
    m_pViewerCore->sendKeyboardEvent(down, rfbKeySym);
  }
}

void DesktopWindow::sendKey(WCHAR key, bool pressed)
{
  m_rfbKeySym->sendModifier(static_cast<unsigned char>(key), pressed); 
}

void DesktopWindow::sendCtrlAltDel()
{
  m_rfbKeySym->sendCtrlAltDel();
}
