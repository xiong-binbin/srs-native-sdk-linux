//
// Created by uuu on 2022/8/17.
//

#include "peer_connection_client.h"

PeerConnectionClient::PeerConnectionClient()
{

}

PeerConnectionClient::~PeerConnectionClient()
{

}

bool PeerConnectionClient::is_connected()
{
    return id_ != -1;
}

void PeerConnectionClient::Connect(const std::string &server, int port, const std::string &client_name)
{

}

bool PeerConnectionClient::SignOut()
{
    return true;
}

bool PeerConnectionClient::SendHangUp(int peer_id)
{
    return true;
}