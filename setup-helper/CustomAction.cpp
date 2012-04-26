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

#include "win-system/RegistryKey.h"
#include <msiquery.h>

const TCHAR SAS_REG_ENTRY[] = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System");
const TCHAR SAS_REG_KEY[] = _T("SoftwareSASGeneration");

void allowSas()
{
  RegistryKey regKey(HKEY_LOCAL_MACHINE,
                     SAS_REG_ENTRY,
                     true);
  int sasValue = 0;
  regKey.getValueAsInt32(SAS_REG_KEY, &sasValue);
  sasValue |= 1;
  regKey.setValueAsInt32(SAS_REG_KEY, sasValue);
}

UINT __stdcall AllowSas(MSIHANDLE hInstall)
{
  allowSas();
  return ERROR_SUCCESS;
}

extern "C" BOOL WINAPI DllMain(
                               __in HINSTANCE hInst,
                               __in ULONG ulReason,
                               __in LPVOID
                               )
{
  return TRUE;
}
