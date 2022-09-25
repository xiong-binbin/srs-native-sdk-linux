//
// Created by uuu on 2022/8/17.
//

#include "peer_connection_client.h"

// This is our magical hangup signal.
const char kByeMessage[] = "BYE";
// Delay between server connection retries, in milliseconds
const int kReconnectDelay = 2000;

PeerConnectionClient::PeerConnectionClient()
{

}

PeerConnectionClient::~PeerConnectionClient()
{

}

void PeerConnectionClient::RegisterObserver(PeerConnectionClientObserver *callback)
{
    callback_ = callback;
}

bool PeerConnectionClient::is_connected()
{
    return id_ != -1;
}

void PeerConnectionClient::Close()
{
    control_socket_->Close();
    hanging_get_->Close();
    onconnect_data_.clear();
    peers_.clear();
    if (resolver_ != NULL) {
        resolver_->Destroy(false);
        resolver_ = NULL;
    }
    id_ = -1;
    state_ = NOT_CONNECTED;
}

bool PeerConnectionClient::ConnectControlSocket()
{
    RTC_DCHECK(control_socket_->GetState() == rtc::Socket::CS_CLOSED);
    int err = control_socket_->Connect(server_address_);
    if (err == SOCKET_ERROR) {
        Close();
        return false;
    }

    return true;
}

void PeerConnectionClient::Connect(const std::string &server, int port, const std::string &client_name)
{
    RTC_DCHECK(!server.empty());
    RTC_DCHECK(!client_name.empty());

    if(state_ != NOT_CONNECTED) {
        callback_->OnServerConnectionFailure();
        return;
    }

    if(server.empty() || client_name.empty()) {
        callback_->OnServerConnectionFailure();
        return;
    }

    if(port <= 0) {
        port = 8888;
    }

    server_address_.SetIP(server);
    server_address_.SetPort(port);
    client_name_ = client_name;

    if(server_address_.IsUnresolvedIP()) {
        state_ = RESOLVING;
        resolver_ = new rtc::AsyncResolver();
        resolver_->SignalDone.connect(this, &PeerConnectionClient::OnResolveResult);
        resolver_->Start(server_address_);
    } else {
        DoConnect();
    }
}

void PeerConnectionClient::OnResolveResult(rtc::AsyncResolverInterface *resolver)
{
    if(resolver->GetError() != 0) {
        callback_->OnServerConnectionFailure();
        resolver_->Destroy(false);
        resolver_ = nullptr;
        state_ = NOT_CONNECTED;
    } else {
        server_address_ = resolver_->address();
        DoConnect();
    }
}

void PeerConnectionClient::DoConnect()
{
    control_socket_.reset(rtc::Thread::Current()->socketserver()->CreateAsyncSocket(server_address_.ipaddr().family(), SOCK_STREAM));
    hanging_get_.reset(rtc::Thread::Current()->socketserver()->CreateAsyncSocket(server_address_.ipaddr().family(), SOCK_STREAM));

    control_socket_->SignalCloseEvent.connect(this, &PeerConnectionClient::OnClose);
    control_socket_->SignalConnectEvent.connect(this,  &PeerConnectionClient::OnConnect);
    control_socket_->SignalReadEvent.connect(this, &PeerConnectionClient::OnRead);

    hanging_get_->SignalCloseEvent.connect(this, &PeerConnectionClient::OnClose);
    hanging_get_->SignalConnectEvent.connect(this, &PeerConnectionClient::OnHangingGetConnect);
    hanging_get_->SignalReadEvent.connect(this, &PeerConnectionClient::OnHangingGetRead);

    char buffer[1024] = {0};
    snprintf(buffer, sizeof(buffer), "GET /sign_in?%s HTTP/1.0\r\n\r\n", client_name_.c_str());
    onconnect_data_ = buffer;

    bool ret = ConnectControlSocket();
    if (ret) {
        state_ = SIGNING_IN;
    } else {
        callback_->OnServerConnectionFailure();
    }
}

bool PeerConnectionClient::SignOut()
{
    return true;
}

bool PeerConnectionClient::SendHangUp(int peer_id)
{
    return true;
}

void PeerConnectionClient::OnMessage(rtc::Message *msg)
{
    // ignore msg; there is currently only one supported message ("retry")
    DoConnect();
}

void PeerConnectionClient::OnClose(rtc::AsyncSocket* socket, int err)
{
    printf("OnClose \n");
    socket->Close();

    if (err != ECONNREFUSED) {
        if (socket == hanging_get_.get()) {
            if (state_ == CONNECTED) {
                hanging_get_->Close();
                hanging_get_->Connect(server_address_);
            }
        } else {
            callback_->OnMessageSent(err);
        }
    } else {
        if (socket == control_socket_.get()) {
            printf("Connection refused; retrying in 2 seconds \n");
            rtc::Thread::Current()->PostDelayed(RTC_FROM_HERE, kReconnectDelay, this, 0);
        } else {
            Close();
            callback_->OnDisconnected();
        }
    }
}

void PeerConnectionClient::OnConnect(rtc::AsyncSocket* socket)
{
    RTC_DCHECK(!onconnect_data_.empty());
    size_t sent = socket->Send(onconnect_data_.c_str(), onconnect_data_.length());
    RTC_DCHECK(sent == onconnect_data_.length());
    onconnect_data_.clear();
}

void PeerConnectionClient::OnRead(rtc::AsyncSocket* socket)
{
    size_t content_length = 0;
    if (ReadIntoBuffer(socket, &control_data_, &content_length)) {
        size_t peer_id = 0, eoh = 0;
        bool ok = ParseServerResponse(control_data_, content_length, &peer_id, &eoh);
        if (ok) {
            if (id_ == -1) {
                // First response.  Let's store our server assigned ID.
                RTC_DCHECK(state_ == SIGNING_IN);
                id_ = static_cast<int>(peer_id);
                RTC_DCHECK(id_ != -1);

                // The body of the response will be a list of already connected peers.
                if (content_length) {
                    size_t pos = eoh + 4;
                    while (pos < control_data_.size()) {
                        size_t eol = control_data_.find('\n', pos);
                        if (eol == std::string::npos)
                            break;
                        int id = 0;
                        std::string name;
                        bool connected;
                        if (ParseEntry(control_data_.substr(pos, eol - pos), &name, &id, &connected) && id != id_) {
                            peers_[id] = name;
                            callback_->OnPeerConnected(id, name);
                        }
                        pos = eol + 1;
                    }
                }
                RTC_DCHECK(is_connected());
                callback_->OnSignedIn();
            } else if (state_ == SIGNING_OUT) {
                Close();
                callback_->OnDisconnected();
            } else if (state_ == SIGNING_OUT_WAITING) {
                SignOut();
            }
        }

        control_data_.clear();

        if (state_ == SIGNING_IN) {
            RTC_DCHECK(hanging_get_->GetState() == rtc::Socket::CS_CLOSED);
            state_ = CONNECTED;
            hanging_get_->Connect(server_address_);
        }
    }
}

void PeerConnectionClient::OnHangingGetConnect(rtc::AsyncSocket* socket)
{
    char buffer[1024] = {0};
    snprintf(buffer, sizeof(buffer), "GET /wait?peer_id=%i HTTP/1.0\r\n\r\n",  id_);

    int len = static_cast<int>(strlen(buffer));
    int sent = socket->Send(buffer, len);
    RTC_DCHECK(sent == len);
}

void PeerConnectionClient::OnHangingGetRead(rtc::AsyncSocket* socket)
{
    size_t content_length = 0;
    if (ReadIntoBuffer(socket, &notification_data_, &content_length)) {
        size_t peer_id = 0, eoh = 0;
        bool ok = ParseServerResponse(notification_data_, content_length, &peer_id, &eoh);

        if (ok) {
            // Store the position where the body begins.
            size_t pos = eoh + 4;

            if (id_ == static_cast<int>(peer_id)) {
                // A notification about a new member or a member that just
                // disconnected.
                int id = 0;
                std::string name;
                bool connected = false;
                if (ParseEntry(notification_data_.substr(pos), &name, &id, &connected)) {
                    if (connected) {
                        peers_[id] = name;
                        callback_->OnPeerConnected(id, name);
                    } else {
                        peers_.erase(id);
                        callback_->OnPeerDisconnected(id);
                    }
                }
            } else {
                OnMessageFromPeer(static_cast<int>(peer_id), notification_data_.substr(pos));
            }
        }

        notification_data_.clear();
    }

    if (hanging_get_->GetState() == rtc::Socket::CS_CLOSED && state_ == CONNECTED) {
        hanging_get_->Connect(server_address_);
    }
}

void PeerConnectionClient::OnMessageFromPeer(int peer_id, const std::string &message)
{
    if (message.length() == (sizeof(kByeMessage) - 1) &&
        message.compare(kByeMessage) == 0) {
        callback_->OnPeerDisconnected(peer_id);
    } else {
        callback_->OnMessageFromPeer(peer_id, message);
    }
}

bool PeerConnectionClient::ReadIntoBuffer(rtc::AsyncSocket *socket, std::string *data, size_t *content_length)
{
    char buffer[65535] = {0};
    do {
        int bytes = socket->Recv(buffer, sizeof(buffer), nullptr);
        if(bytes <= 0) {
            break;
        }
        data->append(buffer, bytes);
    } while (true);

    bool ret = false;
    size_t i = data->find("\r\n\r\n");
    if(i != std::string::npos) {
        printf("Headers received \n");
        if(GetHeaderValue(*data, i, "\r\nContent-Length: ", content_length)) {
            size_t total_response_size = (i + 4) + *content_length;
            if (data->length() >= total_response_size) {
                ret = true;
                std::string should_close;
                const char kConnection[] = "\r\nConnection: ";
                if (GetHeaderValue(*data, i, kConnection, &should_close) &&
                    should_close.compare("close") == 0) {
                    socket->Close();
                    // Since we closed the socket, there was no notification delivered
                    // to us.  Compensate by letting ourselves know.
                    OnClose(socket, 0);
                }
            } else {
                // We haven't received everything.  Just continue to accept data.
            }
        } else {
            printf("No content length field specified by the server \n");
        }
    }

    return ret;
}

bool PeerConnectionClient::GetHeaderValue(const std::string &data, size_t eoh, const char *header_pattern, size_t *value)
{
    RTC_DCHECK(value != NULL);
    size_t found = data.find(header_pattern);
    if (found != std::string::npos && found < eoh) {
        *value = atoi(&data[found + strlen(header_pattern)]);
        return true;
    }
    return false;
}

bool PeerConnectionClient::GetHeaderValue(const std::string &data, size_t eoh, const char *header_pattern, std::string *value)
{
    RTC_DCHECK(value != NULL);
    size_t found = data.find(header_pattern);
    if (found != std::string::npos && found < eoh) {
        size_t begin = found + strlen(header_pattern);
        size_t end = data.find("\r\n", begin);
        if (end == std::string::npos)
            end = eoh;
        value->assign(data.substr(begin, end - begin));
        return true;
    }
    return false;
}

int PeerConnectionClient::GetResponseStatus(const std::string &response)
{
    int status = -1;
    size_t pos = response.find(' ');
    if (pos != std::string::npos)
        status = atoi(&response[pos + 1]);
    return status;
}

bool PeerConnectionClient::ParseServerResponse(const std::string &response, size_t content_length, size_t *peer_id, size_t *eoh)
{
    int status = GetResponseStatus(response.c_str());
    if (status != 200) {
        printf("Received error from server \n");
        Close();
        callback_->OnDisconnected();
        return false;
    }

    *eoh = response.find("\r\n\r\n");
    RTC_DCHECK(*eoh != std::string::npos);
    if (*eoh == std::string::npos)
        return false;

    *peer_id = -1;

    // See comment in peer_channel.cc for why we use the Pragma header.
    GetHeaderValue(response, *eoh, "\r\nPragma: ", peer_id);

    return true;
}

bool PeerConnectionClient::ParseEntry(const std::string &entry, std::string *name, int *id, bool *connected)
{
    RTC_DCHECK(name != NULL);
    RTC_DCHECK(id != NULL);
    RTC_DCHECK(connected != NULL);
    RTC_DCHECK(!entry.empty());

    *connected = false;
    size_t separator = entry.find(',');
    if (separator != std::string::npos) {
        *id = atoi(&entry[separator + 1]);
        name->assign(entry.substr(0, separator));
        separator = entry.find(',', separator + 1);
        if (separator != std::string::npos) {
            *connected = atoi(&entry[separator + 1]) ? true : false;
        }
    }
    return !name->empty();
}

