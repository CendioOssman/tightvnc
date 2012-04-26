// Copyright (C) 2009,2010,2011,2012 GlavSoft LLC.
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

#include "OperationSupport.h"

#include "ft-common/FTMessage.h"

#include "viewer-core/CapsContainer.h"

OperationSupport::OperationSupport()
{
  m_isFileListSupported = false;
  m_isRenameSupported = false;
  m_isRemoveSupported = false;
  m_isMkDirSupported = false;
  m_isCompressionSupported = false;
  m_isMD5Supported = false;
  m_isDirSizeSupported = false;
  m_isUploadSupported = false;
  m_isDownloadSupported = false;
}

OperationSupport::OperationSupport(const CapsContainer *clientCaps,
                                   const CapsContainer *serverCaps)
{
  m_isFileListSupported = ((clientCaps->isEnabled(FTMessage::FILE_LIST_REQUEST)) &&
                           (serverCaps->isEnabled(FTMessage::FILE_LIST_REPLY)));

  m_isRenameSupported = ((clientCaps->isEnabled(FTMessage::RENAME_REQUEST)) &&
                         (serverCaps->isEnabled(FTMessage::RENAME_REPLY)));

  m_isRemoveSupported = ((clientCaps->isEnabled(FTMessage::REMOVE_REQUEST)) &&
                         (serverCaps->isEnabled(FTMessage::REMOVE_REPLY)) &&
                         (m_isFileListSupported));

  m_isMkDirSupported = ((clientCaps->isEnabled(FTMessage::MKDIR_REQUEST)) &&
                        (serverCaps->isEnabled(FTMessage::MKDIR_REPLY)));

  m_isCompressionSupported = ((clientCaps->isEnabled(FTMessage::COMPRESSION_SUPPORT_REQUEST)) &&
                              (serverCaps->isEnabled(FTMessage::COMPRESSION_SUPPORT_REPLY)));

  m_isMD5Supported = ((clientCaps->isEnabled(FTMessage::MD5_REQUEST)) &&
                      (serverCaps->isEnabled(FTMessage::MD5_REPLY)));

  m_isDirSizeSupported = ((clientCaps->isEnabled(FTMessage::DIRSIZE_REQUEST)) &&
                          (serverCaps->isEnabled(FTMessage::DIRSIZE_REPLY)));

  m_isUploadSupported = ((clientCaps->isEnabled(FTMessage::UPLOAD_START_REQUEST)) &&
                         (clientCaps->isEnabled(FTMessage::UPLOAD_DATA_REQUEST)) &&
                         (clientCaps->isEnabled(FTMessage::UPLOAD_END_REQUEST)) &&
                         (serverCaps->isEnabled(FTMessage::UPLOAD_START_REPLY)) &&
                         (serverCaps->isEnabled(FTMessage::UPLOAD_DATA_REPLY)) &&
                         (serverCaps->isEnabled(FTMessage::UPLOAD_END_REPLY)) &&
                         m_isMkDirSupported && m_isFileListSupported);

  m_isDownloadSupported = ((clientCaps->isEnabled(FTMessage::DOWNLOAD_START_REQUEST)) &&
                           (clientCaps->isEnabled(FTMessage::DOWNLOAD_DATA_REQUEST)) &&
                           (serverCaps->isEnabled(FTMessage::DOWNLOAD_START_REPLY)) &&
                           (serverCaps->isEnabled(FTMessage::DOWNLOAD_DATA_REPLY)) &&
                           (serverCaps->isEnabled(FTMessage::DOWNLOAD_END_REPLY)) &&
                           m_isFileListSupported && m_isDirSizeSupported);
}

OperationSupport::~OperationSupport()
{
}

bool OperationSupport::isFileListSupported() const
{
  return m_isFileListSupported;
}

bool OperationSupport::isUploadSupported() const
{
  return m_isUploadSupported;
}

bool OperationSupport::isDownloadSupported() const
{
  return m_isDownloadSupported;
}

bool OperationSupport::isRenameSupported() const
{
  return m_isRenameSupported;
}

bool OperationSupport::isRemoveSupported() const
{
  return m_isRemoveSupported;
}

bool OperationSupport::isMkDirSupported() const
{
  return m_isMkDirSupported;
}

bool OperationSupport::isCompressionSupported() const
{
  return m_isCompressionSupported;
}

bool OperationSupport::isMD5Supported() const
{
  return m_isMD5Supported;
}

bool OperationSupport::isDirSizeSupported() const
{
  return m_isDirSizeSupported;
}
