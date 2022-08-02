//
// Created by uuu on 2022/7/30.
//

#include "srs_client.h"
#include "webrtc/rtc_base/ssl_adapter.h"
#include "webrtc/system_wrappers/include/field_trial.h"

SrsClient::SrsClient()
{
}

SrsClient::~SrsClient()
{
}

int SrsClient::Initialize()
{
    webrtc::field_trial::InitFieldTrialsFromString("");

    CustomSocketServer socketServer;
    rtc::AutoSocketServerThread thread(&socketServer);

    rtc::InitializeSSL();
}
