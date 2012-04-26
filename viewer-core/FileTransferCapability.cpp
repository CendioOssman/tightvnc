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


#include "FileTransferCapability.h"

#include "ft-client-lib/FileTransferRequestSender.h"
#include "ft-client-lib/FileTransferReplyBuffer.h"
#include "ft-client-lib/FileTransferMessageProcessor.h"
#include "ft-client-lib/FileTransferCore.h"

#include "ft-common/FTMessage.h"
#include "rfb/VendorDefs.h"


FileTransferCapability::FileTransferCapability()
: m_ftRequestSender(0),
  m_ftReplyBuffer(0),
  m_ftMessageProcessor(0),
  m_ftCore(0)
{
  addClientCaps();
  addServerCaps();
}

FileTransferCapability::~FileTransferCapability()
{
  if (m_ftCore != 0) {
    try {
      delete m_ftCore;
    } catch (...) {
    }
  }

  if (m_ftMessageProcessor != 0) {
    try {
      delete m_ftMessageProcessor;
    } catch (...) {
    }
  }

  if (m_ftReplyBuffer != 0) {
    try {
      delete m_ftReplyBuffer;
    } catch (...) {
    }
  }

  if (m_ftRequestSender != 0) {
    try {
      delete m_ftRequestSender;
    } catch (...) {
    }
  }
}

bool FileTransferCapability::isEnabled()
{
  return (m_clientMsgCaps.isEnabled(FTMessage::FILE_LIST_REQUEST) && 
          m_serverMsgCaps.isEnabled(FTMessage::FILE_LIST_REPLY));
}

void FileTransferCapability::bind(RfbInputGate *input, RfbOutputGate *output)
{
  MsgCapability::bind(input, output);

  m_ftRequestSender = new FileTransferRequestSender(m_output);
  m_ftReplyBuffer = new FileTransferReplyBuffer(m_input);
  m_ftMessageProcessor = new FileTransferMessageProcessor;
  m_ftMessageProcessor->addListener(m_ftReplyBuffer);

  m_ftCore = new FileTransferCore(m_ftRequestSender,
                                  m_ftReplyBuffer,
                                  m_ftMessageProcessor);
  m_ftCore->updateSupportedOperations(&m_clientMsgCaps, &m_serverMsgCaps);
}

FileTransferCore *FileTransferCapability::getCore()
{
  return m_ftCore;
}

void FileTransferCapability::setInterface(FileTransferInterface *ftInterface)
{
  m_ftCore->setInterface(ftInterface);
}

void FileTransferCapability::listenerMessage(RfbInputGate *input, INT32 msgType)
{
  m_ftMessageProcessor->processRfbMessage(msgType);
}

void FileTransferCapability::addClientCaps()
{
  m_clientMsgCaps.add(FTMessage::COMPRESSION_SUPPORT_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::COMPRESSION_SUPPORT_REQUEST_SIG,
                      _T("Compression support"));

  m_clientMsgCaps.add(FTMessage::FILE_LIST_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::FILE_LIST_REQUEST_SIG,
                      _T("File list request"));

  m_clientMsgCaps.add(FTMessage::MD5_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::MD5_REQUEST_SIG,
                      _T("File md5 sum request"));

  m_clientMsgCaps.add(FTMessage::DIRSIZE_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::DIRSIZE_REQUEST_SIG,
                      _T("Directory size request"));

  m_clientMsgCaps.add(FTMessage::RENAME_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::RENAME_REQUEST_SIG,
                      _T("File move request"));

  m_clientMsgCaps.add(FTMessage::MKDIR_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::MKDIR_REQUEST_SIG,
                      _T("Directory create request"));

  m_clientMsgCaps.add(FTMessage::REMOVE_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::REMOVE_REQUEST_SIG,
                      _T("File remove request"));

  m_clientMsgCaps.add(FTMessage::DOWNLOAD_START_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::DOWNLOAD_START_REQUEST_SIG,
                      _T("File download start request"));

  m_clientMsgCaps.add(FTMessage::DOWNLOAD_DATA_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::DOWNLOAD_DATA_REQUEST_SIG,
                      _T("File download data request"));

  m_clientMsgCaps.add(FTMessage::UPLOAD_START_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::UPLOAD_START_REQUEST_SIG,
                      _T("File upload start request"));

  m_clientMsgCaps.add(FTMessage::UPLOAD_DATA_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::UPLOAD_DATA_REQUEST_SIG,
                      _T("File upload data request"));

  m_clientMsgCaps.add(FTMessage::UPLOAD_END_REQUEST,
                      VendorDefs::TIGHTVNC,
                      FTMessage::UPLOAD_END_REQUEST_SIG,
                      _T("File upload end request"));
}

void FileTransferCapability::addServerCaps()
{
  addServerCapability(FTMessage::COMPRESSION_SUPPORT_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::COMPRESSION_SUPPORT_REPLY_SIG,
                      _T("Compression support"));

  addServerCapability(FTMessage::FILE_LIST_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::FILE_LIST_REPLY_SIG,
                      _T("File list reply"));

  addServerCapability(FTMessage::LAST_REQUEST_FAILED_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::LAST_REQUEST_FAILED_REPLY_SIG,
                      _T("Last request failed"));

  addServerCapability(FTMessage::MD5_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::MD5_REPLY_SIG,
                      _T("File md5 sum reply"));

  addServerCapability(FTMessage::DIRSIZE_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::DIRSIZE_REPLY_SIG,
                      _T("Directory size reply"));

  addServerCapability(FTMessage::RENAME_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::RENAME_REPLY_SIG,
                      _T("File move reply"));

  addServerCapability(FTMessage::MKDIR_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::MKDIR_REPLY_SIG,
                      _T("Directory create reply"));

  addServerCapability(FTMessage::REMOVE_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::REMOVE_REPLY_SIG,
                     _T("File remove reply"));

  addServerCapability(FTMessage::DOWNLOAD_START_REPLY,
                     VendorDefs::TIGHTVNC,
                     FTMessage::DOWNLOAD_START_REPLY_SIG,
                     _T("File download start reply"));

  addServerCapability(FTMessage::DOWNLOAD_DATA_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::DOWNLOAD_DATA_REPLY_SIG,
                      _T("File download data reply"));

  addServerCapability(FTMessage::DOWNLOAD_END_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::DOWNLOAD_END_REPLY_SIG,
                      _T("File download end reply"));

  addServerCapability(FTMessage::UPLOAD_START_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::UPLOAD_START_REPLY_SIG,
                      _T("File upload start reply"));

  addServerCapability(FTMessage::UPLOAD_DATA_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::UPLOAD_DATA_REPLY_SIG,
                      _T("File upload data reply"));

  addServerCapability(FTMessage::UPLOAD_END_REPLY,
                      VendorDefs::TIGHTVNC,
                      FTMessage::UPLOAD_END_REPLY_SIG,
                      _T("File upload end reply"));
}

void FileTransferCapability::addServerCapability(UINT32 code, const char *vendor, const char *name,
                                                 const StringStorage desc)
{
  m_serverMsgCaps.add(code, vendor, name, desc);
  m_availableServerMsg[code] = true;
}
