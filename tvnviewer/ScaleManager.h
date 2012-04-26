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

#ifndef __SCALE_MANAGER_H__
#define __SCALE_MANAGER_H__

#include "region/Rect.h"

class ScaleManager 
{
public:
  ScaleManager();

  // set resolution of the screen
  void setScreenResolution(int maxWidth, int maxHeight);
  // set the scale of image, for example, 10 - it means 10% from original
  void setScale(int scale);
  // set the rectangle of window
  void setWindow(Rect *rcWnd);

  // need to know for scrolling 
  // false -> single page
  // true -> multiple pages
  bool getVertPages(int iHeight);
  bool getHorzPages(int iWidth);

  // how much to scroll
  int getVertPoints();
  int getHorzPoints();

  // set the current starting point
  // point is scaled
  void setStartPoint(int x, int y);

  // get viewed rectangle
  void getViewedRect(Rect * rcViewed);

  // get scaled rectangle
  void getScaledRect(Rect *rcScaled);

  // get source rectangle
  void getSourceRect(Rect *rcSource);

  // get destination rectangle
  void getDestinationRect(Rect *rcDestination);
 
  // get window rectangle from screen
  void getWndFromScreen(const Rect *screen, Rect *wnd);

  // transform display coordinate to screen
  POINTS transformDispToScr(int xPoint, int yPoint);

protected:
  // this method return round of (x/y) if bIncr is false
  // and return x/y round to up
  int sDiv(int x, int y, bool bIncr);

  Rect m_rcWindow;
  Rect m_rcScaled;
  Rect m_rcViewed;

  int m_iCentX;
  int m_iCentY;

  int m_xStart;
  int m_yStart;

  int m_scrWidth;
  int m_scrHeight;

  int m_scrWScale;
  int m_scrHScale;

  int m_scale; 

private:
  void calcScaled(const Rect *rcViewed, Rect *rcScaled, bool bCent);
  void keepAspectRatio(Rect *rc);

};

#endif
