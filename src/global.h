#ifndef GLOBAL_H
#define GLOBAL_H

#include <QObject>
#include <QDebug>

const QString KEY_NAME =        "name";
const QString KEY_HOST =        "host";
const QString KEY_PORT =        "port";
const QString KEY_PORT_DST =    "portDst";
const QString KEY_HOST_DST =    "hostDst";
const QString KEY_START =       "Start";
const QString KEY_STOP =        "Stop";
const QString KEY_DEFAULT =     "default";
const QString KEY_AUTO_START =  "autoStart";
const QString KEY_USE_MULTICAST =   "useMulticast";
const QString KEY_SHOW_HEX =        "showHex";
const QString KEY_SHOW_TEXT =       "showText";
const QString KEY_ADD_NEW_LINE =    "addNewLine";
const QString KEY_SETTINGS =        "settings";
const QString KEY_CONNECTIONS =     "connections";
const QString KEY_SCRIPTS =         "scripts";
const QString KEY_TEXT =            "text";
const QString KEY_SPLITTER_H =      "splitterH";
const QString KEY_SPLITTER_V =      "splitterV";
const QString KEY_SESSIONS_DIR =    "sessionsDir";
const QString KEY_SCRIPTS_DIR =     "scriptsDir";
const QString KEY_GEOMETRY_MAIN =   "geometryMain";
const QString KEY_GEOMETRY_EDITOR = "geometryEditor";
const QString KEY_RECENT_SESSIONS = "recentSessions";
const QString KEY_BAUDRATE =        "baudrate";
const QString KEY_DATA_BITS =       "dataBits";
const QString KEY_PARITY =          "parity";
const QString KEY_STOP_BITS =       "stopBits";
const QString KEY_FLOW_CONTROL =    "flowControl";

const QString KEY_CONNECTION_TYPE = "connectionType";
const QString KEY_TCP_SERVER =      "tcpServer";
const QString KEY_TCP_SOCKET =      "tcpSocket";
const QString KEY_UDP_SOCKET =      "udpSocket";
const QString KEY_WEBSOCKETS_SERVER = "webSocketsServer";
const QString KEY_WEB_SOCKET =      "webSocket";
const QString KEY_SERIAL_PORT =     "serialPort";

const QStringList CONNECTION_TYPES = {
    KEY_TCP_SERVER, KEY_TCP_SOCKET, KEY_UDP_SOCKET,
    KEY_SERIAL_PORT, KEY_WEBSOCKETS_SERVER, KEY_WEB_SOCKET
};

enum ConnectionType
{
    ConnectionTcpServer = 0,
    ConnectionTcpSocket,
    ConnectionUdpSocket,
    ConnectionSerialPort,
    ConnectionWebSocketsServer,
    ConnectionWebSocket
};

enum StatusType
{
    StatusConsole = 0,
    StatusOk,
    StatusError,
    StatusOutput,
    StatusInput
};

#endif // GLOBAL_H
