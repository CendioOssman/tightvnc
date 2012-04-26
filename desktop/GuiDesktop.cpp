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

#include "GuiDesktop.h"
#include "log-server/Log.h"
#include "util/BrokenHandleException.h"

GuiDesktop::GuiDesktop(ClipboardListener *extClipListener,
                       UpdateSendingListener *extUpdSendingListener,
                       AbnormDeskTermListener *extDeskTermListener)
: m_extUpdSendingListener(extUpdSendingListener),
  m_extDeskTermListener(extDeskTermListener),
  m_extClipListener(extClipListener),
  m_userInput(0),
  m_updateHandler(0)
{
}

GuiDesktop::~GuiDesktop()
{
}

void GuiDesktop::getCurrentUserInfo(StringStorage *desktopName,
                                    StringStorage *userName)
{
  _ASSERT(m_userInput != 0);
  _ASSERT(m_extDeskTermListener != 0);
  Log::info(_T("get current user information"));
  try {
    m_userInput->getCurrentUserInfo(desktopName, userName);
  } catch (...) {
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
}

void GuiDesktop::getFrameBufferProperties(Dimension *dim, PixelFormat *pf)
{
  _ASSERT(m_updateHandler != 0);
  _ASSERT(m_extDeskTermListener != 0);
  Log::info(_T("get frame buffer properties"));
  try {
    m_updateHandler->getFrameBufferProp(dim, pf);
  } catch (...) {
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
}

void GuiDesktop::getPrimaryDesktopCoords(Rect *rect)
{
  _ASSERT(m_userInput != 0);
  _ASSERT(m_extDeskTermListener != 0);
  Log::info(_T("get primary desktop coordinates"));
  try {
    m_userInput->getPrimaryDisplayCoords(rect);
  } catch (...) {
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
}

void GuiDesktop::getDisplayNumberCoords(Rect *rect,
                                        unsigned char dispNumber)
{
  _ASSERT(m_userInput != 0);
  _ASSERT(m_extDeskTermListener != 0);
  Log::info(_T("get the %u display coordinates"), (unsigned int) dispNumber);
  try {
    m_userInput->getDisplayNumberCoords(rect, dispNumber);
  } catch (...) {
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
}

void GuiDesktop::getWindowCoords(HWND hwnd, Rect *rect)
{
  _ASSERT(m_userInput != 0);
  _ASSERT(m_extDeskTermListener != 0);
  Log::info(_T("get window coordinates"));
  try {
    m_userInput->getWindowCoords(hwnd, rect);
  } catch (BrokenHandleException &) {
    throw;
  } catch (...) {
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
}

HWND GuiDesktop::getWindowHandleByName(const StringStorage *windowName)
{
  _ASSERT(m_userInput != 0);
  _ASSERT(m_extDeskTermListener != 0);
  Log::info(_T("get a window handle by a window name"));
  try {
    return m_userInput->getWindowHandleByName(windowName);
  } catch (...) {
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
  return 0;
}

bool GuiDesktop::isRemoteInputAllowed()
{
  Log::info(_T("checking remote input allowing"));

  bool enabled = !Configurator::getInstance()->getServerConfig()->isBlockingRemoteInput();
  enabled = enabled && !isRemoteInputTempBlocked();
  return enabled;
}

void GuiDesktop::setKeyboardEvent(UINT32 keySym, bool down)
{
  _ASSERT(m_userInput != 0);
  _ASSERT(m_extDeskTermListener != 0);

  Log::info(_T("set keyboard event (keySym = %u, down = %d)"), keySym, (int)down);
  try {
    if (isRemoteInputAllowed()) {
      m_userInput->setKeyboardEvent(keySym, down);
    }
  } catch (Exception &e) {
    Log::error(_T("setKeyboardEvent() crashed: %s"), e.getMessage());
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
}

void GuiDesktop::setMouseEvent(UINT16 x, UINT16 y, UINT8 buttonMask)
{
  _ASSERT(m_userInput != 0);
  _ASSERT(m_extDeskTermListener != 0);

  Log::info(_T("set mouse event (x = %u, y = %u)"), (UINT32)x, (UINT32)y);
  Point point(x, y);
  try {
    if (isRemoteInputAllowed()) {
      m_userInput->setMouseEvent(&point, buttonMask);
    }
  } catch (...) {
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }
}

void GuiDesktop::setNewClipText(const StringStorage *newClipboard)
{
  _ASSERT(m_userInput != 0);
  _ASSERT(m_extDeskTermListener != 0);

  Log::info(_T("set new clipboard text"));

  bool isEqual;
  {
    AutoLock al(&m_storedClipCritSec);
    isEqual = m_sentClip.isEqualTo(newClipboard);
    isEqual = isEqual || m_receivedClip.isEqualTo(newClipboard);
  }
  if (!isEqual) {
    {
      AutoLock al(&m_storedClipCritSec);
      m_receivedClip = *newClipboard;
    }
    try {
      m_userInput->setNewClipboard(newClipboard);
    } catch (...) {
      m_extDeskTermListener->onAbnormalDesktopTerminate();
    }
  }
}

void GuiDesktop::sendUpdate()
{
  _ASSERT(m_updateHandler != 0);
  _ASSERT(m_extDeskTermListener != 0);
  _ASSERT(m_extUpdSendingListener != 0);

  if (!m_extUpdSendingListener->isReadyToSend()) {
    Log::info(_T("nobody is ready for updates"));
    return;
  }
  UpdateContainer updCont;
  try {
    if (!m_fullReqRegion.isEmpty()) {
      Log::info(_T("set full update request to UpdateHandler"));
      m_updateHandler->setFullUpdateRequested(&m_fullReqRegion);
    }

    Log::info(_T("extracting updates from UpdateHandler"));
    m_updateHandler->extract(&updCont);
  } catch (Exception &e) {
    Log::info(_T("WinDesktop::sendUpdate() failed with error:%s"),
               e.getMessage());
    m_extDeskTermListener->onAbnormalDesktopTerminate();
  }

  if (!updCont.isEmpty() || !m_fullReqRegion.isEmpty()) {
    Log::info(_T("UpdateContainer is not empty.")
              _T(" Updates will be given to all."));
    m_extUpdSendingListener->onSendUpdate(&updCont,
                                          m_updateHandler->getFrameBuffer(),
                                          m_updateHandler->getCursorShape());
    Log::info(_T("Updates have been given to all."));
    AutoLock al(&m_reqRegMutex);
    m_fullReqRegion.clear();
  } else {
    Log::info(_T("UpdateContainer is empty"));
  }
}

void GuiDesktop::onUpdate()
{
  Log::info(_T("update detected"));
  m_newUpdateEvent.notify();
}

void GuiDesktop::onUpdateRequest(const Rect *rectRequested, bool incremental)
{
  Log::info(_T("update requested"));

  AutoLock al(&m_reqRegMutex);
  if (!incremental) {
    m_fullReqRegion.addRect(rectRequested);
  }
  m_newUpdateEvent.notify();
}

void GuiDesktop::onClipboardUpdate(const StringStorage *newClipboard)
{
  _ASSERT(m_extClipListener != 0);

  Log::info(_T("clipboard update detected"));

  // Send new clipboard text, even if it is empty.
  m_extClipListener->onClipboardUpdate(newClipboard);
}

void GuiDesktop::onConfigReload(ServerConfig *serverConfig)
{
  applyNewConfiguration();
}
