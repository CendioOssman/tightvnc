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

#ifndef _AUTH_HANDLER_H_
#define _AUTH_HANDLER_H_

#include "log-server/Log.h"
#include "network/RfbInputGate.h"
#include "network/RfbOutputGate.h"
#include "rfb/AuthDefs.h"
#include "util/Exception.h"

class AuthException : public Exception
{
public:
  AuthException(const TCHAR *message = _T("Error in authentication"));
  virtual ~AuthException();
  int getAuthCode() const;

public:
  static const int AUTH_ERROR = 1;
  static const int AUTH_UNKNOWN_TYPE = 2;
  static const int AUTH_CANCELED = 3;

protected:
  int m_authErrorCode;
};

class AuthUnknownException : public AuthException 
{
public:
  AuthUnknownException(const TCHAR *message = _T("Error in authentification: ")
                                              _T("auth is canceled or isn't support"));
  virtual ~AuthUnknownException();
};

class AuthCanceledException : public AuthException
{
public:
  AuthCanceledException(const TCHAR *message = _T("Auth is canceled"));
  virtual ~AuthCanceledException();
};

class AuthHandler
{
public:
  AuthHandler();
  virtual ~AuthHandler();

  virtual void authenticate(RfbInputGate *input, RfbOutputGate *output, 
                            const StringStorage *password) = 0;
  int getType() const;

  static const UINT32 AUTH_RESULT_OK = 0;

protected:
  int m_id;
};

#endif
