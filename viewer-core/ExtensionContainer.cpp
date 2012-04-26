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

#include "ExtensionContainer.h"

ExtensionContainer::ExtensionContainer()
{
}

ExtensionContainer::~ExtensionContainer()
{
}

void ExtensionContainer::add(MsgCapability *capability)
{
  m_extensions.insert(capability);
}

void ExtensionContainer::bind(RfbInputGate *input, RfbOutputGate *output)
{
  for (ExtensionIter i = m_extensions.begin(); i != m_extensions.end(); ++i)
    (*i)->bind(input, output);
}

void ExtensionContainer::listenerMessage(RfbInputGate *input, INT32 msgType)
{
  for (ExtensionIter i = m_extensions.begin(); i != m_extensions.end(); ++i)
    (*i)->listenerMessage(input, msgType);
}

bool ExtensionContainer::isMsgSupported(INT32 msg)
{
  bool isSupported = false;
  for (ExtensionIter i = m_extensions.begin(); i != m_extensions.end(); ++i)
    isSupported |= (*i)->isMsgSupported(msg);
  return isSupported;
}

void ExtensionContainer::enableClientMsg(const RfbCapabilityInfo *capability)
{
  for (ExtensionIter i = m_extensions.begin(); i != m_extensions.end(); ++i)
    (*i)->enableClientMsg(capability);
}

void ExtensionContainer::enableServerMsg(const RfbCapabilityInfo *capability)
{
  for (ExtensionIter i = m_extensions.begin(); i != m_extensions.end(); ++i)
    (*i)->enableServerMsg(capability);
}
