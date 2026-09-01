#include "HazeClient.hpp"

namespace Haze {

bool HazeClient::Connect(const std::string& host, unsigned short port)
{
    this->host = host;
    this->port = port;

    return true;
}

void HazeClient::Disconnect()
{
    host.clear();
    port = 0;
}

const std::string& HazeClient::Host()
{
    return host;
}

unsigned short HazeClient::Port()
{
    return port;
}

}
