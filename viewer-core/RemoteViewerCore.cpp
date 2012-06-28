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

#include "RemoteViewerCore.h"

#include "ft-common/FTMessage.h"
#include "rfb/AuthDefs.h"
#include "rfb/MsgDefs.h"
#include "rfb/EncodingDefs.h"
#include "rfb/VendorDefs.h"
#include "util/AnsiStringStorage.h"

#include "AuthHandler.h"
#include "SecurityType.h"
#include "RichCursorDecoder.h"
#include "RfbFramebufferUpdateRequestClientMessage.h"
#include "RfbCutTextEventClientMessage.h"
#include "RfbKeyEventClientMessage.h"
#include "RfbPointerEventClientMessage.h"
#include "RfbSetEncodingsClientMessage.h"
#include "RfbSetPixelFormatClientMessage.h"

#include "Decoder.h"
#include "CopyRectDecoder.h"
#include "RreDecoder.h"
#include "HexTileDecoder.h"
#include "TightDecoder.h"
#include "ZrleDecoder.h"

#include "JpegQualityLevel.h"
#include "CompressionLevel.h"

#include "DesktopSizeDecoder.h"
#include "LastRectDecoder.h"
#include "PointerPosDecoder.h"
#include "RichCursorDecoder.h"

RemoteViewerCore::RemoteViewerCore(Logger *logger)
: m_logWriter(logger),
  m_tcpConnection(&m_logWriter),
  m_fbUpdateNotifier(&m_frameBuffer, &m_fbLock, &m_logWriter),
  m_decoderStore(&m_logWriter)
{
  init();
}

RemoteViewerCore::RemoteViewerCore(const TCHAR *host, UINT16 port,
                                   CoreEventsAdapter *adapter,
                                   Logger *logger,
                                   bool sharedFlag)
: m_logWriter(logger),
  m_tcpConnection(&m_logWriter),
  m_fbUpdateNotifier(&m_frameBuffer, &m_fbLock, &m_logWriter),
  m_decoderStore(&m_logWriter)
{
  init();

  start(host, port, adapter, sharedFlag);
}

RemoteViewerCore::RemoteViewerCore(SocketIPv4 *socket,
                                   CoreEventsAdapter *adapter,
                                   Logger *logger,
                                   bool sharedFlag)
: m_logWriter(logger),
  m_tcpConnection(&m_logWriter),
  m_fbUpdateNotifier(&m_frameBuffer, &m_fbLock, &m_logWriter),
  m_decoderStore(&m_logWriter)
{
  init();

  start(socket, adapter, sharedFlag);
}

RemoteViewerCore::RemoteViewerCore(RfbInputGate *input, RfbOutputGate *output,
                                   CoreEventsAdapter *adapter,
                                   Logger *logger,
                                   bool sharedFlag)
: m_logWriter(logger),
  m_tcpConnection(&m_logWriter),
  m_fbUpdateNotifier(&m_frameBuffer, &m_fbLock, &m_logWriter),
  m_decoderStore(&m_logWriter)
{
  init();

  start(input, output, adapter, sharedFlag);
}

void RemoteViewerCore::init()
{
  m_decoderStore.addDecoder(new Decoder(&m_logWriter), 0);
  m_decoderStore.addDecoder(new CopyRectDecoder(&m_logWriter), 10);
  m_decoderStore.addDecoder(new RreDecoder(&m_logWriter), 1);
  m_decoderStore.addDecoder(new HexTileDecoder(&m_logWriter), 4);
  m_decoderStore.addDecoder(new TightDecoder(&m_logWriter), 9);
  m_decoderStore.addDecoder(new ZrleDecoder(&m_logWriter), 9);

  m_decoderStore.addDecoder(new DesktopSizeDecoder(&m_logWriter), -1);
  m_decoderStore.addDecoder(new LastRectDecoder(&m_logWriter), -1);
  m_decoderStore.addDecoder(new PointerPosDecoder(&m_logWriter), -1);
  m_decoderStore.addDecoder(new RichCursorDecoder(&m_logWriter), -1);

  m_input = 0;
  m_output = 0;

  m_wasStarted = false;
  m_wasConnected = false;
  m_isNewPixelFormat = false;
  m_isFreeze = false;
  m_isNeedRequestUpdate = true;
}

RemoteViewerCore::~RemoteViewerCore()
{
  try {
    stop();
    waitTermination();
  } catch (...) {
  }
}

void RemoteViewerCore::start(CoreEventsAdapter *adapter,
                             bool sharedFlag)
{
  m_logWriter.detail(_T("Starting remote viewer core..."));

  {
    AutoLock al(&m_startLock);
    if (m_wasStarted)
      throw Exception(_T("Remote viewer core is already started"));
    m_wasStarted = true;
  }
  m_sharedFlag = sharedFlag;
  m_adapter = adapter;

  m_fbUpdateNotifier.setAdapter(adapter);
  // thread started
  resume();
  m_logWriter.detail(_T("Remote viewer core is started"));
}

void RemoteViewerCore::start(const TCHAR *host,
                             UINT16 port,
                             CoreEventsAdapter *adapter,
                             bool sharedFlag)
{
  m_tcpConnection.bind(host, port);
  start(adapter, sharedFlag);
}
void RemoteViewerCore::start(SocketIPv4 *socket,
                             CoreEventsAdapter *adapter,
                             bool sharedFlag)
{
  m_tcpConnection.bind(socket);
  start(adapter, sharedFlag);
}

void RemoteViewerCore::start(RfbInputGate *input, RfbOutputGate *output,
                             CoreEventsAdapter *adapter,
                             bool sharedFlag)
{
  m_tcpConnection.bind(input, output);
  start(adapter, sharedFlag);
}

bool RemoteViewerCore::wasStarted() const
{
  AutoLock al(&m_startLock);
  return m_wasStarted;
}

void RemoteViewerCore::stop()
{
  m_tcpConnection.close();
  m_fbUpdateNotifier.terminate();
  terminate();
}

void RemoteViewerCore::waitTermination()
{
  if (m_fbUpdateNotifier.isActive()) {
    m_fbUpdateNotifier.wait();
  }
  if (isActive()) {
    wait();
  }
}

void RemoteViewerCore::setPixelFormat(const PixelFormat *pixelFormat)
{
  m_logWriter.info(_T("Pixel format will changed"));
  AutoLock al(&m_pixelFormatLock);
  m_isNewPixelFormat = true;
  m_viewerPixelFormat = *pixelFormat;
}

bool RemoteViewerCore::updatePixelFormat()
{
  PixelFormat pxFormat;
  m_logWriter.info(_T("Updating pixel format..."));
  {
    AutoLock al(&m_pixelFormatLock);
    if (!m_isNewPixelFormat)
      return false;
    m_isNewPixelFormat = false;
    pxFormat = m_viewerPixelFormat;
  }

  {
    AutoLock al(&m_fbLock);
    if (pxFormat.isEqualTo(&m_frameBuffer.getPixelFormat()))
      return false;
    if (m_frameBuffer.getBuffer() != 0)
      setFbProperties(&m_frameBuffer.getDimension(), &pxFormat);
  }

  RfbSetPixelFormatClientMessage pixelFormatMessage(&pxFormat);
  pixelFormatMessage.send(m_output);

  return true;
}

void RemoteViewerCore::refreshFrameBuffer()
{
  m_logWriter.info(_T("Frame buffer will refreshed"));
  AutoLock al(&m_refreshingLock);
  m_isRefreshing = true;
}

void RemoteViewerCore::sendFbUpdateRequest(bool incremental)
{
  {
    AutoLock al(&m_requestUpdateLock);
    bool requestUpdate = m_isNeedRequestUpdate;
    m_isNeedRequestUpdate = false;
    if (!requestUpdate)
      return;
  }

  bool isRefresh = false;
  bool isUpdateFbProperties = false;
  if (updatePixelFormat()) {
    isUpdateFbProperties = true;
  }

  {
    AutoLock al(&m_refreshingLock);
    if (m_isRefreshing) {
      m_isRefreshing= false;
      isRefresh = true;
    }
  }

  bool isIncremental = incremental && !isRefresh && !isUpdateFbProperties;
  if (isIncremental)
    m_logWriter.info(_T("Sending frame buffer incremental update request..."));
  else
    m_logWriter.info(_T("Sending frame buffer full update request..."));
  Rect updateRect;
  {
    AutoLock al(&m_fbLock);
    updateRect = m_frameBuffer.getDimension().getRect();
  }
  RfbFramebufferUpdateRequestClientMessage fbUpdReq(isIncremental, updateRect);
  fbUpdReq.send(m_output);
  m_logWriter.detail(_T("Frame buffer update request is sent"));
}

void RemoteViewerCore::sendKeyboardEvent(bool downFlag, UINT32 key)
{
  {
    AutoLock al(&m_connectLock);
    if (!m_wasConnected)
      return;
  }
  m_logWriter.info(_T("Sending key event: %d, %d..."), downFlag, key);
  RfbKeyEventClientMessage keyMessage(downFlag, key);
  keyMessage.send(m_output);
  m_logWriter.detail(_T("Key event: %d, %d is sent"), downFlag, key);
}

void RemoteViewerCore::sendPointerEvent(UINT8 buttonMask,
                                        const Point *position)
{
  {
    AutoLock al(&m_connectLock);
    if (!m_wasConnected)
      return;
  }
  m_logWriter.info(_T("Sending pointer event %d, (%d, %d)..."),
                   buttonMask, position->x, position->y);
  // send position to server
  RfbPointerEventClientMessage pointerMessage(buttonMask, position);
  pointerMessage.send(m_output);
  // update position
  m_fbUpdateNotifier.updatePointerPos(position);

  m_logWriter.detail(_T("Pointer event: %d, (%d, %d) is sent"),
                     buttonMask, position->x, position->y);
}

void RemoteViewerCore::sendCutTextEvent(const StringStorage *cutText)
{
  {
    AutoLock al(&m_connectLock);
    if (!m_wasConnected)
      return;
  }
  m_logWriter.info(_T("Sending clipboard cut text: \"%s\"..."), cutText->getString());
  RfbCutTextEventClientMessage cutTextMessage(cutText);
  cutTextMessage.send(m_output);
  m_logWriter.debug(_T("Clipboard cut text: \"%s\" is sent"), cutText->getString());
}

void RemoteViewerCore::addExtension(MsgCapability *capability)
{
  m_extensions.add(capability);
}

void RemoteViewerCore::setPreferredEncoding(INT32 encodingType)
{
  m_decoderStore.setPreferredEncoding(encodingType);
  sendEncodings();
}

void RemoteViewerCore::allowCopyRect(bool allow)
{
  m_decoderStore.allowCopyRect(allow);
  sendEncodings();
}

void RemoteViewerCore::setCompressionLevel(int newLevel)
{
  bool needUpdate = false;
  for (int level = CompressionLevel::COMPRESSION_LEVEL_MIN;
       level <= CompressionLevel::COMPRESSION_LEVEL_MAX;
       level++)
    if (level != newLevel)
      needUpdate |= m_decoderStore.removeDecoder(CompressionLevel(&m_logWriter, level).getCode());

  // new compression level is valid?
  if (newLevel < CompressionLevel::COMPRESSION_LEVEL_MIN ||
      newLevel > CompressionLevel::COMPRESSION_LEVEL_MAX)
    return;

  needUpdate |= m_decoderStore.addDecoder(new CompressionLevel(&m_logWriter, newLevel), -1);
  if (needUpdate) {
    sendEncodings();
  }
}

void RemoteViewerCore::setJpegQualityLevel(int newLevel)
{
  bool needUpdate = false;
  for (int level = JpegQualityLevel::JPEG_QUALITY_LEVEL_MIN;
       level <= JpegQualityLevel::JPEG_QUALITY_LEVEL_MAX;
       level++)
    if (level != newLevel)
      needUpdate |= m_decoderStore.removeDecoder(JpegQualityLevel(&m_logWriter, level).getCode());

  // new jpeg quality level is valid?
  if (newLevel < JpegQualityLevel::JPEG_QUALITY_LEVEL_MIN ||
      newLevel > JpegQualityLevel::JPEG_QUALITY_LEVEL_MAX)
    return;

  needUpdate |= m_decoderStore.addDecoder(new JpegQualityLevel(&m_logWriter, newLevel), -1);
  if (needUpdate) {
    sendEncodings();
  }
}

void RemoteViewerCore::enableCursorShapeUpdates(bool enabled)
{
  bool needUpdate = false;
  if (enabled) {
    needUpdate |= m_decoderStore.addDecoder(new RichCursorDecoder(&m_logWriter), -1);
    needUpdate |= m_decoderStore.addDecoder(new PointerPosDecoder(&m_logWriter), -1);
  } else {
    needUpdate |= m_decoderStore.removeDecoder(PseudoEncDefs::RICH_CURSOR);
    needUpdate |= m_decoderStore.removeDecoder(PseudoEncDefs::POINTER_POS);
  }
  
  if (needUpdate) {
    sendEncodings();
  }
}

void RemoteViewerCore::ignoreCursorShapeUpdates(bool ignored)
{
  m_fbUpdateNotifier.setIgnoreShapeUpdates(ignored);
}

void RemoteViewerCore::stopUpdating(bool isStopped)
{
  {
    AutoLock al(&m_freezeLock);
    if (isStopped == m_isFreeze)
      return;
    m_isFreeze = isStopped;
  }
  if (!isStopped) {
    m_logWriter.info(_T("Sending of frame buffer update request..."));
    sendFbUpdateRequest();
  }
}

PixelFormat RemoteViewerCore::readPixelFormat()
{
  PixelFormat pixelFormat;
  pixelFormat.bitsPerPixel = m_input->readUInt8();
  pixelFormat.colorDepth = m_input->readUInt8();
  pixelFormat.bigEndian = !!m_input->readUInt8();
  // now, supported only true color
  int trueColour = m_input->readUInt8();
  if (trueColour == false) {
    m_logWriter.error(_T("Not supported palette. Flag \"True colour\" is not set."));
  }
  pixelFormat.redMax = m_input->readUInt16();
  pixelFormat.greenMax = m_input->readUInt16();
  pixelFormat.blueMax = m_input->readUInt16();
  pixelFormat.redShift = m_input->readUInt8();
  pixelFormat.greenShift = m_input->readUInt8();
  pixelFormat.blueShift = m_input->readUInt8();
  m_input->readUInt8(); // padding bytes (3)
  m_input->readUInt8();
  m_input->readUInt8();
  return pixelFormat;
}

void RemoteViewerCore::connectToHost()
{
  m_tcpConnection.connect();
  m_input = m_tcpConnection.getInput();
  m_output = m_tcpConnection.getOutput();
  m_logWriter.detail(_T("Connection is established"));
  m_adapter->onEstablished();
}

void RemoteViewerCore::authenticate()
{
  m_logWriter.info(_T("Negotiating about security type..."));
  int securityType = negotiateAboutSecurityType();
  m_logWriter.message(_T("Security type accepted: %s"),
                      SecurityType::getSecurityTypeName(securityType).getString());

  if (securityType) {
    m_logWriter.info(_T("Authentication..."));
    m_adapter->doAuthenticate(securityType, m_input, m_output);
  }

  // get authentication result, if version 3.8 or authentication isn't None
  if (m_minor >= 8 || securityType != SecurityDefs::NONE) {
    UINT32 authResult = 0;
    if (securityType) {
      authResult = m_input->readUInt32();
      m_logWriter.detail(_T("Auth result is %d"), authResult);
    }
    if (!securityType || authResult != AuthHandler::AUTH_RESULT_OK) {
      // if version 3.3 or 3.7 then server connection closed
      m_logWriter.warning(_T("Authentication failure"));
      if (m_minor < 8) {
        throw AuthException(_T("Authentication failure"));
      }
      // if version 3.8 then try read reasonAuth.
      StringStorage reasonAuth;
      m_input->readUTF8(&reasonAuth);
      StringStorage errorMessage = _T("Authentication reason: ");
      errorMessage.appendString(reasonAuth.getString());
      m_logWriter.warning(_T("%s"), errorMessage.getString());
      throw AuthException(errorMessage.getString());
    }
  }
}

int RemoteViewerCore::negotiateAboutSecurityType()
{
  m_logWriter.info(_T("Reading list of security types..."));
  // read list of security types
  vector<UINT32> secTypes;
  readSecurityTypeList(&secTypes);
  m_logWriter.detail(_T("List of security type is read"));
  if (secTypes.size() == 0) {
    m_logWriter.warning(_T("Error in negotiate about of security: only security type is 0"));
    return 0;
  }

  // log information about security list
  StringStorage secTypeString;
  for (vector<UINT32>::iterator i = secTypes.begin(); i != secTypes.end(); i++) {
    if(i != secTypes.begin())
      secTypeString.appendString(_T(", "));
    secTypeString.appendString(SecurityType::getSecurityTypeName(*i).getString());
  }
  m_logWriter.info(_T("Security Types received (%d): %s"),
                   secTypes.size(), secTypeString.getString()); 

  // select type security
  m_logWriter.info(_T("Selecting auth-handler"));
  int typeSelected = SecurityType::selectAuthHandler(&secTypes);
  m_logWriter.detail(_T("Auth-handler is selected"));
  if (typeSelected == SecurityDefs::TIGHT) {
    m_logWriter.info(_T("Enable tight capabilities"));
    m_isTight = true;

    m_output->writeUInt8(typeSelected);
    m_output->flush();

    initTunnelling();
    return initAuthentication();
  }

  if (m_minor >= 7) {
    m_output->writeUInt8(typeSelected);
    m_output->flush();
  }

  return typeSelected;
}

void RemoteViewerCore::readSecurityTypeList(vector<UINT32> *secTypes)
{
  if (m_minor < 7) {
    UINT32 type = m_input->readUInt32();
    if (type != 0)
      secTypes->push_back(type);
  } else { // m_minor >= 7
    int secTypesNumber = m_input->readUInt8();
    if (secTypesNumber != 0) {
      secTypes->resize(secTypesNumber);
      for (vector<UINT32>::iterator i = secTypes->begin(); i != secTypes->end(); i++)
        *i = m_input->readUInt8();
    }
  }
}

void RemoteViewerCore::initTunnelling()
{
  m_logWriter.info(_T("Initialization of tight-tunneling..."));
  UINT32 tunnelCount = m_input->readUInt32();
  if (tunnelCount > 0) {
    m_logWriter.error(_T("Viewer not support tunneling in tight-authentication"));
    throw Exception(_T("Viewer not support tunneling in tight-authentication"));
  }
  m_logWriter.detail(_T("Tunneling is init"));
}

int RemoteViewerCore::initAuthentication()
{
  m_logWriter.info(_T("Initialization of tight-authentication..."));
  UINT32 authTypesNumber = m_input->readUInt32();

  m_logWriter.detail(_T("Number of auth-types is %d"), authTypesNumber);
  if (authTypesNumber == 0) {
    return AuthDefs::NONE;
  }

  m_logWriter.detail(_T("Reading authentication capability..."));
  vector<UINT32> secTypes;
  secTypes.resize(authTypesNumber);
  for (UINT32 i = 0; i < authTypesNumber; i++) {
    RfbCapabilityInfo cap = readCapability();
    secTypes[i] = SecurityDefs::convertFromAuthType(cap.code);
  }
  m_logWriter.debug(_T("Authentication capability is read"));

  m_logWriter.detail(_T("Select authentication capability"));
  int typeSelected = SecurityType::selectAuthHandler(&secTypes);
  m_logWriter.debug(_T("Selected type of authentication: %d"), typeSelected);

  m_output->writeUInt32(typeSelected);
  m_output->flush();

  m_logWriter.info(_T("Tight authentication is init"));
  return typeSelected;
}

void RemoteViewerCore::setFbProperties(const Dimension *fbDimension,
                                       const PixelFormat *fbPixelFormat)
{
  const PixelFormat &pxFormat = *fbPixelFormat;
  StringStorage pxString;
  pxString.format(_T("[bits-per-pixel: %d, depth: %d, big-endian-flag: %d, ")
                  _T("true-color-flag: is set, ") // true color always is set
                  _T("red-max: %d, green-max: %d, blue-max: %d, ")
                  _T("red-shift: %d, green-shift: %d, blue-shift: %d]"),
                  pxFormat.bitsPerPixel, pxFormat.colorDepth, pxFormat.bigEndian,
                  pxFormat.redMax, pxFormat.greenMax, pxFormat.blueMax,
                  pxFormat.redShift, pxFormat.greenShift, pxFormat.blueShift);

  m_logWriter.info(_T("Setting frame buffer properties..."));
  m_logWriter.info(_T("Frame buffer dimension: (%d, %d)"),
                   fbDimension->width, fbDimension->height);
  m_logWriter.info(_T("Frame buffer pixel format: %s"), pxString.getString());

  if (!m_frameBuffer.setProperties(fbDimension, fbPixelFormat)) {
    StringStorage error;
    error.format(_T("Failed to set property frame buffer. ")
                 _T("Dimension: (%d, %d), Pixel format: %s"),
                 fbDimension->width, fbDimension->height,
                 pxString.getString());
    throw Exception(error.getString());
  }
  m_frameBuffer.setColor(0, 0, 0);
  refreshFrameBuffer();
  m_fbUpdateNotifier.onPropertiesFb();
  m_logWriter.detail(_T("Frame buffer properties set"));
}

StringStorage RemoteViewerCore::getProtocolString() const
{
  StringStorage protocolString;
  protocolString.format(_T("RFB %03d.%03d\n"), m_major, m_minor);
  return protocolString;
}

StringStorage RemoteViewerCore::getRemoteDesktopName() const
{
  return m_remoteDesktopName;
}

void RemoteViewerCore::execute()
{
  try {
    // connect to host and create RfbInputGate/RfbOutputGate
    // if already connected, then function do nothing
    m_logWriter.info(_T("Protocol stage is \"Connection establishing\"."));
    connectToHost();

    // get server version and set client version
    m_logWriter.info(_T("Protocol stage is \"Handshake\"."));
    handshake();

    // negotiaty about security type and authenticate
    m_logWriter.info(_T("Protocol stage is \"Authentication\"."));
    authenticate();

    // set shared flag, get server dimension, pixel format and hostname
    // send client pixel format and set him.
    m_logWriter.info(_T("Protocol stage is \"Initialization\"."));
    clientAndServerInit();

    // binding of extension
    m_extensions.bind(m_input, m_output);

    // is connected
    m_logWriter.info(_T("Protocol stage is \"Is connected\"."));
    m_adapter->onConnected();
    {
      AutoLock al(&m_connectLock);
      m_wasConnected = true;
    }

    // send supporting encoding
    m_logWriter.info(_T("Protocol stage is \"Encoding select\"."));
    sendEncodings();

    // send request of frame buffer update
    m_logWriter.info(_T("Protocol stage is \"Working phase\"."));
    sendFbUpdateRequest(false);

    // received server messages
    while (!isTerminating()) {
      UINT32 msgType = receiveServerMessageType();

      switch (msgType) {
      case ServerMsgDefs::FB_UPDATE:
        m_logWriter.info(_T("Received message: FB_UPDATE"));
        receiveFbUpdate();
        break;

      case ServerMsgDefs::SET_COLOR_MAP_ENTRIES:
        m_logWriter.info(_T("Received message: SET_COLOR_MAP_ENTRIES"));
        receiveSetColorMapEntries();
        break;

      case ServerMsgDefs::BELL:
        m_logWriter.info(_T("Received message: BELL"));
        receiveBell();
        break;

      case ServerMsgDefs::SERVER_CUT_TEXT:
        m_logWriter.info(_T("Received message: SERVER_CUT_TEXT"));
        receiveServerCutText();
        break;

      default:
        if (m_extensions.isMsgSupported(msgType)) {
          m_logWriter.info(_T("Received message: some with file transfer (%d)"), msgType);
          m_extensions.listenerMessage(m_input, msgType);
        } else {
          m_logWriter.error(_T("Server to client message: %d is not supported"), msgType);
        }
      }
    }
    StringStorage message(_T("Remote viewer's core thread terminated"));
    m_adapter->onDisconnect(&message);
  } catch (AuthException &ex) {
    m_logWriter.message(_T("RemoteVewerCore. Auth exception: %s"), ex.getMessage());
    m_adapter->onAuthError(&ex);
  } catch (Exception &ex) {
    m_logWriter.message(_T("RemoteViewerCore. Exception: %s"), ex.getMessage());
    m_adapter->onError(&ex);
  } catch (...) {
    StringStorage error;
    error.format(_T("RemoteViewerCore. Unknown exception"));
    m_logWriter.message(_T("%s"), error.getString());
    Exception ex(error.getString());
    m_adapter->onError(&ex);
  }
}

UINT32 RemoteViewerCore::receiveServerMessageType()
{
  // Viewer in common case read first byte (UINT8) as message id,
  // but if first byte is equal to 0xFC then it's TightVNC extension message and
  // must read next 3 bytes and create UINT32 message id for processing.

  static const UINT16 SERVER_MSG_SPECIAL_TIGHT_CODE = 0xFC;

  UINT32 msgType = m_input->readUInt8();
  if (msgType == SERVER_MSG_SPECIAL_TIGHT_CODE) {
    for (int i = 0; i < 3; i++) {
      msgType <<= 8;
      msgType += m_input->readUInt8();
    }
  }
  return msgType;
}

void RemoteViewerCore::receiveFbUpdate()
{
  // message type is already known: 0

  // read padding: one byte
  m_input->readUInt8();

  UINT16 numberOfRectangles = m_input->readUInt16();
  m_logWriter.info(_T("number of rectangles: %d"), numberOfRectangles);

  bool isLastRect = false;
  for (int rectangle = 0; rectangle < numberOfRectangles && !isLastRect; rectangle++) {
    m_logWriter.info(_T("Receiving rectangle #%d..."), rectangle);
    isLastRect = receiveFbUpdateRectangle();
    m_logWriter.detail(_T("Rectangle #%d is received"), rectangle);
  }

  {
    AutoLock al(&m_requestUpdateLock);
    m_isNeedRequestUpdate = true;
  }
  {
    AutoLock al(&m_freezeLock);
    if (m_isFreeze)
      return;
  }
  m_logWriter.info(_T("Sending of frame buffer update request..."));
  sendFbUpdateRequest();
}

bool RemoteViewerCore::receiveFbUpdateRectangle()
{
  Rect rect;
  rect.left = m_input->readUInt16();
  rect.top = m_input->readUInt16();
  rect.setWidth(m_input->readUInt16());
  rect.setHeight(m_input->readUInt16());

  int encodingType = m_input->readInt32();

  m_logWriter.detail(_T("Rectangle: (%d, %d), (%d, %d). Type is %d"), 
                     rect.left, rect.top, rect.right, rect.bottom, encodingType);

  if (encodingType == PseudoEncDefs::LAST_RECT)
    return true;
  m_logWriter.detail(_T("Decoder selecting..."));
  if (!Decoder::isPseudo(encodingType)) {
    if (!m_frameBuffer.getDimension().getRect().intersection(&rect).isEqualTo(&rect))
      throw Exception(_T("Error in protocol: incorrect size of rectangle"));

    Decoder *decoder = m_decoderStore.getDecoder(encodingType);
    if (decoder != 0) {
      m_logWriter.detail(_T("Decoder is selected"));

      m_logWriter.detail(_T("Decoding..."));
      if (encodingType != EncodingDefs::COPYRECT) {
        AutoLock al(&m_fbLock);
        if (!rectangleFb.isEqualTo(&m_frameBuffer)) {
          rectangleFb.assignProperties(&m_frameBuffer);
        }
      } else {  // if copy rect
        AutoLock al(&m_fbLock);
        rectangleFb.clone(&m_frameBuffer);
      }
      decoder->decode(m_input, &rectangleFb, &rect);
      {
        AutoLock al(&m_fbLock);
        m_frameBuffer.copyFrom(&rect, &rectangleFb, rect.left, rect.top);
      }
      m_fbUpdateNotifier.onUpdate(&rect);
      m_logWriter.detail(_T("Decoded"));
    } else { // decoder is 0
      StringStorage errorString;
      errorString.format(_T("unknown Encoding: %d"), encodingType);
      m_logWriter.error(_T("%s"), errorString.getString());
      throw Exception(errorString.getString());
    } 
  } else { // it's pseudo encoding
    m_logWriter.detail(_T("It's pseudo encoding"));
    processPseudoEncoding(&rect, encodingType);
  }
  return false;
}

void RemoteViewerCore::processPseudoEncoding(const Rect *rect,
                                             int encodingType)
{
  switch (encodingType) {
  case PseudoEncDefs::DESKTOP_SIZE:
    m_logWriter.info(_T("Changed size of desktop"));
    {
      AutoLock al(&m_fbLock);
      setFbProperties(&Dimension(rect), &m_frameBuffer.getPixelFormat());
    }
    break;
    
  case PseudoEncDefs::RICH_CURSOR:
    {
      m_logWriter.info(_T("New rich cursor"));

      UINT16 width = rect->getWidth();
      UINT16 height = rect->getHeight();
      UINT8 bytesPerPixel = m_frameBuffer.getBytesPerPixel();

      vector<UINT8> cursor;
      vector<UINT8> bitmask;

      size_t cursorLen = width * height * bytesPerPixel;
      if (cursorLen != 0) {
        cursor.resize(cursorLen);
        m_input->readFully(&cursor.front(), cursorLen);

        size_t bitmaskLen = ((width + 7) / 8) * height;
        bitmask.resize(bitmaskLen);
        m_input->readFully(&bitmask.front(), bitmaskLen);
      }
      Point hotSpot(rect->left, rect->top);

      m_logWriter.detail(_T("Setting new rich cursor..."));
      m_fbUpdateNotifier.setNewCursor(&hotSpot, width, height,
                                      &cursor, &bitmask);
      m_logWriter.detail(_T("New rich cursor set"));
    }
    break;

  case PseudoEncDefs::POINTER_POS:
    {
      m_logWriter.detail(_T("Updating pointer position"));
      Point position(rect->left, rect->top);
      m_fbUpdateNotifier.updatePointerPos(&position);
      m_logWriter.detail(_T("Pointer position is updated"));
    }
    break;

  default:
    StringStorage errorString;
    errorString.format(_T("Pseudo encoding %d is not supported"), encodingType);
    m_logWriter.error(_T("%s"), errorString.getString());
    throw Exception(errorString.getString());
  }
}

void RemoteViewerCore::receiveSetColorMapEntries()
{
  // message type is already known: 1

  // read padding: 1 byte
  m_input->readUInt8();

  UINT16 firstColour = m_input->readUInt16();
  UINT16 numberOfColours = m_input->readUInt16();
  for (size_t i = 0; i < numberOfColours; i++) {
    UINT16 red = m_input->readUInt16();
    UINT16 green = m_input->readUInt16();
    UINT16 blue = m_input->readUInt16();
  }
}

void RemoteViewerCore::receiveBell()
{
  // message is already readed. Message type: 2

  m_logWriter.message(_T("Bell!"));
  m_adapter->onBell();
}

void RemoteViewerCore::receiveServerCutText()
{
  // message type is already known: 3

  // read padding: three byte
  m_input->readUInt16();
  m_input->readUInt8();

  UINT32 length = m_input->readUInt32();
  std::vector<char> buffer(length + 1);
  m_input->readFully(&buffer.front(), length);
  buffer[length] = '\0';
  
  AnsiStringStorage cutTextAnsi(&buffer.front());

  StringStorage cutText;
  cutTextAnsi.toStringStorage(&cutText);

  m_logWriter.debug(_T("Cut text: %s"), cutText.getString());
  m_adapter->onCutText(&cutText);
}

bool RemoteViewerCore::isRfbProtocolString(const char protocol[12]) const
{
  // Format protocol version "RFB XXX.YYY\n"
  // where XXX is major version and YYY is minor version of protocol
  // TODO: replace "if" to regexp

  if (strncmp(protocol, "RFB ", 4) != 0)
    return false;

  if (!isdigit(protocol[4]) || !isdigit(protocol[5]) || !isdigit(protocol[6]))
    return false;

  if (protocol[7] != '.')
    return false;

  if (!isdigit(protocol[8]) || !isdigit(protocol[9]) || !isdigit(protocol[10]))
    return false;

  if (protocol[11] != '\n')
    return false;

  return true;
}

void RemoteViewerCore::handshake()
{
  char serverProtocol[13];
  serverProtocol[12] = 0;
  m_input->readFully(serverProtocol, 12);

  m_major = strtol(&serverProtocol[4], 0, 10);
  m_minor = strtol(&serverProtocol[8], 0, 10);
  m_isTight = false;

  if (!isRfbProtocolString(serverProtocol) || m_major != 3 || m_minor < 3) {
    StringStorage error;
    AnsiStringStorage protocolAnsi(serverProtocol);
    StringStorage protocol;
    protocolAnsi.toStringStorage(&protocol);
    error.format(_T("Unsupported protocol: %s"), protocol.getString());
    m_logWriter.error(_T("%s"), error.getString());
    throw Exception(error.getString());
  }

  // select minor version:
  if (m_minor < 7) {
    m_minor = 3;
  }
  if (m_minor >= 8) {
   m_minor = 8;
  }

  m_logWriter.info(_T("Setting protocol version: %s"), getProtocolString().getString());

  AnsiStringStorage clientProtocolAnsi;
  clientProtocolAnsi.fromStringStorage(&getProtocolString());
  m_output->writeFully(clientProtocolAnsi.getString(), 12);
  m_output->flush();

  m_logWriter.message(_T("Protocol version is %s"), getProtocolString().getString());
}

/**
 * Client send:
 * 2           - U8          - shared flag
 *
 * Server send:
 * 2           - U16         - framebuffer-width
 * 2           - U16         - framebuffer-height
 * 16          - PixelFormat - server-pixel-format
 * 4           - U32         - name-length
 * name-length - U8 array    - name-string
 */
void RemoteViewerCore::clientAndServerInit()
{
  if (m_sharedFlag) {
    m_logWriter.info(_T("Setting share flag in on..."));
  } else {
    m_logWriter.info(_T("Setting share flag is off..."));
  }
  m_output->writeUInt8(m_sharedFlag);
  m_output->flush();
  m_logWriter.detail(_T("Shared flag is set"));

  UINT16 width = m_input->readUInt16();
  UINT16 height = m_input->readUInt16();
  Dimension screenDimension(width, height);
  PixelFormat serverPixelFormat = readPixelFormat();
  
  {
    AutoLock al(&m_fbLock);
    setFbProperties(&screenDimension, &serverPixelFormat);
  }

  m_input->readUTF8(&m_remoteDesktopName);
  m_logWriter.message(_T("Server remote name: %s"), m_remoteDesktopName.getString());

  if (m_isTight) {
    m_logWriter.info(_T("Reading tight capabilities"));
    readCapabilities();
    m_logWriter.detail(_T("Tight capabilities is read"));
  }
}

/**
 * Struct filled from the Capabilities message
 * 2  - U16         - nServerMessageTypes
 * 2  - U16         - nClientMessageTypes
 * 2  - U16         - nEncodingTypes
 * 2  - U16         - padding. Reserved, must be 0
 * followed by nServerMessageTypes * rfbCapabilityInfo structures
 * followed by nClientMessageTypes * rfbCapabilityInfo structures
 * followed by       nEncodingType * rfbCapabilityInfo structures
 */
void RemoteViewerCore::readCapabilities()
{
  int nServerMessageTypes = m_input->readUInt16();
  int nClientMessageTypes = m_input->readUInt16();
  int nEncodingTypes = m_input->readUInt16();
  m_input->readUInt16(); //padding

  m_logWriter.info(_T("Server message types (capability):"));
  while (nServerMessageTypes--) {
    RfbCapabilityInfo cap = readCapability();
    m_extensions.enableServerMsg(&cap);
  }

  m_logWriter.info(_T("Client message types (capability):"));
  while (nClientMessageTypes--) {
    RfbCapabilityInfo cap = readCapability();
    m_extensions.enableClientMsg(&cap);
  }

  m_logWriter.info(_T("Encodings (capability):"));
  while (nEncodingTypes--) {
    RfbCapabilityInfo cap = readCapability();
    m_authCaps.enable(&cap);
  }
}

RfbCapabilityInfo RemoteViewerCore::readCapability()
{
  // read capability
  RfbCapabilityInfo cap;
  cap.code = m_input->readUInt32();
  m_input->readFully(cap.vendorSignature, RfbCapabilityInfo::vendorSigSize);
  m_input->readFully(cap.nameSignature, RfbCapabilityInfo::nameSigSize);

  // transform information for the log and log him
  char vendorSignature[RfbCapabilityInfo::vendorSigSize + 1];
  memcpy(vendorSignature, cap.vendorSignature, RfbCapabilityInfo::vendorSigSize);
  vendorSignature[RfbCapabilityInfo::vendorSigSize] = 0;
  AnsiStringStorage vendorSignatureAnsiString(vendorSignature);
  StringStorage vendorSignatureString;
  vendorSignatureAnsiString.toStringStorage(&vendorSignatureString);

  char nameSignature[RfbCapabilityInfo::nameSigSize + 1];
  memcpy(nameSignature, cap.nameSignature, RfbCapabilityInfo::nameSigSize);
  nameSignature[RfbCapabilityInfo::nameSigSize] = 0;
  AnsiStringStorage nameSignatureAnsiString(nameSignature);
  StringStorage nameSignatureString;
  nameSignatureAnsiString.toStringStorage(&nameSignatureString);

  m_logWriter.info(_T("code: %d, vendor: %s, signature: %s"),
                   cap.code, vendorSignatureString.getString(), nameSignatureString.getString());

  // return code of capability
  return cap;
}

void RemoteViewerCore::sendEncodings()
{
  {
    AutoLock al(&m_connectLock);
    if (!m_wasConnected) {
      return;
    }
  }
  RfbSetEncodingsClientMessage encodingsMessage(&m_decoderStore.getDecoderIds());
  encodingsMessage.send(m_output);
}
