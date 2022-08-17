//
// Created by uuu on 2022/8/17.
//

#ifndef SRS_NATIVE_SDK_PEER_CONNECTION_CLIENT_H
#define SRS_NATIVE_SDK_PEER_CONNECTION_CLIENT_H

#include <map>
#include <memory>
#include <string>

#include "rtc_base/net_helpers.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/third_party/sigslot/sigslot.h"

class PeerConnectionClientObserver
{
public:
    virtual ~PeerConnectionClientObserver() {}

    virtual void OnSignedIn() = 0;  // Called when we're logged on.
    virtual void OnDisconnected() = 0;
    virtual void OnPeerConnected(int id, const std::string& name) = 0;
    virtual void OnPeerDisconnected(int peer_id) = 0;
    virtual void OnMessageFromPeer(int peer_id, const std::string& message) = 0;
    virtual void OnMessageSent(int err) = 0;
    virtual void OnServerConnectionFailure() = 0;
};

class PeerConnectionClient : public sigslot::has_slots<>, public rtc::MessageHandler
{
public:
    enum State {
        NOT_CONNECTED,
        RESOLVING,
        SIGNING_IN,
        CONNECTED,
        SIGNING_OUT_WAITING,
        SIGNING_OUT,
    };

    PeerConnectionClient();
    ~PeerConnectionClient();
};


#endif //SRS_NATIVE_SDK_PEER_CONNECTION_CLIENT_H
