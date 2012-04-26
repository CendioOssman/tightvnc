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

#include "client-config-lib/ViewerConfig.h"
#include "log-server/Log.h"
#include "TvnViewer.h"
#include "ConnectionData.h"
#include "ViewerCmdLine.h"
#include "util/ResourceLoader.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPTSTR lpCmdLine, int nCmdShow)
{
  // loading config
  ConnectionConfig conConf;
  ConnectionConfigSM ccsm(RegistryPaths::VIEWER_PATH,
                          _T(".listen"));
  conConf.loadFromStorage(&ccsm);

  ViewerSettingsManager::initInstance(RegistryPaths::VIEWER_PATH);
  SettingsManager *sm = ViewerSettingsManager::getInstance();

  ViewerConfig config(RegistryPaths::VIEWER_PATH);
  config.initLog(LogNames::LOG_DIR_NAME, LogNames::VIEWER_LOG_FILE_STUB_NAME);
  config.loadFromStorage(sm);

  // resource-loader initialization
  ResourceLoader resourceLoader(hInstance);

  Log::debug(_T("main()"));
  Log::debug(_T("loading settings from storage completed"));
  Log::debug(_T("Log initialization completed"));

  ConnectionData condata;
  ViewerCmdLine cmd(&condata, &conConf, &config);

  int result = 0;
  try {
    cmd.parse();
    TvnViewer tvnViewer(&condata, &conConf);
    result = tvnViewer.run(true);
  } catch (CommandLineFormatException &excep) {
    StringStorage strError(excep.getMessage());
    MessageBox(0, strError.getString(), _T("Error"), MB_OK | MB_ICONERROR);
    return 0;
  } catch (CommandLineFormatHelp &) {
    cmd.onHelp();
    return 0;
  } catch (Exception &excep) {
    Log::debug(excep.getMessage());
  }

  return result;
}
