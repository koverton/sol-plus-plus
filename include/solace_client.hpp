#pragma once
#include <string>
#include <functional>
#include <chrono>

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

struct Envelope 
{
    const std::string topic;
    const std::string msgid;
    const std::chrono::time_point<std::chrono::system_clock> sndtime;
    const std::chrono::time_point<std::chrono::system_clock> rcvtime;
};

class SolClient
{
public:
    enum State 
    {
        DISCONNECTED,
        CONNECTED
    };

    using RawReaderCb = std::function<void(const Envelope&, const void*, const std::uint32_t)>;
    using StateChangeCb = std::function<void(const State)>;

    SolClient();
    SolClient(const SolConfig cfg);
    ~SolClient();

    void connect(
        StateChangeCb stateCallback,
        RawReaderCb msgCallback);

    void disconnect();

    bool isConnected() const {
        return _state == CONNECTED;
    }
    void setState(State state);

    bool subscribe(
        const std::string& topic,
        bool blocking = true);

    void raiseMsg(
        const Envelope& env,
        const void* data,
        const std::uint32_t length);

private:

    SolConfig _cfg;
    void* _ctx;
    void* _sess;
    State _state;
    RawReaderCb _msgCb;
    StateChangeCb _stateCb;
};

}}
