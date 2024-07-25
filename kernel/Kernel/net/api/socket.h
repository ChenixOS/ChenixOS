#pragma once

#include "fs/proxyfs/ProxyFs.h"

#define SOCKET_LOCAL 1
#define SOCKET_TCP 2
#define SOCKET_TCP_SERVER 3
#define SOCKET_UDP 4

class Socket;
class SocketIoObject;

struct SocketObject {
    unsigned int type;
    Socket* socket;
    SocketIoObject* ioObj;
};

class SocketIoObject: public ProxyFs::IoObject {
public:
    SocketIoObject(SocketObject* socket);
    virtual uint64 Read(uint64 pos, void* buffer, uint64 bufferSize) override;
    virtual uint64 Write(uint64 pos, const void* buffer, uint64 bufferSize) override;
    virtual void Clear() override;
    virtual void Close() override;
private:
    SocketObject* socket;
};

class Socket {
public:
    Socket();
};
