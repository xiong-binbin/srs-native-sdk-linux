//
// Created by uuu on 2022/8/17.
//

#include "conductor.h"
#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/types/optional.h"
#include "api/audio/audio_mixer.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/audio_options.h"
#include "api/create_peerconnection_factory.h"
#include "api/rtp_sender_interface.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
#include "p2p/base/port_allocator.h"
#include "pc/video_track_source.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/ref_counted_object.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "pc/test/fake_audio_capture_module.h"
#include "pc/test/fake_periodic_video_track_source.h"
#include "json.hpp"


Conductor::Conductor(PeerConnectionClient *client)
    : client_(client)
{
}

Conductor::~Conductor()
{
}

bool Conductor::InitializePeerConnection()
{
    RTC_DCHECK(!peer_connection_factory_);
    RTC_DCHECK(!peer_connection_);

    peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(nullptr, nullptr, nullptr, nullptr,
                                                                   webrtc::CreateBuiltinAudioEncoderFactory(),
                                                                   webrtc::CreateBuiltinAudioDecoderFactory(),
                                                                   webrtc::CreateBuiltinVideoEncoderFactory(),
                                                                   webrtc::CreateBuiltinVideoDecoderFactory(),
                                                                   nullptr, nullptr);
    if(!peer_connection_factory_) {
        printf("InitializePeerConnection Error \n");
        DeletePeerConnection();
        return false;
    }

    if(!CreatePeerConnection(true)) {
        printf("InitializePeerConnection Error \n");
        DeletePeerConnection();
        return false;
    }

    AddTracks();

    return peer_connection_ != nullptr;
}

bool Conductor::CreatePeerConnection(bool dtls)
{
    RTC_DCHECK(peer_connection_factory_);
    RTC_DCHECK(!peer_connection_);

    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = "stun:stun.l.google.com:19302";

    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = dtls;
    config.servers.push_back(server);

    peer_connection_ = peer_connection_factory_->CreatePeerConnection(config, nullptr, nullptr, this);

    return peer_connection_ != nullptr;
}

void Conductor::DeletePeerConnection()
{
    peer_connection_ = nullptr;
    peer_connection_factory_ = nullptr;
}

void Conductor::AddTracks()
{
    if(!peer_connection_->GetSenders().empty()) {
        return;
    }

    cricket::AudioOptions audioOptions;
    audioOptions.highpass_filter = false;
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(peer_connection_factory_->CreateAudioTrack("audio_label", peer_connection_factory_->CreateAudioSource(audioOptions)));
    auto res = peer_connection_->AddTrack(audio_track, {"stream_id"});
    if(!res.ok()) {
        printf("AddTracks Error \n");
    }

//    auto* videoTrackSource = new rtc::RefCountedObject<webrtc::FakePeriodicVideoTrackSource>(false /* remote */);


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


