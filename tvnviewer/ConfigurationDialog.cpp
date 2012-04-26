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

#include "ConfigurationDialog.h"
#include "NamingDefs.h"

#include "win-system/Process.h"

ConfigurationDialog::ConfigurationDialog()
: BaseDialog(IDD_CONFIGURATION)
{
}

BOOL ConfigurationDialog::onCommand(UINT controlID, UINT notificationID)
{
  if (controlID == IDC_EVERBLVL) {
    if (notificationID == EN_CHANGE) {
      StringStorage text;
      int logLevel;
      m_verbLvl.getText(&text);
      StringParser::parseInt(text.getString(), &logLevel);
      if (logLevel != 0) {
        m_logging.setEnabled(true);
        m_openLogDir.setEnabled(true);
      } else {
        m_logging.setEnabled(false);
        m_openLogDir.setEnabled(false);
      }
    }
  }
  if (controlID == IDOK) {
    onOkPressed();
    kill(1);
    return TRUE;
  }
  if (controlID == IDCANCEL) {
    kill(0);
    return TRUE;
  }
  if (controlID == IDC_BCLEAR_LIST) {
    ViewerConfig::getInstance()->getConnectionHistory()->clear();
    m_historyWasCleared = true;
  }
  if (controlID == IDC_OPEN_LOG_FOLDER_BUTTON) {
    onOpenFolderButtonClick();
  }
  return FALSE;
}

void ConfigurationDialog::onOpenFolderButtonClick()
{
  StringStorage logDir;
  
  ViewerConfig::getInstance()->getLogDir(&logDir);

  StringStorage command;
  command.format(_T("explorer /select,%s\\%s.log"),
                 logDir.getString(),
                 LogNames::VIEWER_LOG_FILE_STUB_NAME);

  Process explorer(command.getString());

  try {
    explorer.start();
  } catch (...) {
    // TODO: Place error notification here.
  }
}

BOOL ConfigurationDialog::onInitDialog()
{
  m_historyWasCleared = false;

  setControlById(m_showToolBars, IDC_CSHOWTOOLBARS); 
  setControlById(m_warnAtSwitching, IDC_CWARNATSW);
  setControlById(m_numberConn, IDC_ENUMCON);
  setControlById(m_snumConn, IDC_SNUMCON);
  setControlById(m_reverseConn, IDC_EREVCON);
  setControlById(m_sreverseConn, IDC_SREVCON);
  setControlById(m_verbLvl, IDC_EVERBLVL);
  setControlById(m_sverbLvl, IDC_SVERBLVL);
  setControlById(m_logging, IDC_ELOGGING);
  setControlById(m_openLogDir, IDC_OPEN_LOG_FOLDER_BUTTON);

  m_snumConn.setRange(0, 1024);
  m_snumConn.setBuddy(&m_numberConn);

  m_sreverseConn.setRange32(1, 65535);
  m_sreverseConn.setBuddy(&m_reverseConn);

  m_sverbLvl.setRange(0, 9);
  m_sverbLvl.setBuddy(&m_verbLvl);

  updateControlValues();

  return FALSE;
}

void ConfigurationDialog::updateControlValues()
{
  ViewerConfig *config = ViewerConfig::getInstance();

  StringStorage txt;

  txt.format(_T("%d"), config->getListenPort());
  m_reverseConn.setText(txt.getString());

  txt.format(_T("%d"), config->getLogLevel());
  m_verbLvl.setText(txt.getString());

  txt.format(_T("%d"), config->getHistoryLimit());
  m_numberConn.setText(txt.getString());

  m_showToolBars.check(config->isToolbarShown());
  m_warnAtSwitching.check(config->isPromptOnFullscreenEnabled());

  StringStorage logFileName;
  logFileName.format(_T("%s\\%s.log"), config->getPathToLogFile(), LogNames::VIEWER_LOG_FILE_STUB_NAME);
  m_logging.setText(logFileName.getString());
}

bool ConfigurationDialog::isInputValid()
{
  if (!testNum(&m_reverseConn, _T("Listen port"))) {
    return false;
  }
  if (!testNum(&m_verbLvl, _T("Log level"))) {
    return false;
  }
  if (!testNum(&m_numberConn, _T("History limit"))) {
    return false;
  }
  return true;
}

bool ConfigurationDialog::testNum(TextBox *tb, const TCHAR *tbName)
{
  StringStorage text;
  tb->getText(&text);

  if (StringParser::tryParseInt(text.getString())) {
    return true;
  }

  StringStorage message;
  message.format(_T("Value in field '%s' must be numeric"), tbName);

  MessageBox(m_ctrlThis.getWindow(), message.getString(),
             _T("TightVNC Viewer Configuration"), MB_OK | MB_ICONWARNING);

  tb->setFocus();

  return false;
}

void ConfigurationDialog::onOkPressed()
{
  if (!isInputValid()) {
    return ;
  }

  ViewerConfig *config = ViewerConfig::getInstance();

  StringStorage text;
  int intVal;

  m_reverseConn.getText(&text);
  StringParser::parseInt(text.getString(), &intVal);
  config->setListenPort(intVal);

  m_verbLvl.getText(&text);
  StringParser::parseInt(text.getString(), &intVal);
  config->setLogLevel(intVal);

  int oldLimit = config->getHistoryLimit();
  m_numberConn.getText(&text);
  StringParser::parseInt(text.getString(), &intVal);
  config->setHistoryLimit(intVal);

  if (config->getHistoryLimit() < oldLimit) {
    config->getConnectionHistory()->truncate();
  }

  config->showToolbar(m_showToolBars.isChecked());
  config->promptOnFullscreen(m_warnAtSwitching.isChecked());

  SettingsManager *sm = ViewerSettingsManager::getInstance();
  config->saveToStorage(sm);
}
