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

#include "FileTransferMessageProcessor.h"
#include "ft-common/FTMessage.h"

FileTransferMessageProcessor::FileTransferMessageProcessor()
{
}

FileTransferMessageProcessor::~FileTransferMessageProcessor()
{
}

void FileTransferMessageProcessor::processRfbMessage(UINT32 rfbMessage)
     throw (IOException, ZLibException,
            OperationNotSupportedException,
            OperationNotPermittedException)
{
  AutoLock al(&m_listeners);

  for (size_t i = 0; i < m_listeners.size(); i++) {
    FileTransferEventHandler *listener = m_listeners.at(i);

    switch (rfbMessage) {
    case FTMessage::COMPRESSION_SUPPORT_REPLY:
      listener->onCompressionSupportReply();
      break;
    case FTMessage::FILE_LIST_REPLY:
      listener->onFileListReply();
      break;
    case FTMessage::DOWNLOAD_START_REPLY:
      listener->onDownloadReply();
      break;
    case FTMessage::DOWNLOAD_DATA_REPLY:
      listener->onDownloadDataReply();
      break;
    case FTMessage::DOWNLOAD_END_REPLY:
      listener->onDownloadEndReply();
      break;
    case FTMessage::UPLOAD_START_REPLY:
      listener->onUploadReply();
      break;
    case FTMessage::UPLOAD_DATA_REPLY:
      listener->onUploadDataReply();
      break;
    case FTMessage::UPLOAD_END_REPLY:
      listener->onUploadEndReply();
      break;
    case FTMessage::MD5_REPLY:
      listener->onMd5DataReply();
      break;
    case FTMessage::DIRSIZE_REPLY:
      listener->onDirSizeReply();
      break;
    case FTMessage::RENAME_REPLY:
      listener->onMvReply();
      break;
    case FTMessage::MKDIR_REPLY:
      listener->onMkdirReply();
      break;
    case FTMessage::REMOVE_REPLY:
      listener->onRmReply();
      break;
    case FTMessage::LAST_REQUEST_FAILED_REPLY:
      listener->onLastRequestFailedReply();
      break;
    } // switch
  } // for
}
