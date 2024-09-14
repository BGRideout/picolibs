#ifndef WEB_H
#define WEB_H

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "lwip/altcp.h"
#include "lwip/pbuf.h"
extern "C" 
{
#include "cyw43.h"
#include "dhcpserver.h"
}
#include "pico/time.h"
#include "httprequest.h"
#include "ws.h"

class WEB
{
private:
    struct altcp_pcb    *server_;                   // Server PCB

    class SENDBUF
    {
    private:
        uint8_t     *buffer_;                   // Buffer pointer
        int32_t     size_;                      // Buffer length
        int32_t     sent_;                      // Bytes sent
        bool        allocated_;                 // Buffer allocated

    public:
        SENDBUF() : buffer_(nullptr), size_(0), sent_(0), allocated_(false) {}
        SENDBUF(void *buf, uint32_t size, bool alloc = true);
        ~SENDBUF();

        uint32_t to_send() const { return size_ - sent_; }
        bool get_next(u16_t count, void **buffer, u16_t *buflen);
        void requeue(void *buffer, u16_t buflen);
    };

    class CLIENT
    {
    private:
        std::string             rqst_;              // Request message
        struct altcp_pcb        *pcb_;              // Client pcb
        bool                    closed_;            // Closed flag
        bool                    websocket_;         // Web socket open flag

        std::list<SENDBUF *>    sendbuf_;           // Send buffers
        HTTPRequest             http_;              // HTTP request info
        WebsocketPacketHeader_t wshdr_;             // Websocket message header

        CLIENT() : pcb_(nullptr), closed_(true), websocket_(false) {}

    public:
        CLIENT(struct altcp_pcb *client_pcb) : pcb_(client_pcb), closed_(false), websocket_(false) {}
        ~CLIENT();

        void addToRqst(const char *str, u16_t ll);
        bool rqstIsReady();
        void clearRqst() { rqst_.clear(); }
        void resetRqst();
        std::string &rqst() { return rqst_; }
        const std::string &rqst() const { return rqst_; }
        const HTTPRequest &http() const { return http_; }
        const WebsocketPacketHeader_t &wshdr() const { return wshdr_; }

        struct altcp_pcb *pcb() const { return pcb_; }

        bool isClosed() const { return closed_; }
        void setClosed() { closed_ = true; pcb_ = nullptr; }

        void setWebSocket() { websocket_ = true; }
        bool isWebSocket() const { return websocket_; }

        void queue_send(void *buffer, u16_t buflen, bool allocate);
        bool get_next(u16_t count, void **buffer, u16_t *buflen);
        bool more_to_send() const { return sendbuf_.size() > 0; }
        void requeue(void *buffer, u16_t buflen);
    };
    std::map<struct altcp_pcb *, CLIENT> clients_;    // Connected clients

    static err_t tcp_server_accept(void *arg, struct altcp_pcb *client_pcb, err_t err);
    static err_t tcp_server_recv(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);
    static err_t tcp_server_sent(void *arg, struct altcp_pcb *tpcb, u16_t len);
    static err_t tcp_server_poll(void *arg, struct altcp_pcb *tpcb);
    static void  tcp_server_err(void *arg, err_t err);

    void process_rqst(CLIENT &client);
    void process_http_rqst(CLIENT &client);
    void open_websocket(CLIENT &client);
    void process_websocket(CLIENT &client);
    void send_websocket(struct altcp_pcb *client_pcb, enum WebSocketOpCode opc, const std::string &payload, bool mask = false);

    void close_client(struct altcp_pcb *client_pcb, bool isClosed = false);

    int  wifi_state_;
    ip_addr_t wifi_addr_;
    bool connect_to_wifi();
    void check_wifi();
    void get_wifi(struct altcp_pcb *client_pcb);
    void update_wifi(const std::string &cmd);
    std::set<struct altcp_pcb *> scans_;
    std::map<std::string, int> ssids_;
    void scan_wifi(struct altcp_pcb *client_pcb);
    static int scan_cb(void *arg, const cyw43_ev_scan_result_t *rslt);
    void check_scan_finished();
    repeating_timer_t timer_;
    static bool timer_callback(repeating_timer_t *rt);

    int  ap_active_;
    bool ap_requested_;
    bool mdns_active_;
    dhcp_server_t dhcp_;
    void enable_ap_button();
    static void ap_button_callback(uint gpio, uint32_t event_mask);
    void start_ap();
    void stop_ap();

    static struct netif *wifi_netif(int ift) { return &cyw43_state.netif[ift]; }
    
    static WEB          *singleton_;                // Singleton pointer
    WEB();

    err_t send_buffer(struct altcp_pcb *client_pcb, void *buffer, u16_t buflen, bool allocate = true);
    err_t write_next(struct altcp_pcb *client_pcb);

    bool (*http_callback_)(WEB *web, void *client, const HTTPRequest &rqst);
    void (*message_callback_)(WEB *web, void *client, const std::string &msg);
    void (*notice_callback_)(int state);
    void send_notice(int state) {if (notice_callback_) notice_callback_(state);}

public:
    static WEB *get();
    bool init();

    void set_http_callback(bool (*cb)(WEB *web, void *client, const HTTPRequest &rqst)) { http_callback_ = cb; }
    void set_message_callback(void(*cb)(WEB *web, void *client, const std::string &msg)) { message_callback_ = cb; }
    void broadcast_websocket(const std::string &txt);

    static const int STA_INITIALIZING = 101;
    static const int STA_CONNECTED = 102;
    static const int STA_DISCONNECTED = 103;
    static const int AP_ACTIVE = 104;
    static const int AP_INACTIVE = 105;
    void set_notice_callback(void(*cb)(int state)) { notice_callback_ = cb;}

    bool send_data(void *client, const char *data, u16_t datalen, bool allocate=true);
    bool send_message(void *client, const std::string &message);
};

#endif
