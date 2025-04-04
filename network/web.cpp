#include "web.h"
#include "ws.h"
#include "cyw43_locker.h"

#include <stdio.h>

#include <pico/cyw43_arch.h>
#include <lwip/altcp_tcp.h>
#include <lwip/altcp_tls.h>
#include <lwip/apps/mdns.h>
#include <lwip/apps/sntp.h>
#include <lwip/netif.h>
#include <lwip/ip.h>
#include <lwip/tcp.h>
#include <lwip/prot/iana.h>
#include <mbedtls/sha1.h>
#include <mbedtls/base64.h>
#include <mbedtls/debug.h>

WEB *WEB::singleton_ = nullptr;

#ifndef HTTP_IDLE_TIME
#define HTTP_IDLE_TIME  10      // Maximum idle time of HTTP connction (minutes)
#endif
#ifndef WS_IDLE_TIME
#define WS_IDLE_TIME     0      // Maximum idle time of websocket connction (minutes)
#endif
#ifndef WS_CLOSE_WAIT
#define WS_CLOSE_WAIT    2      // Time to wait for response to websocket close (minutes)
#endif

//  HACK!!!
//  Set the SO_REUSEADDR option to allow stop/start of listen sockets
//  Should be replaced if altcp revised or supports this option
void set_reuseaddr(struct altcp_pcb *conn)
{
    if (conn && conn->inner_conn)
    {
        if (conn->inner_conn->state)
        {
            struct tcp_pcb *pcb = (struct tcp_pcb *)conn->inner_conn->state;
            ip_set_option(pcb, SOF_REUSEADDR);
        }
    }
}



WEB::WEB() : http_server_(nullptr), https_server_(nullptr),
             wifi_state_(CYW43_LINK_DOWN), tls_conf_(nullptr), reconnect_time_(0),
             ap_active_(0), ap_requested_(0), mdns_active_(false),
             http_callback_(nullptr), http_user_data_(nullptr),
             message_callback_(nullptr), message_user_data_(nullptr),
             notice_callback_(nullptr), notice_user_data_(nullptr),
             tls_callback_(nullptr)
{
    log_ = &default_logger_;
}

WEB *WEB::get()
{
    if (!singleton_)
    {
        singleton_ = new WEB();
    }
    return singleton_;
}

bool WEB::init()
{
    log_->print("Initializing webserver\n");
    CYW43Locker::lock();
    cyw43_arch_enable_sta_mode();
    cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 20, 1, 1, 1));
    uint32_t pm;
    cyw43_wifi_get_pm(&cyw43_state, &pm);

    mdns_resp_init();

#if SNTP_SERVER_DNS
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
#endif

    CYW43Locker::unlock();

    bool listening = false;
#ifdef USE_HTTPS
    listening = start_https();
#endif
    if (!listening)
    {
        listening = start_http();
    }
    
    add_repeating_timer_ms(500, timer_callback, this, &timer_);

    return true;
}

bool WEB::start_http()
{
    if (!http_server_)
    {
        CYW43Locker lock;
        u16_t port = LWIP_IANA_PORT_HTTP;
        struct altcp_tls_config * conf = nullptr;
        altcp_allocator_t alloc = {altcp_tcp_alloc, conf};
        struct altcp_pcb *pcb = altcp_new_ip_type(&alloc, IPADDR_TYPE_ANY);

        set_reuseaddr(pcb);
        err_t err = altcp_bind(pcb, IP_ANY_TYPE, port);
        if (err)
        {
            log_->print("failed to bind to port %u: %d\n", port, err);
            return false;
        }

        http_server_ = altcp_listen_with_backlog(pcb, 1);
        if (!http_server_)
        {
            log_->print("failed to listen to HTTP port\n");
            if (pcb)
            {
                altcp_close(pcb);
            }
            return false;
        }

        altcp_arg(http_server_, this);
        altcp_accept(http_server_, tcp_server_accept);
        log_->print("Listening on HTTP port %d\n", port);
    }
    return true;
}

bool WEB::start_https()
{
    bool ret = false;
#ifdef USE_HTTPS
    if (!https_server_ && tls_callback_)
    {
        CYW43Locker lock;
        u16_t port = LWIP_IANA_PORT_HTTPS;
        std::string cert;
        std::string pkey;
        std::string pkpass;
        if (tls_callback_(this, cert, pkey, pkpass))
        {
            ret = true;
            tls_conf_ = altcp_tls_create_config_server_privkey_cert((const u8_t *)pkey.c_str(), pkey.length() + 1,
                                                                    (const u8_t *)pkpass.c_str(), pkpass.length() + 1,
                                                                    (const u8_t *)cert.c_str(), cert.length() + 1);
            if (!tls_conf_)
            {
                log_->print("TLS configuration not loaded\n");
                return false;
            }
            mbedtls_debug_set_threshold(1);

            altcp_allocator_t alloc = {altcp_tls_alloc, tls_conf_};
            struct altcp_pcb *pcb = altcp_new_ip_type(&alloc, IPADDR_TYPE_ANY);

            set_reuseaddr(pcb);
            err_t err = altcp_bind(pcb, IP_ANY_TYPE, port);
            if (err)
            {
                log_->print("failed to bind to port %u: %d\n", port, err);
                altcp_close(pcb);
                altcp_tls_free_config(tls_conf_);
                tls_conf_ = nullptr;
                return false;
            }

            https_server_ = altcp_listen_with_backlog(pcb, 1);
            if (!https_server_)
            {
                log_->print("failed to listen to HTTPS port\n");
                altcp_close(pcb);
                altcp_tls_free_config(tls_conf_);
                tls_conf_ = nullptr;
                return false;
            }

            altcp_arg(https_server_, this);
            altcp_accept(https_server_, tcp_server_accept);
            log_->print("Listening on HTTPS port %d\n", port);
        }
    }
#endif
    return ret;
}

bool WEB::stop_http()
{
    bool ret = false;
    if (http_server_)
    {
        CYW43Locker lock;
        altcp_arg(http_server_, nullptr);
        altcp_accept(http_server_, nullptr);
        altcp_close(http_server_);
        http_server_ = nullptr;
        ret = true;
    }
    return ret;
}

bool WEB::stop_https()
{
    bool ret = false;
    if (https_server_)
    {
        CYW43Locker lock;
        altcp_arg(https_server_, nullptr);
        altcp_accept(https_server_, nullptr);
        altcp_close(https_server_);
        if (tls_conf_)
        {
            //altcp_tls_free_config(tls_conf_);
            tls_conf_ = nullptr;
        }
        https_server_ = nullptr;
        ret = true;
    }
    return ret;
}

bool WEB::connect_to_wifi(const std::string &hostname, const std::string &ssid, const std::string &password)
{
    bool ret = false;
    CYW43Locker lock;
    hostname_ = hostname;
    if (!hostname.empty())
    {
        netif_set_hostname(wifi_netif(CYW43_ITF_STA), hostname_.c_str());
    }

    wifi_ssid_ = ssid;
    wifi_pwd_ = password;
    if (!ssid.empty())
    {
        log_->print("Host '%s' connecting to Wi-Fi on SSID '%s' ...\n", hostname_.c_str(), wifi_ssid_.c_str());
        ret = cyw43_arch_wifi_connect_async(wifi_ssid_.c_str(), wifi_pwd_.c_str(), CYW43_AUTH_WPA2_AES_PSK) == 0;
    }
    return ret;
}

WEB::CLIENT *WEB::addClient(struct altcp_pcb *pcb)
{
    auto it = clientPCB_.find(pcb);
    if (it == clientPCB_.end())
    {
        CLIENT *client = new CLIENT(pcb);
        clientPCB_.emplace(pcb, client->handle());
        clientHndl_.emplace(client->handle(), client);
        return client;
    }
    else
    {
        log_->print("A client entry for pcb %p already eists. Points to handle %d\n", pcb, it->second);
    }
    return nullptr;
}

void WEB::deleteClient(struct altcp_pcb *pcb)
{
    auto it1 = clientPCB_.find(pcb);
    if (it1 != clientPCB_.end())
    {
        auto it2 = clientHndl_.find(it1->second);
        if (it2 != clientHndl_.end())
        {
            delete it2->second;
            clientHndl_.erase(it2);
        }
        clientPCB_.erase(it1);
    }
}

WEB::CLIENT *WEB::findClient(ClientHandle handle)
{
    auto it = clientHndl_.find(handle);
    if (it != clientHndl_.end())
    {
        return it->second;
    }
    return nullptr;
}

WEB::CLIENT *WEB::findClient(struct altcp_pcb *pcb)
{
    auto it = clientPCB_.find(pcb);
    if (it != clientPCB_.end())
    {
        return findClient(it->second);
    }
    return nullptr;
}

bool WEB::update_wifi(const std::string &hostname, const std::string &ssid, const std::string &password)
{
    bool ret = true;
    if (hostname != hostname_ || ssid != wifi_ssid_ || password != wifi_pwd_)
    {
        CYW43Locker lock;
        cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
        ret = connect_to_wifi(hostname, ssid, password);
    }
    else
    {
        switch (wifi_state_)
        {
        case CYW43_LINK_JOIN:
        case CYW43_LINK_NOIP:
            send_notice(STA_INITIALIZING);
            break;

        case CYW43_LINK_UP:
            send_notice(STA_CONNECTED);
            break;

        case CYW43_LINK_DOWN:
        case CYW43_LINK_FAIL:
        case CYW43_LINK_NONET:
        case CYW43_LINK_BADAUTH:
            send_notice(STA_DISCONNECTED);
            break;
        }
    }
    return ret;
}

err_t WEB::tcp_server_accept(void *arg, struct altcp_pcb *client_pcb, err_t err)
{
    WEB *web = get();
    if (err != ERR_OK || client_pcb == NULL) {
        WEB::get()->log_->print("Failure in accept %d\n", err);
        return ERR_VAL;
    }
    set_reuseaddr(client_pcb);
    CLIENT *client = web->addClient(client_pcb);
#if SNTP_SERVER_DNS
    time_t now;
    time(&now);
    char timbuf[64];
    strftime(timbuf, sizeof(timbuf), "%c", localtime(&now));
    web->log_->print_debug(1, "%s ", timbuf);
#endif
    web->log_->print_debug(1, "Client connected %p (handle %d) (%d clients)\n", client_pcb, client->handle(), web->clientPCB_.size());
    if (web->log_->isDebug(3))
    {
        for (auto it = web->clientHndl_.cbegin(); it != web->clientHndl_.cend(); ++it)
        {
            web->log_->print("  %c-%p (%d)", it->second->isWebSocket() ? 'w' : 'h', it->first, it->second->handle());
        }
        if (web->clientHndl_.size() > 0) web->log_->print("\n");
    }

    altcp_arg(client_pcb, client_pcb);
    altcp_sent(client_pcb, tcp_server_sent);
    altcp_recv(client_pcb, tcp_server_recv);
    altcp_poll(client_pcb, tcp_server_poll, 1 * 2);
    altcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

err_t WEB::tcp_server_recv(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    WEB *web = get();
    CLIENT *client = web->findClient(tpcb);
    if (!p)
    {
        web->log_->print_debug(1, "Client %p (%d) closed by peer\n", tpcb, client ? client->handle() : 0);
        web->close_client(tpcb, true);
        return ERR_OK;
    }

    // this method is callback from lwIP, so cyw43_arch_lwip_begin is not required, however you
    // can use this method to cause an assertion in debug mode, if this method is called when
    // cyw43_arch_lwip_begin IS needed
    cyw43_arch_lwip_check();
    if (p->tot_len > 0)
    {
        // Receive the buffer
        bool ready = false;
        if (client)
        {
            char buf[64];
            u16_t ll = 0;
            while (ll < p->tot_len)
            {
                u16_t l = pbuf_copy_partial(p, buf, sizeof(buf), ll);
                ll += l;
                client->addToRqst(buf, l);
            }
        }

        altcp_recved(tpcb, p->tot_len);

        if (client)
        {
            while (client->rqstIsReady())
            {
                if (!client->isWebSocket())
                {
                    web->process_rqst(*client);
                }
                else
                {
                    web->process_websocket(*client);
                }

                //  Look up again in case client was closed
                web->findClient(tpcb);
                if (client)
                {
                    client->resetRqst();
                }
            }
        }
    }
    else
    {
        WEB::get()->log_->print("Zero length receive from %p\n", tpcb);
    }
    pbuf_free(p);

    return ERR_OK;
}

err_t WEB::tcp_server_sent(void *arg, struct altcp_pcb *tpcb, u16_t len)
{
    WEB *web = get();
    CLIENT *client = web->findClient(tpcb);
    if (client)
    {
        client->acknowledge(len);
    }
    web->write_next(tpcb);
    return ERR_OK;
}

err_t WEB::tcp_server_poll(void *arg, struct altcp_pcb *tpcb)
{
    WEB *web = get();
    CLIENT *client = web->findClient(tpcb);
    if (client)
    {
        //  Test for match on PCB to avoid race condition on close
        if (client->pcb() == tpcb)
        {
            if (client->more_to_send())
            {
                web->log_->print_debug(1, "Sending to %d (%s) on poll (%d clients)\n",
                                       client->handle(), client->isWebSocket() ? "ws" : "http", web->clientPCB_.size());
                web->write_next(client->pcb());
            }

            //  Check for idle connections
            if (client->isIdle())
            {
                if (client->isWebSocket())
                {
                    if (!client->wasWSCloseSent())
                    {
                        web->log_->print_debug(1, "Closing websocket %p (%d) for idle (%d clients)\n",
                                                tpcb, client->handle(), web->clientPCB_.size());
                        client->setWSCloseSent();
                        web->send_websocket(client->pcb(), WEBSOCKET_OPCODE_CLOSE, std::string());
                    }
                    else
                    {
                        web->log_->print_debug(1, "Closing websocket %p (%d) after no response to close (%d clients)\n",
                                                tpcb, client->handle(), web->clientPCB_.size());
                        web->mark_for_close(tpcb);
                    }
                }
                else
                {
                   web->log_->print_debug(1, "Closing http %p (%d) for idle (%d clients)\n",
                                            tpcb, client->handle(), web->clientPCB_.size());
                    web->mark_for_close(tpcb);
                }
            }

            //  If marked for close, try close now
            if (client->isClosed())
            {
                web->close_client(tpcb);
            }
        }
        else
        {
            web->log_->print("Poll on closed pcb %p (client %d)\n", tpcb, client->handle());
            web->close_client(tpcb);
        }
    }
    return ERR_OK;
}

void WEB::tcp_server_err(void *arg, err_t err)
{
    WEB *web = get();
    altcp_pcb *client_pcb = (altcp_pcb *)arg;
    CLIENT *client = web->findClient(client_pcb);
    WEB::get()->log_->print("Error %d on client %p (%d)\n", err, client_pcb, client ? client->handle() : 0);
    if (client)
    {
        web->deleteClient(client_pcb);
    }
}

err_t WEB::send_buffer(struct altcp_pcb *client_pcb, void *buffer, u16_t buflen, Allocation allocate)
{
    CLIENT *client = get()->findClient(client_pcb);
    if (client)
    {
        client->queue_send(buffer, buflen, allocate);
        write_next(client_pcb);
    }
    return ERR_OK;
}

err_t WEB::write_next(altcp_pcb *client_pcb)
{
    err_t err = ERR_OK;
    CLIENT *client = get()->findClient(client_pcb);
    if (client)
    {
        client->activity();
        u16_t nn = altcp_sndbuf(client_pcb);
        if (nn > TCP_MSS)
        {
            nn = TCP_MSS;
        }
        void *buffer;
        u16_t buflen;
        if (client->get_next(nn, &buffer, &buflen))
        {
            CYW43Locker lock;
            cyw43_arch_lwip_check();
            err = altcp_write(client_pcb, buffer, buflen, 0);
            altcp_output(client_pcb);
            if (err != ERR_OK)
            {
                log_->print("Failed to write %d bytes of data %d to %p (%d)\n", buflen, err, client_pcb, client->handle());
                client->requeue(buffer, buflen);
            }
        }

        if (client->isClosed() && !client->more_to_send())
        {
            close_client(client_pcb);
        }
    }
    else
    {
        log_->print("Unknown client %p for write\n", client_pcb);
    }
    return err;    
}

void WEB::process_rqst(CLIENT &client)
{
    bool ok = false;
    bool close = true;
    client.activity();
    log_->print_debug(2, "Request from %p (%d):\n%s\n", client.pcb(), client.handle(), client.rqst().c_str());
    if (!client.isWebSocket())
    {
        ok = true;
        if (client.http().header("Upgrade") == "websocket")
        {
            open_websocket(client);
        }
        else
        {
            if (client.http().type() == "POST")
            {
                client.http().parseRequest(client.rqst(), true);
            }
            process_http_rqst(client, close);
        }
    }

    if (!ok)
    {
        send_buffer(client.pcb(), (void *)"HTTP/1.0 500 Internal Server Error\r\n\r\n", 38);
    }

    if (!client.isWebSocket() && close)
    {
        close_client(client.pcb());
    }
}

void WEB::process_http_rqst(CLIENT &client, bool &close)
{
    const char *data;
    u16_t datalen = 0;
    bool is_static = false;
    if (http_callback_ && http_callback_(this, client.handle(), client.http(), close, http_user_data_))
    {
        ;
    }
    else
    {
        send_buffer(client.pcb(), (void *)"HTTP/1.0 404 NOT_FOUND\r\n\r\n", 26);
    }
}

bool WEB::send_data(ClientHandle client, const char *data, u16_t datalen, Allocation allocate)
{
    CLIENT *clptr = findClient(client);
    CLIENT *clpcb = findClient(clptr->pcb());
    if (clptr && clpcb == clptr)
    {
        send_buffer(clptr->pcb(), (void *)data, datalen, allocate);
    }
    else
    {
        log_->print("send_data to non-existent client handle %d (pcb: %p)\n", client, clpcb);
    }
    return clptr != nullptr;
}

void WEB::open_websocket(CLIENT &client)
{
    log_->print_debug(1, "Accepting websocket connection on %p (handle %d) url: %s\n", client.pcb(), client.handle(), client.http().url().c_str());
    std::string host = client.http().header("Host");
    std::string key = client.http().header("Sec-WebSocket-Key");
    
    bool hasConnection = client.http().header("Connection").find("Upgrade") != std::string::npos;
    bool hasUpgrade = client.http().header("Upgrade") == "websocket";
    bool hasOrigin = client.http().headerIndex("Origin") > 0;
    bool hasVersion = client.http().header("Sec-WebSocket-Version") == "13";

    if (hasConnection && hasOrigin && hasUpgrade && hasVersion && host.length() > 0)
    {
        key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        unsigned char sha1[20];
        mbedtls_sha1((const unsigned char *)key.c_str(), key.length(), sha1);
        unsigned char b64[64];
        size_t b64ll;
        mbedtls_base64_encode(b64, sizeof(b64), &b64ll, sha1, sizeof(sha1));

        std::string resp("HTTP/1.1 101 Switching Protocols\r\n"
                         "Upgrade: websocket\r\n"
                         "Connection: Upgrade\r\n"
                         "Sec-WebSocket-Accept: ");

        resp.append((const char *)b64, b64ll);
        resp += "\r\n\r\n";

        send_buffer(client.pcb(), (void *)resp.c_str(), resp.length());

        client.setWebSocket();
        client.clearRqst();
    }
    else
    {
        log_->print("Bad websocket request from %p\n", client.pcb());
    }
}

void WEB::process_websocket(CLIENT &client)
{
    client.activity();
    std::string func;
    uint8_t opc = client.wshdr().meta.bits.OPCODE;
    std::string payload = client.rqst().substr(client.wshdr().start, client.wshdr().length);
    switch (opc)
    {
    case WEBSOCKET_OPCODE_TEXT:
        if (message_callback_)
        {
            message_callback_(this, client.handle(), payload, message_user_data_);
        }
        break;

    case WEBSOCKET_OPCODE_PING:
        send_websocket(client.pcb(), WEBSOCKET_OPCODE_PONG, payload);
        break;

    case WEBSOCKET_OPCODE_CLOSE:
        log_->print_debug(1, "WS %p (%d) received close opcode\n", client.pcb(), client.handle());
        if (!client.wasWSCloseSent())
        {
            client.setWSCloseSent();
            send_websocket(client.pcb(), WEBSOCKET_OPCODE_CLOSE, payload);
        }
        mark_for_close(client.pcb());
        break;

    default:
        log_->print("Unhandled websocket opcode %d from %p\n", opc, client.pcb());
    }
}

bool WEB::send_message(ClientHandle client, const std::string &message)
{
    CLIENT *clptr = findClient(client);
    CLIENT *clpcb = findClient(clptr->pcb());
    if (clptr && clpcb == clptr)
    {
        log_->print_debug(2, "%p (%d) message: %s\n", clptr->pcb(), clptr->handle(), message.c_str());
        send_websocket(clptr->pcb(), WEBSOCKET_OPCODE_TEXT, message);
    }
    else
    {
        log_->print("send_message to non-existent client handle %d (pcb: %p)\n", client, clpcb);
    }
    return clptr != nullptr;
}

bool WEB::send_message(ClientHandle client, TXT &message)
{
    CLIENT *clptr = findClient(client);
    CLIENT *clpcb = findClient(clptr->pcb());
    if (clptr && clpcb == clptr)
    {
        log_->print_debug(2, "%p (%d) message: %s\n", clptr->pcb(), clptr->handle(), message.data());
        WS::BuildPacket(WEBSOCKET_OPCODE_TEXT, message, false);
        char *data = message.data();
        uint32_t datalen = message.datasize();
        message.release();
        send_buffer(clptr->pcb(), data, datalen, WEB::PREALL);
    }
    else
    {
        log_->print("send_message to non-existent client handle %d (pcb: %p)\n", client, clpcb);
    }
    return clptr != nullptr;
}

void WEB::send_websocket(struct altcp_pcb *client_pcb, enum WebSocketOpCode opc, const std::string &payload, bool mask)
{
    std::string msg;
    WS::BuildPacket(opc, payload, msg, mask);
    send_buffer(client_pcb, (void *)msg.c_str(), msg.length());
}

void WEB::broadcast_websocket(const std::string &txt)
{
    CYW43Locker lock;
    for (auto it = clientHndl_.cbegin(); it != clientHndl_.cend(); ++it)
    {
        if (it->second->isWebSocket())
        {
            send_websocket(it->second->pcb(), WEBSOCKET_OPCODE_TEXT, txt);
        }
    }
}

void WEB::broadcast_websocket(TXT &txt)
{
    CYW43Locker lock;
    int nwsc = 0;
    for (auto it = clientHndl_.cbegin(); it != clientHndl_.cend(); ++it)
    {
        if (it->second->isWebSocket())
        {
            nwsc += 1;
        }
    }
    for (auto it = clientHndl_.cbegin(); it != clientHndl_.cend(); ++it)
    {
        if (it->second->isWebSocket())
        {
            if (--nwsc > 0)
            {
                TXT txt1(txt.data(), txt.datasize(), txt.datasize() + 16);
                send_message(it->first, txt1);
            }
            else
            {
                send_message(it->first, txt);
            }
        }
    }
}

void WEB::mark_for_close(struct altcp_pcb *client_pcb)
{
    CLIENT *client = findClient(client_pcb);
    if (client)
    {
        client->setClosed();
    }
}

void WEB::close_client(struct altcp_pcb *client_pcb, bool isClosed)
{
    CLIENT *client = findClient(client_pcb);
    if (client)
    {
        if (!isClosed)
        {
            client->setClosed();
            client->acknowledge(0);
            if (!client->more_to_send())
            {
                err_t csts = altcp_close(client_pcb);
                if (csts == ERR_OK)
                {
                    log_->print_debug(1, "Closed %s %p (%d). client count = %d\n",
                                    (client->isWebSocket() ? "ws" : "http"), client_pcb, client->handle(), clientPCB_.size() - 1);
                    deleteClient(client_pcb);
                    if (log_->isDebug(3))
                    {
                        for (auto it = clientHndl_.cbegin(); it != clientHndl_.cend(); ++it)
                        {
                            log_->print(" %c-%p (%d)", client->isWebSocket() ? 'w' : 'h', client->pcb(), client->handle());
                        }
                        if (clientHndl_.size() > 0) log_->print("\n");
                    }
                }
                else
                {
                    log_->print_debug(1, "Deferred close of %s %s %p (%d). status = %d\n",
                                    (client->isWebSocket() ? "ws" : "http"), client_pcb, client->handle(), csts);
                }
            }
            else
            {
                log_->print_debug(1, "Waiting to close %s %p (%d)\n", (client->isWebSocket() ? "ws" : "http"), client_pcb, client->handle());
            }
        }
        else
        {
            log_->print_debug(1, "Closing %s %p (%d)\n", (client->isWebSocket() ? "ws" : "http"), client_pcb, client->handle());
            client->setClosed();
            CYW43Locker lock;
            altcp_close(client_pcb);
            deleteClient(client_pcb);
        }
    }
    else
    {
        if (!isClosed)
        {
            CYW43Locker lock;
            err_t csts = altcp_close(client_pcb);
            log_->print_debug(1, "Close status %p  = %d\n", client_pcb, csts);
        }   
    }
}

void WEB::check_wifi()
{
    netif *ni = wifi_netif(CYW43_ITF_STA);
    CYW43Locker::lock();
    int sts = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    CYW43Locker::unlock();
    if (sts != wifi_state_ || !ip_addr_eq(&ni->ip_addr, &wifi_addr_))
    {
        wifi_state_ = sts;
        wifi_addr_ = ni->ip_addr;

        switch (wifi_state_)
        {
        case CYW43_LINK_JOIN:
        case CYW43_LINK_NOIP:
            // Progress - no action needed
            send_notice(STA_INITIALIZING);
            break;

        case CYW43_LINK_UP:
            //  Connected
            CYW43Locker::lock();
            if (mdns_active_)
            {
                mdns_resp_remove_netif(ni);
            }
            mdns_resp_add_netif(ni, hostname_.c_str());
            mdns_resp_announce(ni);
            CYW43Locker::unlock();
            mdns_active_ = true;
            log_->print("Connected to WiFi with IP address %s\n", ip4addr_ntoa(netif_ip4_addr(ni)));
            send_notice(STA_CONNECTED);
            reconnect_time_ = 0;
            break;

        case CYW43_LINK_DOWN:
        case CYW43_LINK_FAIL:
        case CYW43_LINK_NONET:
            //  Not connected
            log_->print("WiFi disconnected. status = %s\n", (wifi_state_ == CYW43_LINK_DOWN) ? "link down" :
                                                       (wifi_state_ == CYW43_LINK_FAIL) ? "link failed" :
                                                       "No network found");
            send_notice(STA_DISCONNECTED);
            reconnect_time_ = reconnect_interval_;
            break;

        case CYW43_LINK_BADAUTH:
            //  Need intervention to connect
            log_->print("WiFi authentication failed\n");
            send_notice(STA_DISCONNECTED);
            break;
        }
    }

    //  Checck for reconnection attempt needed
    if (reconnect_time_ > 0)
    {
        reconnect_time_ -= 1;
        if (reconnect_time_ == 0)
        {
            switch (wifi_state_)
            {
            case CYW43_LINK_DOWN:
            case CYW43_LINK_FAIL:
            case CYW43_LINK_NONET:
                if (wifi_ssid_.length() > 0)
                {
                    bool ret = connect_to_wifi(hostname_, wifi_ssid_, wifi_pwd_);
                    log_->print("Reconnect WiFi\n");
                }
            }
        }
    }

    if (ap_active_ > 0)
    {
        ap_active_ -= 1;
        if (ap_active_ == 0)
        {
            stop_ap();
        }
    }
    if (ap_requested_ > 0)
    {
        start_ap();
    }
}

void WEB::scan_wifi(ClientHandle client, WiFiScan_cb callback, void *user_data)
{
    CYW43Locker lock;
    if (!cyw43_wifi_scan_active(&cyw43_state))
    {
        cyw43_wifi_scan_options_t opts = {0};
        int sts = cyw43_wifi_scan(&cyw43_state, &opts, this, scan_cb);
        scans_.clear();
    }
    ScanRqst rqst = {.client = client, .cb = callback, .user_data = user_data };
    scans_.push_back(rqst);
}

int WEB::scan_cb(void *arg, const cyw43_ev_scan_result_t *rslt)
{
    if (rslt)
    {
        std::string ssid;
        ssid.append((char *)rslt->ssid, rslt->ssid_len);
        get()->ssids_[ssid] = rslt->rssi;
    }
    return 0;
}

void WEB::check_scan_finished()
{
    if (scans_.size() > 0)
    {
        CYW43Locker::lock();
        bool active = cyw43_wifi_scan_active(&cyw43_state);
        CYW43Locker::unlock();
        if (!active)
        {
            for (auto it = scans_.cbegin(); it != scans_.cend(); ++it)
            {
                if (it->cb != nullptr)
                {
                    it->cb(this, it->client, ssids_, it->user_data);
                }
            }

            ssids_.clear();
            scans_.clear();
        }
    }
}

bool WEB::timer_callback(repeating_timer_t *rt)
{
    get()->check_wifi();
    get()->check_scan_finished();
    return true;
}

void WEB::start_ap()
{
    if (ap_active_ == 0)
    {
        log_->print("Starting AP %s\n", ap_name_.c_str());
        CYW43Locker lock;
        cyw43_arch_enable_ap_mode(ap_name_.c_str(), "12345678", CYW43_AUTH_WPA2_AES_PSK);
        netif_set_hostname(wifi_netif(CYW43_ITF_AP), ap_name_.c_str());
    
        // Start the dhcp server
        ip4_addr_t addr;
        ip4_addr_t mask;
        IP4_ADDR(ip_2_ip4(&addr), 192, 168, 4, 1);
        IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);
        dhcp_server_init(&dhcp_, &addr, &mask);
    }
    else
    {
        log_->print("AP is already active. Timer reset.\n");
    }
    ap_active_ = ap_requested_ * 60 * 2;
    ap_requested_ = 0;
    send_notice(AP_ACTIVE);
}

void WEB::stop_ap()
{
    CYW43Locker::lock();
    dhcp_server_deinit(&dhcp_);
    cyw43_arch_disable_ap_mode();
    CYW43Locker::unlock();
    log_->print("AP deactivated\n");
    send_notice(AP_INACTIVE);
}

WEB::CLIENT::~CLIENT()
{
    while (sendbuf_.size() > 0)
    {
        delete sendbuf_.front();
        sendbuf_.pop_front();
    }
}

void WEB::CLIENT::addToRqst(const char *str, u16_t ll)
{
    rqst_.append(str, ll);
}

bool WEB::CLIENT::rqstIsReady()
{
    bool ret = false;
    if (!isWebSocket())
    {
        ret = http_.parseRequest(rqst_, false);
    }
    else
    {
        ret = WS::ParsePacket(&wshdr_, rqst_) == WEBSOCKET_SUCCESS;
    }
    return ret;
}

void WEB::CLIENT::queue_send(void *buffer, u16_t buflen, Allocation allocate)
{
    WEB::SENDBUF *sbuf = new WEB::SENDBUF(buffer, buflen, allocate);
    sendbuf_.push_back(sbuf);
}

bool WEB::CLIENT::get_next(u16_t count, void **buffer, u16_t *buflen)
{
    bool ret = false;
    *buffer = nullptr;
    *buflen = 0;
    if (count > 0)
    {
        acknowledge(0);
        if (sendbuf_.size() > 0 && sendbuf_.front()->to_send() > 0)
        {
            ret = sendbuf_.front()->get_next(count, buffer, buflen);
        }
    }

    return ret;
}

void WEB::CLIENT::requeue(void *buffer, u16_t buflen)
{
    if (sendbuf_.size() > 0)
    {
        sendbuf_.front()->requeue(buffer, buflen);
    }
}

void WEB::CLIENT::acknowledge(int count)
{
    while (sendbuf_.size() > 0)
    {
        SENDBUF *sb = sendbuf_.front();
        count = sb->acknowledge(count);
        if (sb->isAcknowledged())
        {
            sendbuf_.pop_front();
            delete sb;
        }
        if (count == 0)
        {
            break;
        }
    }
}

void WEB::CLIENT::resetRqst()
{
    if (!isWebSocket())
    {
        std::size_t ii = rqst_.find("\r\n\r\n");
        if (http_.isComplete())
        {
            rqst_.erase(0, http_.size());
        }
        else
        {
            rqst_.clear();
        }
        http_.clear();
    }
    else
    {
        std::size_t ii = wshdr_.start + wshdr_.length;
        if (ii < rqst_.length())
        {
            rqst_.erase(0, ii);
        }
        else
        {
            rqst_.clear();
        }
    }
}

bool WEB::CLIENT::isIdle() const
{
    bool ret = false;
    if (!isClosed())
    {
        int64_t idle = absolute_time_diff_us(last_activity_, get_absolute_time()) / 60000000LL;
        if (isWebSocket())
        {
            #if WS_IDLE_TIME > 0
                if (wasWSCloseSent())
                {
                    ret = idle > WS_CLOSE_WAIT;
                }
                else
                {
                    ret = idle > WS_IDLE_TIME;
                }
            #endif    
        }
        else
        {
            #if HTTP_IDLE_TIME > 0
                ret = idle > HTTP_IDLE_TIME;
            #endif
        }
    }
    return ret;
}

ClientHandle WEB::CLIENT::next_handle_ = 999;

ClientHandle WEB::CLIENT::nextHandle()
{
    ClientHandle start = next_handle_;
    next_handle_ += 1;
    if (next_handle_ == 0) next_handle_ = 1000;
    while (WEB::get()->findClient(next_handle_))
    {
        next_handle_ += 1;
        if (next_handle_ == 0) next_handle_ = 1000;
        if (next_handle_ == start) assert("No client handles");
    }
    return next_handle_;
}

WEB::SENDBUF::SENDBUF(void *buf, uint32_t size, Allocation alloc)
 : buffer_((uint8_t *)buf), size_(size), sent_(0), ack_(0), allocated_(alloc)
{
    if (allocated_ == ALLOC)
    {
        buffer_ = new uint8_t[size];
        memcpy(buffer_, buf, size);
    }
}

WEB::SENDBUF::~SENDBUF()
{
    if (allocated_ != STAT)
    {
        delete [] buffer_;
    }
}

bool WEB::SENDBUF::get_next(u16_t count, void **buffer, u16_t *buflen)
{
    bool ret = false;
    *buffer = nullptr;
    uint32_t nn = to_send();
    if (count < nn)
    {
        nn = count;
    }
    if (nn > 0)
    {
        *buffer = &buffer_[sent_];
        ret = true;
    }
    *buflen = nn;
    sent_ += nn;

    return ret;
}

void WEB::SENDBUF::requeue(void *buffer, u16_t buflen)
{
    int32_t nn = sent_ - buflen;
    if (nn >= 0)
    {
        if (memcmp(&buffer_[nn], buffer, buflen) == 0)
        {
            sent_ = nn;
            WEB::get()->log_->print("%d bytes requeued\n", buflen);
        }
        else
        {
            WEB::get()->log_->print("Buffer mismatch! %d bytes not requeued\n", buflen);
        }
    }
    else
    {
        WEB::get()->log_->print("%d bytes to requeue exceeds %d sent\n", buflen, sent_);
    }
}

int32_t WEB::SENDBUF::acknowledge(int count)
{
    int32_t na = ack_ + count;
    if (na > size_)
    {
        ack_ = size_;
        na -= size_;
    }
    else
    {
        ack_ = na;
        na = 0;
    }

    if (ack_ > sent_)
    {
        WEB::get()->log_->print("ERROR: Acknowledge count exceeds sent count\n");
    }
    return na;
}
