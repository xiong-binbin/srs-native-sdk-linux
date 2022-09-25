//
// Created by uuu on 2022/7/30.
//

#ifndef SRS_NATIVE_SDK_LINUX_SRS_CLIENT_H
#define SRS_NATIVE_SDK_LINUX_SRS_CLIENT_H

#include <iostream>
#include "rtc_base/thread.h"
#include "conductor.h"
#include "peer_connection_client.h"
#include "rtc_base/physical_socket_server.h"


class CustomSocketServer : public rtc::PhysicalSocketServer
{
public:
    explicit CustomSocketServer() {}
    virtual ~CustomSocketServer() {}

    virtual void SetMessageQueue(rtc::Thread* queue) override { msgQueue = queue; }

    void set_client(PeerConnectionClient* client) { client_ = client; }
    void set_conductor(Conductor* conductor) { conductor_ = conductor; }

    virtual bool Wait(int cms, bool process_io) override
    {
        if (!conductor_->connection_active() && client_ != NULL && !client_->is_connected()) {
            msgQueue->Quit();
        }
        return rtc::PhysicalSocketServer::Wait(0, process_io);
    }

private:
    rtc::Thread* msgQueue;
    Conductor*   conductor_;
    PeerConnectionClient* client_;
};


class SrsClient
{
public:
    SrsClient();
    ~SrsClient();

    int  start(int argc, char* argv[]);
    void stop();

private:
};


#endif //SRS_NATIVE_SDK_LINUX_SRS_CLIENT_H
