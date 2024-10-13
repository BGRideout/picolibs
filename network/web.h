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
#include "logger.h"

class WEB;

typedef uint32_t   ClientHandle;

/**
 * @brief   Data returned by WiFI scan
 * 
 * @details Map of names and signal strength
 * 
 * @param   first       WiFI access point name (SSID)
 * @param   second      Relative signal strength (RSSI)
 */
typedef std::map<std::string, int> WiFiScanData;    // SSID -> RSSI

/**
 * @brief   Callback function for WiFi SSDI scan
 * 
 * @param   web         Pointer to WEB object
 * @param   client      Handle of client connection
 * @param   ssids       Map of SSID name ro signal stength (RSSI)
 * @param   user_data   Data pointer for user data
 */
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
        int32_t     ack_;                       // Bytes acknowledged
        Allocation  allocated_;                 // Buffer allocation type

    public:
        SENDBUF() : buffer_(nullptr), size_(0), sent_(0), ack_(0), allocated_(ALLOC) {}
        SENDBUF(void *buf, uint32_t size, Allocation alloc = ALLOC);
        ~SENDBUF();

        uint32_t to_send() const { return size_ - sent_; }
        bool get_next(u16_t count, void **buffer, u16_t *buflen);
        void requeue(void *buffer, u16_t buflen);

        int32_t acknowledge(int count);
        bool isAcknowledged() const { return ack_ == size_; }
    };

    class CLIENT
    {
    private:
        std::string             rqst_;              // Request message
        struct altcp_pcb        *pcb_;              // Client pcb
        bool                    closed_;            // Closed flag
        bool                    websocket_;         // Web socket open flag
        bool                    ws_close_sent_;     // Web socket close sent

        std::list<SENDBUF *>    sendbuf_;           // Send buffers
        HTTPRequest             http_;              // HTTP request info
        WebsocketPacketHeader_t wshdr_;             // Websocket message header

        absolute_time_t         last_activity_;     // Time of last activity

        ClientHandle            handle_;            // Client handle
        static ClientHandle     next_handle_;       // Next handle
        static ClientHandle     nextHandle();       // Get next handle

        CLIENT() : pcb_(nullptr), closed_(true), websocket_(false), handle_(0) {}

    public:
        CLIENT(struct altcp_pcb *client_pcb)
         : pcb_(client_pcb), closed_(false), websocket_(false), ws_close_sent_(false)
          { activity(); handle_ = nextHandle(); }
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

        bool wasWSCloseSent() const { return ws_close_sent_; }
        void setWSCloseSent() { activity(); ws_close_sent_ = true; }
; 
        void queue_send(void *buffer, u16_t buflen, Allocation allocate);
        bool get_next(u16_t count, void **buffer, u16_t *buflen);
        bool more_to_send(bool quick=true) const { return sendbuf_.size() > 0; }
        void requeue(void *buffer, u16_t buflen);
        void acknowledge(int count);

        bool isIdle() const;
        void activity() { if (!ws_close_sent_) last_activity_ = get_absolute_time(); }

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

    void mark_for_close(struct altcp_pcb *client_pcb);
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

    Logger              default_logger_;            // Default logger
    Logger              *log_;                      // Active logger
    
    static WEB          *singleton_;                // Singleton pointer
    WEB();

    err_t send_buffer(struct altcp_pcb *client_pcb, void *buffer, u16_t buflen, Allocation allocate = ALLOC);
    err_t write_next(struct altcp_pcb *client_pcb);

    bool (*http_callback_)(WEB *web, ClientHandle client, const HTTPRequest &rqst, bool &close);
    void (*message_callback_)(WEB *web, ClientHandle client, const std::string &msg);
    void (*notice_callback_)(int state);
    void send_notice(int state) {if (notice_callback_) notice_callback_(state);}

public:
    /**
     * @brief   Get access to the web singleton object
     * 
     * @return  Pointer to WEB object
     */
    static WEB *get();

    /**
     * @brief   Initialize the web object
     * 
     * @return  true if successfully initialized
     */
    bool init();

    /**
     * @brief   Initiate a connection to a WiFi access point
     * 
     * @param   hostname    Name of this host
     * @param   ssid        WiFi service set identifier (access point name)
     * @param   password    WiFi access point password
     * 
     * @return  true if connection initiated successfully
     */
    bool connect_to_wifi(const std::string &hostname, const std::string &ssid, const std::string &password);

    /**
     * @brief   Change host name or WiFi connection and initiate reconnection
     * 
     * @param   hostname    Name of this host
     * @param   ssid        WiFi service set identifier (access point name)
     * @param   password    WiFi access point password
     * 
     * @return  true if connection initiated successfully
     */
    bool update_wifi(const std::string &hostname, const std::string &ssid, const std::string &password);

    /**
     * @brief   Getter for hostname
     */
    const std::string &hostname() const { return hostname_; }

    /**
     * @brief   Getter for WiFi SSID
     */
    const std::string &wifi_ssid() const { return wifi_ssid_; }

    /**
     * @brief   Getter for current IP address on WiFi
     */
    const std::string ip_addr() const { return ip4addr_ntoa(&wifi_addr_); }

    /**
     * @brief   Set callback for receipt of HTTP message
     * 
     * @param   cb          Pointer to callback function
     * 
     * @details Callback function takes the following parameters:
     * 
     *              -web    Pointer to the WEB object
     *              -client Handle to client connection
     *              -rqst   HTTP request object
     *              -close  boolean initially set to true. Called function can
     *                      set it to false to keep connection open after return
     * 
     *          -Callback to return true if it handled the request. If returns false,
     *          an error response is sent to client and connection is closed.
     */
    void set_http_callback(bool (*cb)(WEB *web, ClientHandle client, const HTTPRequest &rqst, bool &close)) { http_callback_ = cb; }

    /**
     * @brief   Set callback for receipt of websocket tet message
     * 
     * @param   cb          Pointer to callbck function
     * 
     * @details Callback function takes the following parameters:
     * 
     *              -web    Pointer to the WEB object
     *              -client Handle to client connection
     *              -msg    Payload of tet message
     */
    void set_message_callback(void(*cb)(WEB *web, ClientHandle client, const std::string &msg)) { message_callback_ = cb; }

    /**
     * @brief   Send a tet message to all connected websockets
     * 
     * @param   txt         Message t be sent
     */
    void broadcast_websocket(const std::string &txt);

    /**
     * @brief   Set callbck to receive notice of connection changes
     * 
     * @param   cb          Pointr to callbck function
     * 
     * @details Callback function takes the following parameter:
     * 
     *              -state  New connection state:
     *                      +STA_INITIALIZING   Initializing WiFi connection
     *                      +STA_CONNECTED      Connected to WiFi access point
     *                      +STA_DISCONNECTED   Disconnected from WiFI access point
     *                      +AP_ACTIVE          Device is acting as a WiFI access point
     *                      +AP_INACTIVE        Device has stopped acting as access point
     */
    void set_notice_callback(void(*cb)(int state)) { notice_callback_ = cb;}
    static const int STA_INITIALIZING = 101;
    static const int STA_CONNECTED = 102;
    static const int STA_DISCONNECTED = 103;
    static const int AP_ACTIVE = 104;
    static const int AP_INACTIVE = 105;

    /**
     * @brief   Send HTTP message
     * 
     * @param   client      Handle of client connection
     * @param   data        Pointr to data buffer
     * @param   datalen     Number of bytes to send from data buffer
     * @param   allocate    Type of buffer allocation to be performed:
     *                          -ALLOC  Buffer allocated and data copied
     *                          -STAT   data points to static data
     *                          -PREALL Buffer waas allocated by application
     *                                  and will be deleted by WEB object
     * 
     * @return  true if send queued successfully
     */
    bool send_data(ClientHandle client, const char *data, u16_t datalen, Allocation allocate=ALLOC);

    /**
     * @brief   Send a text message on websocket
     * 
     * @param   client      Handle of client connection
     * @param   data        Pointr to data buffer
     * @param   datalen     Number of bytes to send from data buffer
     * 
     * @return  true if send queued successfully
     */
    bool send_message(ClientHandle client, const std::string &message);

    /**
     * @brief   Modify client URL
     * 
     * @details Useful in POST routines that want to directly their corresponding GET
     * 
     * @param   client      Handle of client connection
     * @param   newurl      New URL string
     */
    void modifyURL(ClientHandle client, const std::string &newurl);

    /**
     * @brief   Enable this device to act as a WiFI access point
     * 
     * @param   minutes     Number of minutes for access point to be active
     * @param   name        Name (SSID) of access point
     * 
     * @details Access point can be used for connectins for the period specified
     *          typically to perform configuration of the device application. The
     *          access point will have a password of 12345678
     */
    void enable_ap(int minutes = 30, const std::string &name = "webapp") { ap_requested_ = minutes; ap_name_ = name; }

    /**
     * @brief   Test if access point is active
     */
    bool ap_active() const { return ap_active_ > 0; }

    /**
     * @brief   Scan for available WiFI access point names (SSID's)
     * 
     * @param   client      Handle of client connection (for passing to callback)
     * @param   callback    Callback function for completion of scan
     * @param   user_data   Data to be passed back to callbck function
     * 
     * @details The callback function takes the following parameters:
     * 
     *              -web        Pointer to the WEB object
     *              -client     Handle to client connection passed in scan_wifi call
     *              -ssids      map of SSID names and their signal strength
     *              -user_data  User data pointer passed in scan_wifi call
     */
    void scan_wifi(ClientHandle client, WiFiScan_cb callback, void *user_data = nullptr);

    /**
     * @brief   Set debug level
     * 
     * @param   level       Debug level (0=minimal, higher numbers for more detail)
     */
    void setDebug(int level) { debug_level_ = level; }

    /**
     * @brief   Set logger
     * 
     * @param   logger      Pointer to logger class to use
     */
    void setLogger(Logger *logger=nullptr) { if (logger) log_ = logger; else log_ = &default_logger_; }
};

#endif
