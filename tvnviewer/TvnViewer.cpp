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

#include "TvnViewer.h"
#include "LoginDialog.h"
#include "network/socket/WindowsSocket.h"
#include "AboutDialog.h"
#include "ConfigurationDialog.h"
#include "NamingDefs.h"
#include "OptionsDialog.h"
#include "resource.h"

TvnViewer::TvnViewer(ConnectionData *condata, 
                     ConnectionConfig *conConf)
: m_conListener(0),
  m_condata(condata),
  m_conConf(conConf),
  m_isListening(false)
{
  Log::info(_T("Init WinSock 2.1"));
  WindowsSocket::startup(2, 1);
  StringStorage str(WindowNames::TVN_WINDOW_CLASS_NAME);
  m_regClass.regClass(&str);
}

TvnViewer::~TvnViewer()
{
  Log::info(_T("Thread collector: destroy all thread"));
  m_threadCollector.destroyAllThreads();

  for (size_t i = 0; i < m_conDatas.size(); i++) {
    delete m_conDatas[i];
  }

  Log::info(_T("Shutdown WinSock"));
  WindowsSocket::cleanup();
}

int TvnViewer::loginDialog()
{
  LoginDialog dialog;

  dialog.setListening(m_condata->isListening());
  dialog.loadIcon(IDI_APPICON);  
  dialog.setConConf(m_conConf);
  int res = dialog.showModal();

  m_fullHost = dialog.getServerHost();

  if (res == LoginDialog::LISTENING_MODE) {
    m_condata->setListening();
  }
  return res;
}

void TvnViewer::listeningMode()
{
  m_isListening = true;
  ViewerConfig *config = ViewerConfig::getInstance();

  try {
    ConnectionListener conListener(m_condata, m_conConf, config->getListenPort());
    showTrayIcon();
    runTrayIcon();
  } catch (Exception &exception) {
    MessageBox(0, exception.getMessage(), _T("Application Error"), MB_OK | MB_ICONERROR);
  }
}

void TvnViewer::runInstance(bool isBlocked)
{
  ConnectionData *conData = new ConnectionData;

  conData->setHost(&m_fullHost);
  ViewerInstance *viewerInst = new ViewerInstance(conData,
                                                  m_conConf);
  viewerInst->start(!isBlocked);
  m_threadCollector.addThread(viewerInst);
  if (!isBlocked) {
    m_conDatas.push_back(conData);
  } else {
    delete conData;
  }
}

int TvnViewer::run(bool isBlocked)
{
  bool modeIsSelected = false;

  if (!m_isListening) {
    if (m_condata->isListening()) {
      listeningMode();
      modeIsSelected = true;
    }
  }

  if (!modeIsSelected && m_condata->isUsed()) {
    ViewerInstance viewerInst(m_condata, m_conConf);
    viewerInst.start(false);
    modeIsSelected = true;
  }

  if (!modeIsSelected) {
    int resLogin = loginDialog();

    switch (resLogin) {
    case LoginDialog::CONNECTION_MODE:
      runInstance(isBlocked);
      break;
    case LoginDialog::LISTENING_MODE:
      if (!m_isListening) {
        listeningMode();
      }
      break;
    case LoginDialog::CANCEL_MODE:
      break;
    default:
      _ASSERT(true);
    }
  }
  return 0;
}

void TvnViewer::onNewConnection()
{
  run(false);
}

void TvnViewer::onDefaultConnection()
{
  ConnectionConfigSM ccsm(RegistryPaths::VIEWER_PATH,
                          _T(".listen"));
  OptionsDialog dialog;

  dialog.setConnectionConfig(m_conConf);
  if (dialog.showModal() == 1) {
    m_conConf->saveToStorage(&ccsm);
  }
}

void TvnViewer::onConfiguration()
{
  ConfigurationDialog dialog;

  dialog.showModal();
}

void TvnViewer::onAboutViewer()
{
  AboutDialog dlg;

  dlg.showModal();
}

void TvnViewer::onCloseViewer()
{
  PostQuitMessage(0);
}

void TvnViewer::onShowMainWindow()
{
  run(false);
}
