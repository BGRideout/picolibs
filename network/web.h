#ifndef WEB_H
#define WEB_H

#include <list>
#include <map>
#include <string>
#include <vector>
#include <stdint.h>

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

class WEB;

typedef uint32_t   ClientHandle;
typedef std::map<std::string, int> WiFiScanData;    // SSID -> RSSI
typedef bool (*WiFiScan_cb)(WEB *, ClientHandle, const WiFiScanData &, void *);

class WEB
{
public:
    enum Allocation
    {
        ALLOC = 1,      // Buffer to be allocated
        STAT,           // Buffer is static
        PREALL          // Buffer is preallocated
    };

private:
    struct altcp_pcb    *server_;               // Server PCB

    class SENDBUF
    {
    private:
        uint8_t     *buffer_;                   // Buffer pointer
        int32_t     size_;                      // Buffer length
        int32_t     sent_;                      // Bytes sent
        Allocation  allocated_;                 // Buffer allocation type

    public:
        SENDBUF() : buffer_(nullptr), size_(0), sent_(0), allocated_(ALLOC) {}
        SENDBUF(void *buf, uint32_t size, Allocation alloc = ALLOC);
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

        ClientHandle            handle_;            // Client handle
        static ClientHandle     next_handle_;       // Next handle
        static ClientHandle     nextHandle();       // Get next handle

        CLIENT() : pcb_(nullptr), closed_(true), websocket_(false), handle_(0) {}

    public:
        CLIENT(struct altcp_pcb *client_pcb) : pcb_(client_pcb), closed_(false), websocket_(false) { handle_ = nextHandle(); }
        ~CLIENT();

        void addToRqst(const char *str, u16_t ll);
        bool rqstIsReady();
        void clearRqst() { rqst_.clear(); }
        void resetRqst();
        std::string &rqst() { return rqst_; }
        const std::string &rqst() const { return rqst_; }
        const HTTPRequest &http() const { return http_; }
        HTTPRequest &http() { return http_; }
        const WebsocketPacketHeader_t &wshdr() const { return wshdr_; }

        struct altcp_pcb *pcb() const { return pcb_; }

        bool isClosed() const { return closed_; }
        void setClosed() { closed_ = true; pcb_ = nullptr; }

        void setWebSocket() { websocket_ = true; }
        bool isWebSocket() const { return websocket_; }

        void queue_send(void *buffer, u16_t buflen, Allocation allocate);
        bool get_next(u16_t count, void **buffer, u16_t *buflen);
        bool more_to_send() const { return sendbuf_.size() > 0; }
        void requeue(void *buffer, u16_t buflen);

        const ClientHandle &handle() const { return handle_; }
    };
    std::map<ClientHandle, CLIENT *> clientHndl_;           // Connected clients by handle
    std::map<struct altcp_pcb *, ClientHandle> clientPCB_;  // Connected clienthandles by PCB
    CLIENT *addClient(struct altcp_pcb *pcb);
    void    deleteClient(struct altcp_pcb *pcb);
    CLIENT *findClient(ClientHandle handle);
    CLIENT *findClient(struct altcp_pcb *pcb);

    static err_t tcp_server_accept(void *arg, struct altcp_pcb *client_pcb, err_t err);
    static err_t tcp_server_recv(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err);
    static err_t tcp_server_sent(void *arg, struct altcp_pcb *tpcb, u16_t len);
    static err_t tcp_server_poll(void *arg, struct altcp_pcb *tpcb);
    static void  tcp_server_err(void *arg, err_t err);

    void process_rqst(CLIENT &client);
    void process_http_rqst(CLIENT &client, bool &close);
    void open_websocket(CLIENT &client);
    void process_websocket(CLIENT &client);
    void send_websocket(struct altcp_pcb *client_pcb, enum WebSocketOpCode opc, const std::string &payload, bool mask = false);

    void close_client(struct altcp_pcb *client_pcb, bool isClosed = false);

    std::string     hostname_;              // Host name
    std::string     wifi_ssid_;             // WiFi SSID
    std::string     wifi_pwd_;              // WiFI password

    int             wifi_state_;            // WiFi state
    ip_addr_t       wifi_addr_;             // WiFi IP address

    void check_wifi();

    struct ScanRqst
    {
        ClientHandle    client;
        WiFiScan_cb     cb;
        void            *user_data;
    };
    std::vector<ScanRqst> scans_;
    WiFiScanData ssids_;
    static int scan_cb(void *arg, const cyw43_ev_scan_result_t *rslt);
    void check_scan_finished();
    repeating_timer_t timer_;
    static bool timer_callback(repeating_timer_t *rt);

    std::string ap_name_;
    int  ap_active_;
    int  ap_requested_;
    bool mdns_active_;
    dhcp_server_t dhcp_;
    void start_ap();
    void stop_ap();

    static struct netif *wifi_netif(int ift) { return &cyw43_state.netif[ift]; }

    static int debug_level_;                               // Debug level
    static bool isDebug(int level = 1) { return level <= debug_level_; }
    
    static WEB          *singleton_;                // Singleton pointer
    WEB();

    err_t send_buffer(struct altcp_pcb *client_pcb, void *buffer, u16_t buflen, Allocation allocate = ALLOC);
    err_t write_next(struct altcp_pcb *client_pcb);

    bool (*http_callback_)(WEB *web, ClientHandle client, const HTTPRequest &rqst, bool &close);
    void (*message_callback_)(WEB *web, ClientHandle client, const std::string &msg);
    void (*notice_callback_)(int state);
    void send_notice(int state) {if (notice_callback_) notice_callback_(state);}

public:
    static WEB *get();
    bool init();
    bool connect_to_wifi(const std::string &hostname, const std::string &ssid, const std::string &password);
    bool update_wifi(const std::string &hostname, const std::string &ssid, const std::string &password);

    const std::string &hostname() const { return hostname_; }
    const std::string &wifi_ssid() const { return wifi_ssid_; }
    const std::string ip_addr() const { return ip4addr_ntoa(&wifi_addr_); }

    void set_http_callback(bool (*cb)(WEB *web, ClientHandle client, const HTTPRequest &rqst, bool &close)) { http_callback_ = cb; }
    void set_message_callback(void(*cb)(WEB *web, ClientHandle client, const std::string &msg)) { message_callback_ = cb; }
    void broadcast_websocket(const std::string &txt);

    static const int STA_INITIALIZING = 101;
    static const int STA_CONNECTED = 102;
    static const int STA_DISCONNECTED = 103;
    static const int AP_ACTIVE = 104;
    static const int AP_INACTIVE = 105;
    void set_notice_callback(void(*cb)(int state)) { notice_callback_ = cb;}

    bool send_data(ClientHandle client, const char *data, u16_t datalen, Allocation allocate=ALLOC);
    bool send_message(ClientHandle client, const std::string &message);

    void enable_ap(int minutes = 30, const std::string &name = "webapp") { ap_requested_ = minutes; ap_name_ = name; }
    bool ap_active() const { return ap_active_ > 0; }

    void scan_wifi(ClientHandle client, WiFiScan_cb callback, void *user_data = nullptr);

    void setDebug(int level) { debug_level_ = level; }
};

#endif
