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

#ifndef _VIEWER_WINDOW_H_
#define _VIEWER_WINDOW_H_

#include "ConnectionData.h"
#include "DesktopWindow.h"
#include "FileTransferMainDialog.h"
#include "NamingDefs.h"
#include "OptionsDialog.h"
#include "ScaleManager.h"
#include "ViewerMenu.h"
#include "gui/ToolBar.h"
#include "viewer-core/FileTransferCapability.h"
#include "viewer-core/RemoteViewerCore.h"
#include "viewer-core/CoreEventsAdapter.h"
#include "viewer-core/VncAuthentication.h"
#include "win-system/SystemInformation.h"

class ViewerWindow : public BaseWindow,
                     public CoreEventsAdapter
{
public:
  ViewerWindow(ConnectionData *conData, ConnectionConfig *conConf);
  virtual ~ViewerWindow();

  void setFileTransfer(FileTransferCapability *ft);
  void setRemoteViewerCore(RemoteViewerCore *pCore);

  static const int WM_USER_ERROR = WM_USER + 1;
  static const int WM_USER_STOP = WM_USER + 2;

protected:
  bool onMessage(UINT message, WPARAM wParam, LPARAM lParam);
  bool onEraseBackground(HDC hdc);
  bool onError(WPARAM wParam);
  bool onSize(WPARAM wParam, LPARAM lParam);
  bool onCreate(LPCREATESTRUCT lps);
  bool onCommand(WPARAM wParam, LPARAM lParam);
  bool onNotify(int idCtrl, LPNMHDR pnmh);
  bool onSysCommand(WPARAM wParam, LPARAM lParam);
  bool onDestroy();
  bool onFocus(WPARAM wParam);
  bool onKillFocus(WPARAM wParam);

  void commandCtrlAltDel();
  void commandCtrlEsc();
  void commandCtrl();
  void commandAlt();
  void commandToolBar();
  void commandPause();
  void onAbout();
  void commandSaveConnection();
  void commandScaleIn();
  void commandScaleOut();
  void commandScale100();
  void commandScaleAuto();

  // CoreEventsAdapter
  void onBell();
  void onConnected();
  void onDisconnect(const StringStorage *message);
  void onAuthError(const AuthException *exception);
  void onError(const Exception *exception);
  void onFrameBufferUpdate(const FrameBuffer *fb, const Rect *rect);
  void onFrameBufferPropChange(const FrameBuffer *fb);
  void doAuthenticate(const int securityType,
                      RfbInputGate *input,
                      RfbOutputGate *output);
  void onCutText(const StringStorage *cutText);
  void getPassword(StringStorage *strPassw);
  int translateAccelToTB(int val);
  void applyScreenChanges(bool isFullScreen);
  void calculateDefaultSize(RECT * prc, int *width, int *height);

  ConnectionConfigSM m_ccsm;
  ConnectionConfig *m_conConf;
  RemoteViewerCore *m_pViewerCore;
  FileTransferCapability *m_fileTransfer;
  FileTransferMainDialog *m_ftDialog;
  DesktopWindow m_dsktWnd;
  Exception m_error;
  StringStorage m_strToolTip;
  ToolBar m_toolbar;
  ViewerMenu m_menu;
  ConnectionData *m_conData;
  SystemInformation m_sysinf;

  // for full screen mode
  bool m_isFullScr;
  RECT m_rcNormal;
  bool m_bToolBar;
  int m_scale;

private:
  vector<int> m_standardScale;
  void changeCursor(int type);
  void applySettings();
  void doFullScr();
  void doUnFullScr();
  void doSize();
  void doCommand(int iCommand);
  void showFileTransferDialog();
  void showWindow();
  void enableUserElements();
  bool viewerCoreSettings();
  void dialogConnectionOptions();
  void dialogConnectionInfo();
  void switchFullScreenMode();
  void dialogConfiguration();

};

#endif
