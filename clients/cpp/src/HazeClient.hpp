#pragma once

#include <string>
namespace Haze {
  class HazeClient {
    private:
      std::string host;
      unsigned short port;
    public:
      bool Connect(const std::string& host, unsigned short port);
      void Disconnect();
      bool Ping();
      const std::string& Host();
      unsigned short Port();
  };
}
