// Copyright (C) 2008,2009,2010,2011,2012 GlavSoft LLC.
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

#include "WinDesktop.h"
#include "server-config-lib/Configurator.h"
#include "desktop-ipc/UpdateHandlerClient.h"
#include "LocalUpdateHandler.h"
#include "WindowsInputBlocker.h"
#include "desktop-ipc/UserInputClient.h"
#include "SasUserInput.h"
#include "WindowsUserInput.h"
#include "DesktopConfigLocal.h"
#include "win-system/Environment.h"
#include "win-system/WindowsDisplays.h"

WinDesktop::WinDesktop(ClipboardListener *extClipListener,
                       UpdateSendingListener *extUpdSendingListener,
                       AbnormDeskTermListener *extDeskTermListener,
                       LogWriter *log)
: GuiDesktop(extClipListener, extUpdSendingListener, extDeskTermListener, log),
  m_wallPaper(0),
  m_deskConf(0),
  m_log(log)
{
  m_log->info(_T("Creating WinDesktop"));

  logDesktopInfo();

  try {
    m_updateHandler = new LocalUpdateHandler(this, m_log);
    bool ctrlAltDelEnabled = false;
    m_userInput = new WindowsUserInput(this, ctrlAltDelEnabled, m_log);
    m_deskConf = new DesktopConfigLocal(m_log);
    applyNewConfiguration();
    m_wallPaper = new WallpaperUtil(m_log);
    m_wallPaper->updateWallpaper();

    Configurator::getInstance()->addListener(this);
  } catch (Exception &ex) {
    m_log->error(_T("exception during WinDesktop creaion: %s"), ex.getMessage());
    freeResource();
    throw;
  }
  resume();
}

WinDesktop::~WinDesktop()
{
  m_log->info(_T("Deleting WinDesktop"));
  terminate();
  wait();
  freeResource();
  m_log->info(_T("WinDesktop deleted"));
}

void WinDesktop::freeResource()
{
  Configurator::getInstance()->removeListener(this);

  if (m_wallPaper) delete m_wallPaper;

  if (m_updateHandler) delete m_updateHandler;
  if (m_deskConf) delete m_deskConf;
  if (m_userInput) delete m_userInput;
}

void WinDesktop::onTerminate()
{
  m_newUpdateEvent.notify();
}

void WinDesktop::execute()
{
  m_log->info(_T("WinDesktop thread started"));

  while (!isTerminating()) {
    m_newUpdateEvent.waitForEvent();
    if (!isTerminating()) {
      sendUpdate();
    }
  }

  m_log->info(_T("WinDesktop thread stopped"));
}

bool WinDesktop::isRemoteInputTempBlocked()
{
  return !m_deskConf->isRemoteInputAllowed();
}

void WinDesktop::applyNewConfiguration()
{
  m_log->info(_T("reload WinDesktop configuration"));
  m_deskConf->updateByNewSettings();
}

void WinDesktop::logDesktopInfo()
{
  try {
    if (Environment::isAeroOn(m_log)) {
      m_log->debug(_T("The Aero is On"));
    } else {
      m_log->debug(_T("The Aero is Off"));
    }
  } catch (Exception &e) {
    m_log->error(_T("Can't get information for the Aero: %s"), e.getMessage());
  }

  // Log all display coordinates
  WindowsDisplays m_winDisp;
  std::vector<Rect> displays = m_winDisp.getDisplays();
  m_log->debug(_T("The console desktop has %d displays"), (int)displays.size());
  for (size_t i = 0; i < displays.size(); i++) {
    m_log->debug(_T("Display %d placed at the %d, %d, %dx%d coordinates"),
               i + 1,
               displays[i].left, displays[i].top,
               displays[i].getWidth(), displays[i].getHeight());
  }
}
