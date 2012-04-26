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

#ifndef _MSG_CAPABILITY_H_
#define _MSG_CAPABILITY_H_

#include <map>

#include "network/RfbInputGate.h"
#include "network/RfbOutputGate.h"

#include "CapsContainer.h"

class MsgCapability
{
public:
  MsgCapability();
  virtual ~MsgCapability();

  virtual bool isEnabled();
  virtual void bind(RfbInputGate *input, RfbOutputGate *output);
  virtual void listenerMessage(RfbInputGate *input, INT32 msgType) = 0;

  virtual bool isMsgSupported(INT32 msg);

  virtual void enableClientMsg(const RfbCapabilityInfo *capability);
  virtual void enableServerMsg(const RfbCapabilityInfo *capability);
protected:
  RfbInputGate *m_input;
  RfbOutputGate *m_output;

  CapsContainer m_clientMsgCaps;
  CapsContainer m_serverMsgCaps;

  map<INT32, bool> m_availableServerMsg;
};

#endif
