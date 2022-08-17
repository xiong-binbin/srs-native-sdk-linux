//
// Created by uuu on 2022/7/30.
//

#ifndef SRS_NATIVE_SDK_LINUX_SRS_CLIENT_H
#define SRS_NATIVE_SDK_LINUX_SRS_CLIENT_H

#include <iostream>
#include "rtc_base/thread.h"
#include "rtc_base/physical_socket_server.h"


class CustomSocketServer : public rtc::PhysicalSocketServer
{
public:
    explicit CustomSocketServer() {}
    virtual ~CustomSocketServer() {}

    virtual void SetMessageQueue(rtc::Thread* queue) override { msgQueue = queue; }

    virtual bool Wait(int cms, bool process_io) override
    {
        return rtc::PhysicalSocketServer::Wait(0, process_io);
    }

private:
    rtc::Thread* msgQueue;
};


class SrsClient
{
public:
    SrsClient();
    ~SrsClient();

    int Initialize();

private:
};


#endif //SRS_NATIVE_SDK_LINUX_SRS_CLIENT_H
