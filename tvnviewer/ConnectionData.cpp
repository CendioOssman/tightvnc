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

#include <iomanip>
#include <sstream>

#include "ConnectionData.h"
#include "util/AnsiStringStorage.h"
#include "util/VncPassCrypt.h"
#include "viewer-core/VncAuthentication.h"

ConnectionData::ConnectionData()
: m_isEmpty(true)
{
}

bool ConnectionData::isEmpty() const
{
  return m_isEmpty;
}

void ConnectionData::setHost(const StringStorage *host)
{
  StringStorage chompedString = *host;
  TCHAR spaceChar[] = _T(" \t\n\r");
  chompedString.removeChars(spaceChar, sizeof(spaceChar));

  AnsiStringStorage ansiStr(&chompedString);

  set(ansiStr.getString());
  m_isEmpty = false;
}

const StringStorage *ConnectionData::getDefaultPassword() const
{
  return &m_defaultPassword;
}

void ConnectionData::setPlainPassword(const StringStorage *password)
{
  m_defaultPassword = *password;
}

void ConnectionData::setCryptedPassword(const StringStorage *hidePassword)
{
  AnsiStringStorage ansiHidePassword(hidePassword);
  UINT8 encPassword[VncAuthentication::VNC_PASSWORD_SIZE];
  for (size_t i = 0; i < VncAuthentication::VNC_PASSWORD_SIZE; ++i) {
    std::stringstream passwordStream;
    passwordStream << ansiHidePassword.getString()[i * 2]
                   << ansiHidePassword.getString()[i * 2 + 1];
    int ordOfSymbol = 0;
    passwordStream >> std::hex >> ordOfSymbol;
    encPassword[i] = static_cast<UINT8>(ordOfSymbol);
  }
  UINT8 plainPassword[VncAuthentication::VNC_PASSWORD_SIZE];
  VncPassCrypt::getPlainPass(plainPassword, encPassword);

  AnsiStringStorage ansiPlainPassword(reinterpret_cast<char *>(plainPassword));
  StringStorage password;
  ansiPlainPassword.toStringStorage(&password);
  setPlainPassword(&password);
}

void ConnectionData::getCryptedPassword(StringStorage *hidePassword) const
{
  AnsiStringStorage ansiPlainPassword(getDefaultPassword());
  UINT8 plainPassword[VncAuthentication::VNC_PASSWORD_SIZE];
  UINT8 encryptedPassword[VncAuthentication::VNC_PASSWORD_SIZE];
  memcpy(plainPassword, ansiPlainPassword.getString(), VncAuthentication::VNC_PASSWORD_SIZE);
  VncPassCrypt::getEncryptedPass(encryptedPassword, plainPassword);
  UINT8 hidePasswordChars[VncAuthentication::VNC_PASSWORD_SIZE * 2 + 1];
  hidePasswordChars[VncAuthentication::VNC_PASSWORD_SIZE * 2] = 0;
  for (size_t i = 0; i < VncAuthentication::VNC_PASSWORD_SIZE; ++i) {
    std::stringstream passwordStream;
    int ordOfSymbol = encryptedPassword[i];
    passwordStream << std::hex << setw(2) << setfill('0') << ordOfSymbol;
    passwordStream >> hidePasswordChars[i * 2] >> hidePasswordChars[i * 2 + 1];
  }
  AnsiStringStorage ansiHidePassword(reinterpret_cast<char *>(hidePasswordChars));
  ansiHidePassword.toStringStorage(hidePassword);
}

void ConnectionData::getHost(StringStorage *strHost) const
{
  if (sizeof(TCHAR) == 1) {
    *strHost = reinterpret_cast<const TCHAR *>(get());
  } else {
    AnsiStringStorage ansiStr(get());

    ansiStr.toStringStorage(strHost);
  }
}

const StringStorage ConnectionData::getHost() const
{
  StringStorage host;
  AnsiStringStorage hostAnsi(get());
  hostAnsi.toStringStorage(&host);
  return host;
}

void ConnectionData::getReducedHost(StringStorage *strHost) const
{
  if (sizeof(TCHAR) == 1) {
    *strHost = reinterpret_cast<const TCHAR *>(getVncHost());
  } else {
    AnsiStringStorage ansiStr(getVncHost());

    ansiStr.toStringStorage(strHost);
  }
}

int ConnectionData::getPort() const
{
  return getVncPort();
}
