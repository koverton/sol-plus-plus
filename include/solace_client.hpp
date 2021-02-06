#pragma once
#include "solconfig.hpp"
#include <string>
#include <functional>
#include <chrono>

namespace kov {
namespace solace {


class SolClient
{
public:
    enum State 
    {
        DISCONNECTED,
        CONNECTED
    };
    
    struct MsgInfo 
    {
        const std::string topic;
        const std::string msgid;
        const std::chrono::time_point<std::chrono::system_clock> sndtime;
        const std::chrono::time_point<std::chrono::system_clock> rcvtime;
    };

    using RawReaderCb = std::function<void(const MsgInfo&, const void*, const std::uint32_t)>;
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

    bool publish(
        const std::string& topic,
        const void* data,
        const std::uint32_t length);

    bool subscribe(
        const std::string& topic,
        bool blocking = true);

    void raiseMsg(
        const MsgInfo& env,
        const void* data,
        const std::uint32_t length);

private:

    SolConfig _cfg;
    void* _ctx;
    void* _sess;
    State _state;
    void* _outmsg;

    RawReaderCb _msgCb;
    StateChangeCb _stateCb;
};

}}
