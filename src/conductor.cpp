//
// Created by uuu on 2022/8/17.
//

#include "conductor.h"


Conductor::Conductor(PeerConnectionClient *client)
    : client_(client)
{
}

Conductor::~Conductor()
{
}

bool Conductor::InitializePeerConnection()
{
    return true;
}

void Conductor::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>> &streams)
{

}

void Conductor::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
{

}

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface *candidate)
{

}

void Conductor::OnSignedIn()
{

}

void Conductor::OnDisconnected()
{

}

void Conductor::OnPeerConnected(int id, const std::string &name)
{

}

void Conductor::OnPeerDisconnected(int id)
{

}

void Conductor::OnMessageFromPeer(int peer_id, const std::string &message)
{

}

void Conductor::OnMessageSent(int err)
{

}

void Conductor::OnServerConnectionFailure()
{

}


