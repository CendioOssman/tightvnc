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

#include "AboutDialog.h"
#include "ConfigurationDialog.h"
#include "LoginDialog.h"
#include "NamingDefs.h"
#include "OptionsDialog.h"

#include "win-system/Shell.h"

LoginDialog::LoginDialog()
: BaseDialog(IDD_LOGINDIALOG),
  m_isListening(false)
{
}

LoginDialog::~LoginDialog()
{
}

BOOL LoginDialog::onInitDialog() 
{
  setControlById(m_server, IDC_CSERVER);
  setControlById(m_listening, IDC_LISTENING);
  setControlById(m_ok, IDOK);
  updateHistory();
  SetForegroundWindow(getControl()->getWindow());
  m_server.setFocus();
  if (m_isListening) {
    onListening();
  }
  return TRUE;
}

void LoginDialog::setConConf(ConnectionConfig *conConf) 
{
  m_conConf = conConf;
}

void LoginDialog::enableConnect()
{
  StringStorage str;
  int iSelected = m_server.getSelectedItemIndex();
  if (iSelected == -1) {
    m_server.getText(&str);
    m_ok.setEnabled(!str.isEmpty());
  } else {
    m_ok.setEnabled(true);
  }
}

void LoginDialog::updateHistory() 
{
  ConnectionHistory *conHistory;

  StringStorage currentServer;
  m_server.getText(&currentServer);
  m_server.removeAllItems();
  conHistory = ViewerConfig::getInstance()->getConnectionHistory();
  conHistory->load();
  for (size_t i = 0; i < conHistory->getHostCount(); i++) {
    m_server.insertItem(static_cast<int>(i), conHistory->getHost(i));
  }
  m_server.setText(currentServer.getString());
  if (m_server.getItemsCount()) {
    if (currentServer.isEmpty()) {
      m_server.setSelectedItem(0);
    }
    StringStorage server;
    m_server.getText(&server);
    ConnectionConfigSM ccsm(RegistryPaths::VIEWER_PATH,
                            server.getString());
    m_conConf->loadFromStorage(&ccsm);
  }
}

void LoginDialog::onConnect() 
{
  ConnectionHistory *conHistory = ViewerConfig::getInstance()->getConnectionHistory();

  m_server.getText(&m_serverHost);

  conHistory->load();
  conHistory->addHost(m_serverHost.getString());
  conHistory->save();
}

void LoginDialog::onConfiguration()
{
  ConfigurationDialog dialog;

  dialog.setParent(getControl());
  dialog.showModal();
  updateHistory();
}

BOOL LoginDialog::onOptions()
{
  OptionsDialog dialog;
  dialog.setConnectionConfig(m_conConf);
  dialog.setParent(getControl());
  if (dialog.showModal() == 1) {
    StringStorage server;
    m_server.getText(&server);
    if (server.isEmpty()) {
      server.setString(_T(".listen"));
    }
    ConnectionConfigSM ccsm(RegistryPaths::VIEWER_PATH,
                            server.getString());
    m_conConf->saveToStorage(&ccsm);
    return FALSE;
  }
  return TRUE;
}

void LoginDialog::onOrder()
{
  openUrl(StringTable::getString(IDS_URL_LICENSING_FVC));
}

void LoginDialog::openUrl(const TCHAR *url)
{
  // TODO: removed duplicated code (see AboutDialog.h)
  try {
    Shell::open(url, 0, 0);
  } catch (SystemException &sysEx) {
    StringStorage message;

    message.format(StringTable::getString(IDS_FAILED_TO_OPEN_URL_FORMAT), sysEx.getMessage());

    MessageBox(m_ctrlThis.getWindow(),
               message.getString(),
               StringTable::getString(IDS_MBC_TVNVIEWER),
               MB_OK | MB_ICONEXCLAMATION);
  }
}

void LoginDialog::setListening(bool isListening)
{
  m_listening.setEnabled(isListening);
  m_isListening = isListening;
}

void LoginDialog::onListening()
{
  ConnectionConfigSM ccsm(RegistryPaths::VIEWER_PATH,
                          _T(".listen"));
  m_conConf->loadFromStorage(&ccsm);

  m_listening.setEnabled(false);
}

void LoginDialog::onAbout()
{
  AboutDialog dlg;

  dlg.setParent(&m_ctrlThis);
  dlg.showModal();
}

BOOL LoginDialog::onCommand(UINT controlID, UINT notificationID)
{
  switch (controlID) {
  case IDC_CSERVER:
    if (notificationID == CBN_SELENDOK) {
      StringStorage server;
      m_server.getItemText(m_server.getSelectedItemIndex(), &server);
      ConnectionConfigSM ccsm(RegistryPaths::VIEWER_PATH,
                              server.getString());
      m_conConf->loadFromStorage(&ccsm);
    }
    enableConnect();
    break;

  case IDOK:
    onConnect();
    kill(CONNECTION_MODE);
    break;
  case IDCANCEL:
    kill(CANCEL_MODE);
    break;

  case IDC_BCONFIGURATION:
    onConfiguration();
    break;

  case IDC_BOPTIONS:
    return onOptions();

  case IDC_LISTENING:
    onListening();
    kill(LISTENING_MODE);
    break;

  case IDC_ORDER_SUPPORT_BUTTON:
    onOrder();
    break;

  case IDC_BABOUT:
    onAbout();
    break;

  default:
    return FALSE;
  }
  return TRUE;
}

StringStorage LoginDialog::getServerHost() 
{
  return m_serverHost;
}
