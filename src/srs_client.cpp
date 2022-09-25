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

int SrsClient::start(int argc, char* argv[])
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

    return 0;
}

void SrsClient::stop()
{
}
