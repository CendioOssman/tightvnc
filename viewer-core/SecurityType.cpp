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

#include "SecurityType.h"

#include "rfb/AuthDefs.h"
#include "util/Exception.h"

int SecurityType::selectAuthHandler(const vector<UINT32> *secTypes)
{
  for (vector<UINT32>::const_iterator i = secTypes->begin();
       i != secTypes->end();
       i++) {
         if (*i == SecurityDefs::TIGHT)
    return SecurityDefs::TIGHT;
  }

  for (vector<UINT32>::const_iterator i = secTypes->begin();
       i != secTypes->end();
       i++) {
    if (*i == SecurityDefs::NONE || *i == SecurityDefs::VNC)
      return *i;
  }

  throw Exception(_T("No security types supported. ")
                  _T("Server sent security types, ")
                  _T("but we do not support any of their."));
}

StringStorage SecurityType::getSecurityTypeName(int securityType)
{
  switch (securityType) {
  case SecurityDefs::NONE:
    return _T("None");
  case SecurityDefs::VNC:
    return _T("VNC");
  case SecurityDefs::TIGHT:
    return _T("Tight");
  default:
    return _T("unknown type");
  }
}

