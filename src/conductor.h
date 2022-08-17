//
// Created by uuu on 2022/8/17.
//

#ifndef SRS_NATIVE_SDK_CONDUCTOR_H
#define SRS_NATIVE_SDK_CONDUCTOR_H

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "peer_connection_client.h"

class Conductor : public webrtc::PeerConnectionObserver,
                  public webrtc::CreateSessionDescriptionObserver,
                  public PeerConnectionClientObserver
{
public:
    enum CallbackID {
        MEDIA_CHANNELS_INITIALIZED = 1,
        PEER_CONNECTION_CLOSED,
        SEND_MESSAGE_TO_PEER,
        NEW_TRACK_ADDED,
        TRACK_REMOVED,
    };

    Conductor(PeerConnectionClient* client);
    virtual ~Conductor();

protected:
    bool InitializePeerConnection();

    bool CreatePeerConnection(bool dtls);

    void DeletePeerConnection();

    void AddTracks();

    //
    // PeerConnectionObserver implementation.
    //

    void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override {}

    void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver, const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override;

    void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;

    void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}

    void OnRenegotiationNeeded() override {}

    void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override {}

    void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}

    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

    void OnIceConnectionReceivingChange(bool receiving) override {}

    //
    // PeerConnectionClientObserver implementation.
    //

    void OnSignedIn() override;

    void OnDisconnected() override;

    void OnPeerConnected(int id, const std::string& name) override;

    void OnPeerDisconnected(int id) override;

    void OnMessageFromPeer(int peer_id, const std::string& message) override;

    void OnMessageSent(int err) override;

    void OnServerConnectionFailure() override;

private:
    PeerConnectionClient* client_ = nullptr;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
};


#endif //SRS_NATIVE_SDK_CONDUCTOR_H
