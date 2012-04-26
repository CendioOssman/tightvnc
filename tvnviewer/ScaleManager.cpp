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

#include "ScaleManager.h"

ScaleManager::ScaleManager() 
: m_scale(100), 
  m_scrWidth(1), 
  m_scrHeight(1), 
  m_scrWScale(1), 
  m_scrHScale(1), 
  m_xStart(0), 
  m_yStart(0), 
  m_iCentX(0), 
  m_iCentY(0)
{
}

int ScaleManager::sDiv(int x, int y, bool bIncr)
{
  if (bIncr)
    return (x + y - 1) / y;
  return x/y;
}

void ScaleManager::setScreenResolution(int maxWidth, int maxHeight)
{
  m_scrWidth = maxWidth;
  m_scrHeight = maxHeight;

  // part of Euclid algorithm
  while (!(maxWidth & 1) && !(maxHeight & 1)) {
    maxWidth = maxWidth >> 1;
    maxHeight = maxHeight >> 1;
  }

  maxWidth = maxWidth > 0 ? maxWidth : 1; 
  maxHeight = maxHeight > 0 ? maxHeight : 1;

  m_scrWScale = maxWidth;
  m_scrHScale = maxHeight;

  // renew other parameters
  setWindow(&m_rcWindow);
}

void ScaleManager::keepAspectRatio(Rect *rc)
{
  int iHeight = rc->getHeight() / m_scrHScale;
  int iWidth = rc->getWidth() / m_scrWScale;
  int iBar = min(iHeight, iWidth);
  rc->right = rc->left + iBar * m_scrWScale;
  rc->bottom = rc->top + iBar * m_scrHScale;
}

void ScaleManager::setScale(int scale) 
{
  m_scale = scale;
  setStartPoint(m_xStart, m_yStart);
}

void ScaleManager::setWindow(Rect *rcWnd) 
{
  m_rcWindow = rcWnd;
  setStartPoint(m_xStart, m_yStart);
}

void ScaleManager::calcScaled(const Rect *rcViewed, Rect *rcScaled, bool bCent) 
{
  // calculate scaled window from viewed
  rcScaled->setRect(rcViewed);
  int scale = m_scale;
  int denomeratorScale = 100;
  if (m_scale == -1) {
    if (m_rcWindow.getWidth() * m_scrHeight <= m_rcWindow.getHeight() * m_scrWidth) {
      scale = m_rcWindow.getWidth();
      denomeratorScale = m_scrWidth;
    } else {
      scale = m_rcWindow.getHeight();
      denomeratorScale = m_scrHeight;
    }
  }
  rcScaled->left = sDiv(rcScaled->left * scale, denomeratorScale, false);
  rcScaled->top = sDiv(rcScaled->top * scale, denomeratorScale, false);
  rcScaled->right = sDiv(rcScaled->right * scale, denomeratorScale, true);
  rcScaled->bottom = sDiv(rcScaled->bottom * scale, denomeratorScale, true);

  if (bCent) {
    if (m_rcWindow.getWidth() > rcScaled->getWidth()) {
      m_iCentX = (m_rcWindow.getWidth() - rcScaled->getWidth()) / 2;
    } else {
      m_iCentX = 0;
    }
    if (m_rcWindow.getHeight() > rcScaled->getHeight()) {
      m_iCentY = (m_rcWindow.getHeight() - rcScaled->getHeight()) / 2;
    } else {
      m_iCentY = 0;
    }
  }
}

bool ScaleManager::getVertPages(int iHeight) 
{
  if (m_scale == -1) {
    return false;
  }
  int lenScr = sDiv(m_scrHeight * m_scale, 100, true);
  int result = sDiv(lenScr, iHeight, true);
  if (result > 1) {
    return true;
  }
  return false;
}

bool ScaleManager::getHorzPages(int iWidth)
{
  if (m_scale == -1) {
    return false;
  }
  int lenScr = sDiv(m_scrWidth * m_scale, 100, true);
  int result = sDiv(lenScr, iWidth, true);
  if (result > 1) {
    return true;
  }
  return false;
}

int ScaleManager::getVertPoints()
{
  if (m_scale == -1) {
    return 0;
  }
  return sDiv(m_scrHeight * m_scale, 100, false);
}

int ScaleManager::getHorzPoints()
{
  if (m_scale == -1) {
    return 0;
  }
  return sDiv(m_scrWidth * m_scale, 100, false);
}

void ScaleManager::setStartPoint(int x, int y)
{
  int wndWidth = m_rcWindow.getWidth();
  int wndHeight = m_rcWindow.getHeight();

  if (m_scale != -1) {
    x = sDiv(x * 100, m_scale, false);
    y = sDiv(y * 100, m_scale, false);

    wndWidth = sDiv(wndWidth * 100, m_scale, false);
    wndHeight = sDiv(wndHeight * 100, m_scale, false);
  } else {
    // scroll is off, is scale is Auto
    x = y = 0;
    wndWidth = m_scrWidth;
    wndHeight = m_scrHeight;
 }

  m_xStart = x;
  m_yStart = y;
  m_rcViewed.left = x;
  m_rcViewed.top = y;
  m_rcViewed.right = x + wndWidth;
  m_rcViewed.bottom = y + wndHeight;

  if (m_rcViewed.right > m_scrWidth) {
    m_rcViewed.right = m_scrWidth;
    m_rcViewed.left = m_scrWidth - wndWidth;
    if (m_rcViewed.left < 0) {
      m_rcViewed.left = 0;
    }
  }
  if (m_rcViewed.bottom > m_scrHeight) {
    m_rcViewed.bottom = m_scrHeight;
    m_rcViewed.top = m_scrHeight - wndHeight;
    if (m_rcViewed.top < 0) {
      m_rcViewed.top = 0;
    }
  }
  calcScaled(&m_rcViewed, &m_rcScaled, true);
}

void ScaleManager::getViewedRect(Rect *rcViewed)
{
  Rect rect(&m_rcViewed);
  rect.setLocation(m_iCentX, m_iCentY);

  rcViewed->setRect(&rect);
}

void ScaleManager::getScaledRect(Rect *rcScaled)
{
  rcScaled->setRect(&m_rcScaled);
}

void ScaleManager::getDestinationRect(Rect *rcDestination)
{
  Rect rect(&m_rcScaled);
  rect.setLocation(m_iCentX, m_iCentY);

  rcDestination->setRect(&rect);
}

void ScaleManager::getSourceRect(Rect *rcSource)
{
  rcSource->setRect(&m_rcViewed);
}

void ScaleManager::getWndFromScreen(const Rect *screen, Rect *wnd) 
{
  Rect scr = screen;
  scr.move(-m_rcViewed.left, -m_rcViewed.top);
  calcScaled(&scr, wnd, false);
  wnd->move(m_iCentX, m_iCentY);
}

POINTS ScaleManager::transformDispToScr(int xPoint, int yPoint)
{
  xPoint -= m_iCentX;
  yPoint -= m_iCentY;

  int scale = m_scale;
  int denomeratorScale = 100;
  if (m_scale == -1) {
    if (m_rcWindow.getWidth() * m_scrHeight <= m_rcWindow.getHeight() * m_scrWidth) {
      scale = m_rcWindow.getWidth();
      denomeratorScale = m_scrWidth;
    } else {
      scale = m_rcWindow.getHeight();
      denomeratorScale = m_scrHeight;
    }
  }

  xPoint = sDiv(xPoint * denomeratorScale, scale, false);
  yPoint = sDiv(yPoint * denomeratorScale, scale, false);

  POINTS pt;
  pt.x = static_cast<SHORT>(xPoint + m_rcViewed.left);
  pt.y = static_cast<SHORT>(yPoint + m_rcViewed.top);

  return pt;
}
