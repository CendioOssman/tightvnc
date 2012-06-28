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

#include "config-lib/IniFileSettingsManager.h"
#include "util/Exception.h"
#include "util/ResourceLoader.h"
#include "viewer-core/StandardPixelFormatFactory.h"

#include "AboutDialog.h"
#include "AuthenticationDialog.h"
#include "ConfigurationDialog.h"
#include "FsWarningDialog.h"
#include "NamingDefs.h"
#include "TvnViewer.h"
#include "ViewerWindow.h"

ViewerWindow::ViewerWindow(WindowsApplication *application,
                           ConnectionData *conData,
                           ConnectionConfig *conConf,
                           Logger *logger)
: m_ccsm(RegistryPaths::VIEWER_PATH,
         conData->getHost().getString()),
  m_application(application),
  m_logWriter(logger),
  m_conConf(conConf),
  m_scale(100),
  m_isFullScr(false),
  m_ftDialog(0),
  m_pViewerCore(0),
  m_fileTransfer(0),
  m_conData(conData),
  m_dsktWnd(&m_logWriter, conConf),
  m_isConnected(false),
  m_sizeIsChanged(false),
  m_stopped(false)
{
  m_standardScale.push_back(10);
  m_standardScale.push_back(15);
  m_standardScale.push_back(25);
  m_standardScale.push_back(50);
  m_standardScale.push_back(75);
  m_standardScale.push_back(90);
  m_standardScale.push_back(100);
  m_standardScale.push_back(150);
  m_standardScale.push_back(200);
  m_standardScale.push_back(400);

  StringStorage windowClass = WindowNames::TVN_WINDOW_CLASS_NAME;
  StringStorage titleName = WindowNames::TVN_WINDOW_TITLE_NAME;
  StringStorage subTitleName = WindowNames::TVN_SUB_WINDOW_TITLE_NAME;

  setClass(&windowClass);
  createWindow(&titleName, WS_OVERLAPPEDWINDOW);

  m_dsktWnd.setClass(&windowClass);
  m_dsktWnd.createWindow(&subTitleName, WS_VISIBLE | WS_CLIPSIBLINGS | WS_CHILD, getHWnd());
}

ViewerWindow::~ViewerWindow()
{
  if (m_ftDialog != 0) {
    delete m_ftDialog;
  }
}

void ViewerWindow::setFileTransfer(FileTransferCapability *ft)
{
  m_fileTransfer = ft;
}

void ViewerWindow::setRemoteViewerCore(RemoteViewerCore *pCore)
{
  m_pViewerCore = pCore;
  m_dsktWnd.setViewerCore(pCore);
  applySettings();
}

bool ViewerWindow::onCreate(LPCREATESTRUCT lps)
{
  m_control.setWindow(m_hWnd);

  setClassCursor(LoadCursor(NULL, IDC_ARROW));
  loadIcon(IDI_APPICON);
  m_toolbar.loadToolBarfromRes(IDB_TOOLBAR);
  m_toolbar.setButtonsRange(IDS_TB_NEWCONNECTION);
  m_toolbar.setViewAutoButtons(4, ToolBar::TB_Style_sep);
  m_toolbar.setViewAutoButtons(6, ToolBar::TB_Style_sep);
  m_toolbar.setViewAutoButtons(10, ToolBar::TB_Style_sep);
  m_toolbar.setViewAutoButtons(11, ToolBar::TB_Style_sep);
  m_toolbar.setViewAutoButtons(15, ToolBar::TB_Style_sep);
  m_toolbar.attachToolBar(getHWnd());
  m_menu.getSystemMenu(getHWnd());
  m_menu.loadMenu();
  applySettings();

  ViewerConfig *config = ViewerConfig::getInstance();
  bool bShowToolbar = config->isToolbarShown();
  if (!bShowToolbar) {
    m_toolbar.hide();
    m_bToolBar = false;
  }
  m_menu.checkedMenuItem(IDS_TB_TOOLBAR, bShowToolbar);
  return true;
}

void ViewerWindow::enableUserElements()
{
  bool isEnable = !m_conConf->isViewOnly();

  m_toolbar.enableButton(IDS_TB_ALT, isEnable);
  m_toolbar.enableButton(IDS_TB_CTRL, isEnable);
  m_toolbar.enableButton(IDS_TB_CTRLESC, isEnable);
  m_toolbar.enableButton(IDS_TB_CTRLALTDEL, isEnable);

  UINT enableMenu = static_cast<UINT>(!isEnable);
  m_menu.enableMenuItem(IDS_TB_CTRLALTDEL, enableMenu);
  m_menu.enableMenuItem(IDS_TB_CTRLESC, enableMenu);
  m_menu.enableMenuItem(IDS_TB_CTRL, enableMenu);
  m_menu.enableMenuItem(IDS_TB_ALT, enableMenu);

  if (!isEnable) {
    m_menu.checkedMenuItem(IDS_TB_ALT, false);
    m_menu.checkedMenuItem(IDS_TB_CTRL, false);
  }

  int scale = m_conConf->getScaleNumerator() * 100 / m_conConf->getScaleDenominator();
  m_toolbar.enableButton(IDS_TB_SCALEOUT, scale > m_standardScale[0]);
  m_toolbar.enableButton(IDS_TB_SCALEIN, scale < m_standardScale[m_standardScale.size() - 1]);
  if (m_conConf->isFitWindowEnabled()) {
    m_toolbar.checkButton(IDS_TB_SCALEAUTO, true);
    m_toolbar.enableButton(IDS_TB_SCALE100, true);
  } else {
    m_toolbar.enableButton(IDS_TB_SCALE100, scale != 100);
  }
}

bool ViewerWindow::viewerCoreSettings()
{
  if (!m_pViewerCore) {
    return false;
  }

  bool bFileTransfer = m_fileTransfer && m_fileTransfer->isEnabled();

  m_toolbar.enableButton(IDS_TB_TRANSFER, bFileTransfer && !m_conConf->isViewOnly());
  UINT enableMenu = static_cast<UINT>(!(bFileTransfer && !m_conConf->isViewOnly()));
  m_menu.enableMenuItem(IDS_TB_TRANSFER, enableMenu);

  m_pViewerCore->allowCopyRect(m_conConf->isCopyRectAllowed());
  m_pViewerCore->setPreferredEncoding(m_conConf->getPreferredEncoding());

  m_pViewerCore->ignoreCursorShapeUpdates(m_conConf->isIgnoringShapeUpdates());
  m_pViewerCore->enableCursorShapeUpdates(m_conConf->isRequestingShapeUpdates());

  // set -1, if compression is disabled
  m_pViewerCore->setCompressionLevel(m_conConf->getCustomCompressionLevel());

  // set -1, if jpeg-compression is disabled
  m_pViewerCore->setJpegQualityLevel(m_conConf->getJpegCompressionLevel());

  if (m_conConf->isUsing8BitColor()) {
    m_pViewerCore->setPixelFormat(&StandardPixelFormatFactory::create8bppPixelFormat());
  } else {
    m_pViewerCore->setPixelFormat(&StandardPixelFormatFactory::create32bppPixelFormat());
  }
  return true;
}

void ViewerWindow::applySettings()
{
  int scale;

  if (m_conConf->isFitWindowEnabled()) {
    scale = -1;
  } else {
    int iNum = m_conConf->getScaleNumerator();
    int iDenom = m_conConf->getScaleDenominator();
    scale = (iNum * 100) / iDenom;
  }

  if (scale != m_scale) {
    m_scale = scale;
    m_dsktWnd.setScale(m_scale);
    doSize();
    redraw();
  }
  if (m_isConnected) {
    if (m_conConf->isFullscreenEnabled()) {
      doFullScr();
    } else {
      doUnFullScr();
    }
  }
  changeCursor(m_conConf->getLocalCursorShape());
  enableUserElements();
  viewerCoreSettings();
}

void ViewerWindow::changeCursor(int type)
{
  HCURSOR hcur = 0;

  ResourceLoader *rLoader = ResourceLoader::getInstance();
  switch (type) {
    case ConnectionConfig::DOT_CURSOR:
      hcur = rLoader->loadCursor(IDI_CDOT);
      break;
    case ConnectionConfig::SMALL_CURSOR:
      hcur = rLoader->loadCursor(IDI_CSMALLDOT);
      break;
    case ConnectionConfig::NO_CURSOR:
      hcur = rLoader->loadCursor(IDI_CNOCURSOR);
      break;
    case ConnectionConfig::NORMAL_CURSOR:
      hcur = rLoader->loadStandartCursor(IDC_ARROW);
      break;
  }
  setClassCursor(hcur);
}

bool ViewerWindow::onSysCommand(WPARAM wParam, LPARAM lParam)
{
  return onCommand(wParam, lParam);
}

bool ViewerWindow::onMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
  case WM_SIZING:
    m_sizeIsChanged = true;
    return false;
  case WM_NCDESTROY:
    m_stopped = true;
    return true;
  case WM_USER_STOP:
    SendMessage(m_hWnd, WM_DESTROY, 0, 0);
    return true;
  case WM_USER_FS_WARNING:
    return onFsWarning();
  case WM_CLOSE:
    return onClose();
  case WM_CREATE:
    return onCreate((LPCREATESTRUCT) lParam);
  case WM_SIZE:
    return onSize(wParam, lParam);
  case WM_USER_ERROR:
    return onError(wParam);
  case WM_SETFOCUS:
    return onFocus(wParam);
  case WM_ERASEBKGND:
    return onEraseBackground((HDC)wParam);
  case WM_KILLFOCUS:
    return onKillFocus(wParam);
  }
  return false;
}

bool ViewerWindow::onEraseBackground(HDC hdc) 
{
  return true;
}

bool ViewerWindow::onKillFocus(WPARAM wParam)
{
  if (!m_conConf->isViewOnly()) {
    m_toolbar.checkButton(IDS_TB_ALT, false);
    m_toolbar.checkButton(IDS_TB_CTRL, false);
  }
  return true;
}

void ViewerWindow::dialogConnectionOptions() 
{
  OptionsDialog dialog;

  dialog.setConnected();
  dialog.setConnectionConfig(m_conConf);
  // FIXME: Removed Control from this code and another
  Control control = getHWnd();
  dialog.setParent(&control);
  if (dialog.showModal() == 1) {
    m_conConf->saveToStorage(&m_ccsm);
    applySettings();
  }
}

void ViewerWindow::dialogConnectionInfo() 
{
  StringStorage str, strHost;

  m_conData->getHost(&strHost);
  std::vector<TCHAR> kbdName;
  kbdName.resize(KL_NAMELENGTH);
  memset(&kbdName[0], 0, sizeof(TCHAR) * KL_NAMELENGTH);
  if (!GetKeyboardLayoutName( &kbdName[0] )) {
    kbdName[0] = _T('?'); 
    kbdName[1] = _T('?'); 
    kbdName[2] = _T('?'); 
  }

  int width, height, pixel;
  m_dsktWnd.getServerGeometry(&width, &height, &pixel);
  str.format(_T("Host: %s\nDesktop: %s\nProtocol: %s\nResolution: %ix%i %i bits\nKeyboard layout: %s"),
             strHost.getString(),
             m_pViewerCore->getRemoteDesktopName().getString(),
             m_pViewerCore->getProtocolString().getString(),
             width,
             height,
             pixel,
             &kbdName[0]);
  MessageBox(getHWnd(), 
             str.getString(), 
             _T("Connection Information"), 
             MB_OK | MB_ICONINFORMATION);
}

void ViewerWindow::switchFullScreenMode()
{
  if (m_isFullScr) {
    doUnFullScr();
  } else {
    doFullScr();
  }
}

void ViewerWindow::dialogConfiguration() 
{
  ConfigurationDialog dialog;

  Control control = getHWnd(); 
  dialog.setParent(&control);
  dialog.showModal();
}

void ViewerWindow::commandCtrlAltDel() 
{
  LRESULT iState = m_toolbar.getState(IDS_TB_CTRLALTDEL);
  if (iState) {
    m_dsktWnd.sendCtrlAltDel();
  }
}

void ViewerWindow::commandCtrlEsc()
{
  LRESULT iState = m_toolbar.getState(IDS_TB_CTRLESC);
  if (iState) {
    m_dsktWnd.sendKey(VK_LCONTROL, true);
    m_dsktWnd.sendKey(VK_ESCAPE,   true);
    m_dsktWnd.sendKey(VK_ESCAPE,   false);
    m_dsktWnd.sendKey(VK_LCONTROL, false);
  }
}

void ViewerWindow::commandCtrl()
{
  LRESULT iState = m_toolbar.getState(IDS_TB_CTRL);
  if (iState) {
    if (iState == TBSTATE_ENABLED) {
      m_menu.checkedMenuItem(IDS_TB_CTRL, true);
      m_toolbar.checkButton(IDS_TB_CTRL,  true);
      m_dsktWnd.sendKey(VK_LCONTROL,      true);
    } else {
      m_menu.checkedMenuItem(IDS_TB_CTRL, false);
      m_toolbar.checkButton(IDS_TB_CTRL,  false);
      m_dsktWnd.sendKey(VK_LCONTROL,      false);
    }
  }
}

void ViewerWindow::commandAlt()
{
  LRESULT iState = m_toolbar.getState(IDS_TB_ALT);
  if (iState) {
    if (iState == TBSTATE_ENABLED) {
      m_menu.checkedMenuItem(IDS_TB_ALT, true);
      m_toolbar.checkButton(IDS_TB_ALT,  true);
      m_dsktWnd.sendKey(VK_LMENU,        true);
    } else {
      m_menu.checkedMenuItem(IDS_TB_ALT, false);
      m_toolbar.checkButton(IDS_TB_ALT,  false);
      m_dsktWnd.sendKey(VK_LMENU,        false);
    }
  }
}

void ViewerWindow::commandPause()
{
  LRESULT iState = m_toolbar.getState(IDS_TB_PAUSE);
  if (iState) {
    if (iState == TBSTATE_ENABLED) {
      m_toolbar.checkButton(IDS_TB_PAUSE, true);
      m_pViewerCore->stopUpdating(true);
    } else {
      m_toolbar.checkButton(IDS_TB_PAUSE, false);
      m_pViewerCore->stopUpdating(false);
    }
  }
}

void ViewerWindow::commandToolBar()
{
  if (m_toolbar.isVisible()) {
    m_menu.checkedMenuItem(IDS_TB_TOOLBAR,   false);
    m_toolbar.hide();
    adjustWindowSize();
    doSize();
  } else {
    if (!m_isFullScr) {
      m_menu.checkedMenuItem(IDS_TB_TOOLBAR, true);
      m_toolbar.show();
      adjustWindowSize();
      doSize();
    }
  }
}

void ViewerWindow::commandNewConnection()
{
  m_application->postMessage(TvnViewer::WM_USER_NEW_CONNECTION);
}

void ViewerWindow::commandSaveConnection()
{
  TCHAR fileName[MAX_PATH] = _T("");

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = m_hWnd;
  ofn.lpstrFilter = _T("VNC-configuration files (*.vnc)\0*.vnc;")
                    _T("All files (*.*)\0*.*\0");
  ofn.lpstrDefExt = _T("vnc");
  ofn.lpstrFile= fileName;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = OFN_OVERWRITEPROMPT;
  try {
    if (GetSaveFileName(&ofn)) {
      TCHAR passMsg[] = _T("Do you want to save the password in this file?\r\n")
                        _T("If you save \"Yes\", anyone with access to this file could access ")
                        _T("your session and (potentially) discover your connection-password.");
      DeleteFile(fileName);
      IniFileSettingsManager sm(fileName);
      sm.setApplicationName(_T("connection"));

      StringStorage host;
      m_conData->getReducedHost(&host);
      sm.setString(_T("host"), host.getString());
      sm.setUINT(_T("port"), m_conData->getPort());
      StringStorage plainPassword = *(m_conData->getDefaultPassword());
      int whetherToSavePass = IDNO;
      if (!plainPassword.isEmpty()) {
        StringStorage password;
        m_conData->getCryptedPassword(&password);
        whetherToSavePass = MessageBox(m_hWnd, passMsg, _T("Security Warning"), MB_YESNO);
      }

      if (whetherToSavePass == IDYES) {
        StringStorage password;
        m_conData->getCryptedPassword(&password);
        sm.setString(_T("password"), password.getString());
      }

      sm.setApplicationName(_T("options"));
      m_conConf->saveToStorage(&sm);
    }
  } catch (...) {
    m_logWriter.error(_T("Unknown error in save connection"));
  }
}

void ViewerWindow::commandScaleIn()
{
  if (m_conConf->isFitWindowEnabled()) {
    commandScaleAuto();
  }

  if (m_standardScale.empty()) {
    _ASSERT(false);
    return;
  }
    
  int scale = m_conConf->getScaleNumerator() * 100 / m_conConf->getScaleDenominator();
  size_t indexNewScale = 0;
  while (indexNewScale < m_standardScale.size() && m_standardScale[indexNewScale] <= scale + 5)
    indexNewScale++;

  if (indexNewScale >= m_standardScale.size())
    indexNewScale = m_standardScale.size() - 1;

  m_conConf->setScale(m_standardScale[indexNewScale], 100);
  m_conConf->fitWindow(false);
  m_conConf->saveToStorage(&m_ccsm);
  applySettings();
}

void ViewerWindow::commandScaleOut()
{
  if (m_conConf->isFitWindowEnabled()) {
    commandScaleAuto();
  }

  if (m_standardScale.empty()) {
    _ASSERT(false);
    return;
  }

  int scale = m_conConf->getScaleNumerator() * 100 / m_conConf->getScaleDenominator();
  size_t indexNewScale = m_standardScale.size();
  do {
    indexNewScale--;
  } while (indexNewScale < m_standardScale.size() && m_standardScale[indexNewScale] >= scale - 5);
  
  if (indexNewScale > m_standardScale.size())
    indexNewScale = 0;

  m_conConf->setScale(m_standardScale[indexNewScale], 100);
  m_conConf->fitWindow(false);
  m_conConf->saveToStorage(&m_ccsm);
  applySettings();
}

void ViewerWindow::commandScale100()
{
  m_toolbar.checkButton(IDS_TB_SCALEAUTO, false);
  m_conConf->setScale(1, 1);
  m_conConf->fitWindow(false);
  m_conConf->saveToStorage(&m_ccsm);
  applySettings();
}

void ViewerWindow::commandScaleAuto()
{
  LRESULT iState = m_toolbar.getState(IDS_TB_SCALEAUTO);
  if (iState) {
    if (iState == TBSTATE_ENABLED) {
      m_toolbar.checkButton(IDS_TB_SCALEAUTO, true);
      m_conConf->fitWindow(true);
    } else {
      m_toolbar.checkButton(IDS_TB_SCALEAUTO, false);

      RECT rcWindow;
      m_dsktWnd.getClientRect(&rcWindow);
      int wndWidth = rcWindow.right - rcWindow.left - 1;
      int wndHeight = rcWindow.bottom - rcWindow.top;

      int scrWidth = 0;
      int scrHeight = 0;
      int pixelSize = 0;
      m_dsktWnd.getServerGeometry(&scrWidth, &scrHeight, &pixelSize);

      if (wndWidth * scrHeight <= wndHeight * scrWidth) {
        m_conConf->setScale(wndWidth, scrWidth);
      } else {
        m_conConf->setScale(wndHeight, scrHeight);
      }
      m_conConf->fitWindow(false);
    }
    m_conConf->saveToStorage(&m_ccsm);
    applySettings();
  }
}

int ViewerWindow::translateAccelToTB(int val) 
{
  static const std::pair<int, int> accelerators[] = {
    make_pair(ID_CONN_OPTIONS,    IDS_TB_CONNOPTIONS),
    make_pair(ID_CONN_INFO,       IDS_TB_CONNINFO), 
    make_pair(ID_SHOW_TOOLBAR,    IDS_TB_TOOLBAR),
    make_pair(ID_FULL_SCR,        IDS_TB_FULLSCREEN), 
    make_pair(ID_REQ_SCR_REFRESH, IDS_TB_REFRESH),
    make_pair(ID_CTRL_ALT_DEL,    IDS_TB_CTRLALTDEL),
    make_pair(ID_TRANSF_FILES,    IDS_TB_TRANSFER)
  };

  for (int i = 0; i < sizeof(accelerators) / sizeof(std::pair<int, int>); i++) {
    if (accelerators[i].first == val) {
      return accelerators[i].second;
    }
  }
  return -1;
}

void ViewerWindow::onAbout()
{
  AboutDialog dlg;

  Control control = getHWnd(); 
  dlg.setParent(&control);
  dlg.showModal();
}

bool ViewerWindow::onCommand(WPARAM wParam, LPARAM lParam)
{
  if (HIWORD(wParam) == 1) {
    int transl = translateAccelToTB(LOWORD(wParam));

    if (transl != -1) {
      wParam = transl;
    }
  }
  switch(wParam) {
    case IDS_ABOUT_VIEWER:
      onAbout();
      return true;
    case IDS_TB_CONNOPTIONS:
      dialogConnectionOptions();
      return true;
    case IDS_TB_CONNINFO:
      dialogConnectionInfo();
      return true;
    case IDS_TB_PAUSE:
      commandPause();
      return true;
    case IDS_TB_REFRESH:
      m_pViewerCore->refreshFrameBuffer();
      return true;
    case IDS_TB_CTRLALTDEL:
      commandCtrlAltDel();
      return true;
    case IDS_TB_CTRLESC:
      commandCtrlEsc();
      return true;
    case IDS_TB_CTRL:
      commandCtrl();
      return true;
    case IDS_TB_ALT:
      commandAlt();
      return true;
    case IDS_TB_TOOLBAR:
      commandToolBar();
      return true;
    case IDS_TB_TRANSFER:
      showFileTransferDialog();
      return true;
    case IDS_TB_NEWCONNECTION:
      commandNewConnection();
      return true;
    case IDS_TB_SAVECONNECTION:
      commandSaveConnection();
      return true;
    case IDS_TB_SCALEIN:
      commandScaleIn();
      return true;
    case IDS_TB_SCALEOUT:
      commandScaleOut();
      return true;
    case IDS_TB_SCALE100:
      commandScale100();
      return true;
    case IDS_TB_SCALEAUTO:
      commandScaleAuto();
      return true;
    case IDS_TB_FULLSCREEN: 
      switchFullScreenMode();
      return true;
    case IDS_TB_CONFIGURATION:
      dialogConfiguration();
      return true;
  }
  return false;
}

void ViewerWindow::showFileTransferDialog()
{
  LRESULT iState = m_toolbar.getState(IDS_TB_TRANSFER);
  if (iState) {
    if (m_ftDialog) {
      if (!m_ftDialog->isCreated()) {
        m_ftDialog->setParent(&m_control);
        m_fileTransfer->setInterface(0);
        delete m_ftDialog;
        m_ftDialog = 0;
      }
    }
    if (!m_ftDialog) {
      m_ftDialog = new FileTransferMainDialog(m_fileTransfer->getCore());
      m_fileTransfer->setInterface(m_ftDialog);
    }
    m_ftDialog->show();
    HWND dialogWnd = m_ftDialog->getControl()->getWindow();
    m_application->addModelessDialog(dialogWnd);
  }
}

void ViewerWindow::applyScreenChanges(bool isFullScreen)
{
  m_isFullScr = isFullScreen;
  doSize();
  redraw();
}

void ViewerWindow::doFullScr()
{
  if (m_isFullScr) {
    return;
  }

  m_conConf->enableFullscreen(true);
  m_conConf->saveToStorage(&m_ccsm);

  ViewerConfig *config = ViewerConfig::getInstance();
  GetWindowRect(getHWnd(), &m_rcNormal);
  m_bToolBar = m_toolbar.isVisible();
  m_toolbar.hide();

  m_menu.checkedMenuItem(IDS_TB_FULLSCREEN, true);
  m_menu.checkedMenuItem(IDS_TB_TOOLBAR,    false);
  m_menu.enableMenuItem(IDS_TB_TOOLBAR,     1);

  RECT rc;
  m_sysinf.getDesktopAllArea(&rc);
  setStyle(WS_VISIBLE | WS_POPUP | WS_SYSMENU);
  setPosition(rc.left, rc.top);
  int width = rc.right - rc.left;
  int height = rc.bottom - rc.top;
  setSize(width, height);

  SetFocus(m_dsktWnd.getHWnd());
  applyScreenChanges(true);

  if (config->isPromptOnFullscreenEnabled()) {
    postMessage(WM_USER_FS_WARNING);
  }
}

void ViewerWindow::doUnFullScr()
{
  if (!m_isFullScr) {
    return;
  }

  m_conConf->enableFullscreen(false);
  m_conConf->saveToStorage(&m_ccsm);

  m_menu.checkedMenuItem(IDS_TB_FULLSCREEN, false);
  m_menu.checkedMenuItem(IDS_TB_TOOLBAR, m_bToolBar);

  if (m_bToolBar) {
    m_toolbar.show();
  } else {
    m_toolbar.hide();
  }

  UINT isEnable = static_cast<UINT>(m_conConf->isViewOnly());
  m_menu.enableMenuItem(IDS_TB_TOOLBAR, isEnable);

  int width  = m_rcNormal.right  - m_rcNormal.left;
  int height = m_rcNormal.bottom - m_rcNormal.top;

  setStyle(WS_VISIBLE | WS_OVERLAPPEDWINDOW);
  setPosition(m_rcNormal.left, m_rcNormal.top);
  setSize(width, height);

  m_dsktWnd.setScale(m_scale);
  applyScreenChanges(false);
}

bool ViewerWindow::onNotify(int idCtrl, LPNMHDR pnmh)
{
  ResourceLoader *rLoader = ResourceLoader::getInstance();
  LPTOOLTIPTEXT toolTipText = reinterpret_cast<LPTOOLTIPTEXT>(pnmh);
  if (toolTipText->hdr.code != TTN_NEEDTEXT) {
    return false;
  }
  int resId = static_cast<int>(toolTipText->hdr.idFrom);
  rLoader->loadString(resId, &m_strToolTip);
  toolTipText->lpszText = const_cast<TCHAR *>(m_strToolTip.getString());
  return true;
}

bool ViewerWindow::onClose()
{
  if (m_ftDialog != 0 && m_ftDialog->isCreated()) {
    if (!m_ftDialog->tryClose()) {
      return true;
    }
  }
  destroyWindow();
  return true;
}

void ViewerWindow::doSize()
{
  postMessage(WM_SIZE);
}

bool ViewerWindow::onSize(WPARAM wParam, LPARAM lParam) 
{
  if (wParam == SIZE_MAXIMIZED) {
    m_isMaximized = true;
  } else {
    m_isMaximized = false;
  }
  RECT rc;
  int x, y;

  getClientRect(&rc);
  x = y = 0;
  if (m_toolbar.isVisible()) {
    m_toolbar.autoSize();
    y = m_toolbar.getHeight() - 1;
    rc.bottom -= y;
  }
  if (m_dsktWnd.getHWnd()) {

    int h = rc.bottom - rc.top;
    int w = rc.right - rc.left;

    if (h > 0 && w > 0) {
      m_dsktWnd.setPosition(x, y);
      m_dsktWnd.setSize(w, h);
    }
  } 
  return true;
}

void ViewerWindow::showWindow()
{
  show();

  StringStorage str = m_pViewerCore->getRemoteDesktopName();
  if (str.getLength() > 0) {
    setWindowText(&str);
    m_dsktWnd.setWindowText(&str);
  }
}

bool ViewerWindow::onError(WPARAM wParam)
{
  if (wParam != AuthException::AUTH_CANCELED) {
    StringStorage error = m_error.getMessage();
    MessageBox(getHWnd(), error.getString(), _T("Error"), MB_OK | MB_ICONERROR);
  }
  destroyWindow();
  return true;
}

bool ViewerWindow::onFsWarning()
{
  FsWarningDialog fsWarning;
  fsWarning.setParent(&m_control);
  fsWarning.showModal();
  return true;
}

bool ViewerWindow::onFocus(WPARAM wParam)
{
  SetFocus(m_dsktWnd.getHWnd());
  return true;
}

void ViewerWindow::onBell()
{
  if (m_conConf->isDeiconifyOnRemoteBellEnabled()) {
    ShowWindow(getHWnd(), SW_RESTORE);
    setForegroundWindow();
  }
  MessageBeep(MB_ICONASTERISK);
}

Rect ViewerWindow::calculateDefaultSize()
{
  RECT desktopRc;
  if (!m_sysinf.getDesktopArea(&desktopRc)) {
     m_sysinf.getDesktopAllArea(&desktopRc);
  }
  Rect defaultRect(&desktopRc);

  int widthDesktop  = defaultRect.getWidth();
  int heightDesktop = defaultRect.getHeight();

  Rect viewerRect = m_dsktWnd.getViewerGeometry();
  int serverWidth = viewerRect.getWidth();
  int serverHeight = viewerRect.getHeight();

  if (serverWidth < widthDesktop && serverHeight < heightDesktop) {
    int borderWidth, borderHeight;
    getBorderSize(&borderWidth, &borderHeight);
    int totalWidth     = serverWidth  + borderWidth;
    int totalHeight    = serverHeight + borderHeight + 1;
    if (m_toolbar.isVisible()) {
      totalHeight += m_toolbar.getHeight();
    }
    defaultRect.setHeight(totalHeight);
    defaultRect.setWidth(totalWidth);
    defaultRect.move((widthDesktop - totalWidth) / 2,
                     (heightDesktop - totalHeight) / 2);
  }
  return defaultRect;
}

void ViewerWindow::onConnected()
{
  m_isConnected = true;

  showWindow();
  setForegroundWindow();
  applySettings();

  m_sizeIsChanged = false;
  adjustWindowSize();
}

void ViewerWindow::onDisconnect(const StringStorage *message)
{
}

void ViewerWindow::onAuthError(const AuthException *exception)
{
  int authCode = exception->getAuthCode(); 
  m_error = *exception;
  postMessage(WM_USER_ERROR, authCode);
}

void ViewerWindow::onError(const Exception *exception)
{
  m_error = *exception;
  postMessage(WM_USER_ERROR);
}

void ViewerWindow::onFrameBufferUpdate(const FrameBuffer *fb, const Rect *rect)
{
  m_dsktWnd.updateFramebuffer(fb, rect);
}

void ViewerWindow::onFrameBufferPropChange(const FrameBuffer *fb)
{
  m_dsktWnd.setNewFramebuffer(fb);
  adjustWindowSize();
}

void ViewerWindow::getPassword(StringStorage *strPassw)
{
  if (!m_conData->getDefaultPassword()->isEmpty()) {
    *strPassw = *m_conData->getDefaultPassword();
  } else {
    AuthenticationDialog passDialog;
    StringStorage strHost;

    m_conData->getHost(&strHost);
    passDialog.setHostName(&strHost);
    if (passDialog.showModal()) {
      *strPassw = *passDialog.getPassw();
      m_conData->setPlainPassword(strPassw);
    } else {
      throw AuthCanceledException();
    }
  }
}

void ViewerWindow::doAuthenticate(const int securityType,
                      RfbInputGate *input,
                      RfbOutputGate *output)
{
  switch (securityType) {
    case SecurityDefs::NONE:
      break;
    case SecurityDefs::VNC:
      {
        VncAuthentication authHandler;
        StringStorage strPassw;

        getPassword(&strPassw);
        authHandler.authenticate(input, 
                                 output, 
                                 &strPassw);
      }
      break;
    default:
      throw AuthUnknownException(_T("Unknown type of authentification in adapter"));
  }
}

void ViewerWindow::onCutText(const StringStorage *cutText)
{
  m_dsktWnd.setClipboardData(cutText);
}

void ViewerWindow::doCommand(int iCommand)
{
  postMessage(WM_COMMAND, iCommand);
}

bool ViewerWindow::isStopped() const
{
  return m_stopped;
}

void ViewerWindow::adjustWindowSize()
{
  Rect defaultSize = calculateDefaultSize();
  m_rcNormal = defaultSize.toWindowsRect();
  if (!m_isFullScr && !m_sizeIsChanged && !m_isMaximized) {
    setPosition(defaultSize.left, defaultSize.top);
    setSize(defaultSize.getWidth(), defaultSize.getHeight());
  }
}
