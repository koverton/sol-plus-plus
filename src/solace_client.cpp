#include "solace_client.hpp"

#include <solclient/solClient.h>
#include <solclient/solClientMsg.h>

#include <iostream>
#include <stdexcept>

namespace 
{
/*****************************************
 * Solace C-lib internals
 *****************************************/

void
on_error ( 
    solClient_returnCode_t rc, 
    const char *errorStr )
{
    auto errorInfo = solClient_getLastErrorInfo();
    solClient_log( SOLCLIENT_LOG_ERROR,
        "%s - ReturnCode=\"%s\", SubCode=\"%s\", ResponseCode=%d, Info=\"%s\"",
        errorStr, 
        solClient_returnCodeToString(rc),
        solClient_subCodeToString(errorInfo->subCode), 
        errorInfo->responseCode, 
        errorInfo->errorStr );
    solClient_resetLastErrorInfo();
}

solClient_rxMsgCallback_returnCode_t
on_msg (
    solClient_opaqueSession_pt sess_p, 
    solClient_opaqueMsg_pt msg_p, 
    void *user_p )
{
    std::cout << "ON_MSG" <<std::endl;
    auto client = static_cast<kov::solace::SolClient*>(user_p);
    if (!client)
    {
        on_error(SOLCLIENT_FAIL, "Missing or unrecognized user-pointer on Message event.");
        return SOLCLIENT_CALLBACK_OK;
    }
    // topic
    solClient_destination_t dest;
    auto rc = solClient_msg_getDestination(
        msg_p,
        &dest,
        sizeof(dest));
    if (SOLCLIENT_OK != rc)
    {
        on_error( rc, "ERROR extracting Destination from msg" );
    }
    // msgid
    std::string msgid{""};
    const char* _id{nullptr};
    rc = solClient_msg_getApplicationMessageId(
        msg_p,
        &_id);
    if (SOLCLIENT_OK != rc)
    {
        on_error( rc, "ERROR extracting ID from msg" );
    }
    if (_id) msgid.append(_id);
    // rcv-time
    std::int64_t rcvtime{0};
    rc = solClient_msg_getRcvTimestamp(
        msg_p,
        &rcvtime);
    if (SOLCLIENT_OK != rc)
    {
        on_error( rc, "ERROR extracting ID from msg" );
    }

    // snd-time
    std::int64_t sndtime{0};
    rc = solClient_msg_getSenderTimestamp(
        msg_p,
        &sndtime);
    if (SOLCLIENT_OK != rc)
    {
        on_error( rc, "ERROR extracting ID from msg" );
    }

    // payload
    std::uint32_t len{0};
    void*  data;
    rc = solClient_msg_getBinaryAttachmentPtr(
        msg_p,
        &data,
        &len);
    if (SOLCLIENT_OK != rc)
    {
        on_error( rc, "ERROR extracting payload from msg" );
        return SOLCLIENT_CALLBACK_OK;
    }
    const kov::solace::SolClient::MsgInfo env{
        dest.dest,
        msgid,
        std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::milliseconds(sndtime)),
        std::chrono::time_point<std::chrono::system_clock>(
            std::chrono::milliseconds(rcvtime))
    };
    client->raiseMsg( env, data, len );
    return SOLCLIENT_CALLBACK_OK;
}

void
on_event (
    solClient_opaqueSession_pt sess_p,
    solClient_session_eventCallbackInfo_pt info_p, 
    void *user_p)
{
    const char* evtstr = solClient_session_eventToString(info_p->sessionEvent);
    std::cout << "EVENT: " << evtstr << std::endl;
    auto client = static_cast<kov::solace::SolClient*>(user_p);
    if (!client)
    {
        on_error(SOLCLIENT_FAIL, "Missing or unrecognized user-pointer on Event message.");
        return;
    }
    switch ( info_p->sessionEvent ) {
        case SOLCLIENT_SESSION_EVENT_UP_NOTICE:
        case SOLCLIENT_SESSION_EVENT_RECONNECTED_NOTICE:
            client->setState( kov::solace::SolClient::State::CONNECTED );
            break;
        case SOLCLIENT_SESSION_EVENT_ACKNOWLEDGEMENT:
        case SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_OK:
        case SOLCLIENT_SESSION_EVENT_CAN_SEND:
        case SOLCLIENT_SESSION_EVENT_PROVISION_OK:
        case SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_OK:
        case SOLCLIENT_SESSION_EVENT_ASSURED_PUBLISHING_UP:
        case SOLCLIENT_SESSION_EVENT_VIRTUAL_ROUTER_NAME_CHANGED:
        case SOLCLIENT_SESSION_EVENT_MODIFYPROP_OK:
        case SOLCLIENT_SESSION_EVENT_REPUBLISH_UNACKED_MESSAGES:
            break;

        case SOLCLIENT_SESSION_EVENT_CONNECT_FAILED_ERROR:
        case SOLCLIENT_SESSION_EVENT_RECONNECTING_NOTICE:
        case SOLCLIENT_SESSION_EVENT_DOWN_ERROR:
            client->setState( kov::solace::SolClient::State::DISCONNECTED );
            break;

        case SOLCLIENT_SESSION_EVENT_REJECTED_MSG_ERROR:
        case SOLCLIENT_SESSION_EVENT_SUBSCRIPTION_ERROR:
        case SOLCLIENT_SESSION_EVENT_RX_MSG_TOO_BIG_ERROR:
        case SOLCLIENT_SESSION_EVENT_TE_UNSUBSCRIBE_ERROR:
        case SOLCLIENT_SESSION_EVENT_PROVISION_ERROR:
        case SOLCLIENT_SESSION_EVENT_ASSURED_CONNECT_FAILED:
        case SOLCLIENT_SESSION_EVENT_MODIFYPROP_FAIL:
        {
            on_error( SOLCLIENT_OK, evtstr );
            break;
        }
        default:
            // Unrecognized or deprecated event
            std::cout << "EVENT: WARNING unrecognized Solace event: "
                      << evtstr << std::endl;
            break;
    }
}

void* 
mkctx() 
{
    /* SolClient needs to be initialized before any other API calls. */
    auto rc = solClient_initialize( 
        SOLCLIENT_LOG_DEFAULT_FILTER, 
        nullptr );
    if (rc != SOLCLIENT_OK) {
        on_error( rc, "FAIL: could not initialize solace library" );
        throw std::runtime_error("failed to init Solace library");
    }
    solClient_opaqueContext_pt ctx;
    solClient_context_createFuncInfo_t cfinfo = 
        SOLCLIENT_CONTEXT_CREATEFUNC_INITIALIZER;
    rc = solClient_context_create ( 
        SOLCLIENT_CONTEXT_PROPS_DEFAULT_WITH_CREATE_THREAD,
        &ctx, 
        &cfinfo, 
        sizeof(cfinfo) );

    if (rc != SOLCLIENT_OK) {
        on_error( rc, "FAIL: could not create solace context thread" );
        throw std::runtime_error("failed to create Solace context");
    }
    return std::move(ctx);
}

void*
mksess(
    void* ctx, 
    const char** sprops, 
    kov::solace::SolClient* user_p)
{
    solClient_opaqueSession_pt sess;
    solClient_session_createFuncInfo_t sfinfo = 
        SOLCLIENT_SESSION_CREATEFUNC_INITIALIZER;
    sfinfo.rxMsgInfo.callback_p = on_msg;
    sfinfo.rxMsgInfo.user_p = user_p;
    sfinfo.eventInfo.callback_p = on_event;
    sfinfo.eventInfo.user_p = user_p;

    auto rc = solClient_session_create ( 
        sprops,
        static_cast<solClient_opaqueContext_pt>(ctx),
        &sess, 
        &sfinfo, 
        sizeof(sfinfo) );

    if (rc != SOLCLIENT_OK) {
        on_error( rc, "FAIL: could not create solace session" );
        throw std::runtime_error("failed to create Solace session");
    }
    return std::move(sess);
}
void*
mkmsg()
{
    solClient_opaqueMsg_pt msg_p;
    solClient_msg_alloc( &msg_p );
    solClient_msg_setDeliveryMode( msg_p, 
    SOLCLIENT_DELIVERY_MODE_DIRECT );
    return std::move(msg_p);
}
}

namespace kov {
namespace solace {

/*****************************************
 * SolClient
 *****************************************/

SolClient::SolClient()
: _ctx(mkctx())
, _sess(mksess(_ctx, _cfg.mkprops(), this))
, _outmsg(mkmsg())
, _msgCb([](const MsgInfo&,const void*,const std::uint32_t){})
, _stateCb([](const State){})
{
}
SolClient::SolClient(const SolConfig cfg)
: _cfg(cfg)
, _ctx(mkctx())
, _sess(mksess(_ctx, _cfg.mkprops(), this))
, _outmsg(mkmsg())
, _msgCb([](const MsgInfo&,const void*,const std::uint32_t){})
, _stateCb([](const State){})
{
}

SolClient::~SolClient()
{
    solClient_msg_free ( &_outmsg );
    solClient_cleanup();
}

void 
SolClient::connect(
    StateChangeCb stateCallback,
    RawReaderCb msgCallback)
{
    _stateCb = stateCallback;
    _msgCb = msgCallback;
    auto rc = solClient_session_connect( _sess );
    if (SOLCLIENT_OK != rc)
    {
        on_error( rc, "FAIL: could not connect solace session" );
        throw std::runtime_error("failed to create Solace session");
    }
}

void 
SolClient::disconnect()
{
    auto rc = solClient_session_disconnect( _sess );
    if (SOLCLIENT_OK != rc)
    {
        on_error( rc, "FAIL: could not disconnect solace session" );
        throw std::runtime_error("failed to create Solace session");
    }
}

bool 
SolClient::subscribe(
    const std::string& topic,
    bool blocking)
{
    auto flag = (blocking ? SOLCLIENT_SUBSCRIBE_FLAGS_WAITFORCONFIRM : 0);
    return SOLCLIENT_OK == 
            solClient_session_topicSubscribeExt ( 
                _sess,
                flag,
                topic.c_str());
}

void SolClient::setState(State state)
{
    _state = state;
    _stateCb( state );
}

bool SolClient::publish(
    const std::string& topic,
    const void* data,
    const std::uint32_t length)
{
    solClient_destination_t destination;
    destination.destType = SOLCLIENT_TOPIC_DESTINATION;
    destination.dest = topic.c_str();
    solClient_msg_setDestination( _outmsg, 
        &destination, sizeof(destination) );
    solClient_msg_setBinaryAttachmentPtr( _outmsg, const_cast<void*>(data), length );

    std::cout << "Calling solClient_session_sendMsg" << std::endl;
    auto rc = solClient_session_sendMsg ( _sess, _outmsg );
    if (SOLCLIENT_OK != rc)
    {
        on_error( rc, "FAIL: could not send solace message" );
        throw std::runtime_error("failed to send solace message");
    }
    std::cout << "success solClient_session_sendMsg" << std::endl;
    return rc == SOLCLIENT_OK;
}


void SolClient::raiseMsg(
    const MsgInfo& env,
    const void* data,
    const std::uint32_t length)
{
    std::cout << "Raised" << std::endl;
    _msgCb( env, data, length );
}

/*****************************************
 * SolConfig
 *****************************************/
const char** 
SolConfig::mkprops()
{
    int i{0};
    sprops[i++] = SOLCLIENT_SESSION_PROP_HOST;
    sprops[i++] = _host.c_str();

    _portstr = std::to_string( _port );
    sprops[i++] = SOLCLIENT_SESSION_PROP_PORT;
    sprops[i++] = _portstr.c_str();

    sprops[i++] = SOLCLIENT_SESSION_PROP_VPN_NAME;
    sprops[i++] = _vpn.c_str();

    sprops[i++] = SOLCLIENT_SESSION_PROP_USERNAME;
    sprops[i++] = _username.c_str();

    sprops[i++] = SOLCLIENT_SESSION_PROP_PASSWORD;
    sprops[i++] = _username.c_str();

    sprops[i++] = SOLCLIENT_SESSION_PROP_CONNECT_RETRIES;
    sprops[i++] = "-1";

    sprops[i++] = SOLCLIENT_SESSION_PROP_RECONNECT_RETRIES;
    sprops[i++] = "-1";

    sprops[i++] = SOLCLIENT_SESSION_PROP_GENERATE_RCV_TIMESTAMPS;
    sprops[i++] = SOLCLIENT_PROP_ENABLE_VAL;

    sprops[i++] = SOLCLIENT_SESSION_PROP_GENERATE_SEND_TIMESTAMPS;
    sprops[i++] = SOLCLIENT_PROP_ENABLE_VAL;

    sprops[i] = nullptr;

    return sprops;
}

}}
