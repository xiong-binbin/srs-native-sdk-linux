//
// Created by uuu on 2022/7/30.
//

#include "srs_client.h"
#include "absl/flags/parse.h"
#include "rtc_base/ssl_adapter.h"
#include "system_wrappers/include/field_trial.h"

SrsClient::SrsClient()
{
}

SrsClient::~SrsClient()
{
}

int SrsClient::Start()
{
    webrtc::field_trial::InitFieldTrialsFromString("");

    CustomSocketServer socketServer;
    rtc::AutoSocketServerThread thread(&socketServer);

    rtc::InitializeSSL();

    // Must be constructed after we set the socketserver.
    PeerConnectionClient client;
    rtc::scoped_refptr<Conductor> conductor(new rtc::RefCountedObject<Conductor>(&client));

    socketServer.set_client(&client);
    socketServer.set_conductor(conductor);

    thread.Run();

    conductor->StartLogin("https://apprtc.webrtcserver.cn/", 443);

    return 0;
}

void SrsClient::Stop()
{
}
