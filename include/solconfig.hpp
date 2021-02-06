#pragma once
#include <string>

namespace kov {
namespace solace {

struct SolConfig
{
    SolConfig() = default;
    SolConfig(const SolConfig&) = default;
    ~SolConfig() = default;

    std::string _host{"localhost"};
    uint32_t    _port{55555};
    std::string _vpn{"default"};
    std::string _username{"default"};
    std::string _password{"default"};
    bool        _tls{false};

    const char** mkprops();
    const char* sprops[100];
    std::string _portstr;
};

}}