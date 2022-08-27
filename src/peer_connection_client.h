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

    void RegisterObserver(PeerConnectionClientObserver* callback);

    bool is_connected();

    void Connect(const std::string& server, int port, const std::string& client_name);

    bool SignOut();

    bool SendHangUp(int peer_id);

protected:
    void Close();
    bool ConnectControlSocket();
    void DoConnect();
    void OnResolveResult(rtc::AsyncResolverInterface* resolver);

    void OnClose(rtc::AsyncSocket* socket, int err);
    void OnConnect(rtc::AsyncSocket* socket);
    void OnRead(rtc::AsyncSocket* socket);

    void OnHangingGetConnect(rtc::AsyncSocket* socket);
    void OnHangingGetRead(rtc::AsyncSocket* socket);

    void OnMessageFromPeer(int peer_id, const std::string& message);

    bool ReadIntoBuffer(rtc::AsyncSocket* socket, std::string* data, size_t* content_length);

    // Quick and dirty support for parsing HTTP header values.
    bool GetHeaderValue(const std::string& data, size_t eoh, const char* header_pattern, size_t* value);
    bool GetHeaderValue(const std::string& data, size_t eoh, const char* header_pattern, std::string* value);

    int  GetResponseStatus(const std::string& response);
    bool ParseServerResponse(const std::string& response, size_t content_length, size_t* peer_id, size_t* eoh);

    // Parses a single line entry in the form "<name>,<id>,<connected>"
    bool ParseEntry(const std::string& entry, std::string* name, int* id, bool* connected);

private:
    int id_ = -1;
    State state_ = NOT_CONNECTED;
    std::string client_name_;
    std::string onconnect_data_;
    std::string control_data_;
    std::string notification_data_;
    std::map<int, std::string> peers_;
    PeerConnectionClientObserver* callback_ = nullptr;

    rtc::SocketAddress server_address_;
    rtc::AsyncResolver* resolver_;
    std::unique_ptr<rtc::AsyncSocket> control_socket_;
    std::unique_ptr<rtc::AsyncSocket> hanging_get_;
};


#endif //SRS_NATIVE_SDK_PEER_CONNECTION_CLIENT_H
