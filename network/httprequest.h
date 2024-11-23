//                  *****  HTTPRequest Class  *****

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include "txt.h"
/**
 * @class   HTTPRequest
 * 
 * This class encapsulates an HTTP request received by the web server 
 */
class HTTPRequest
{
public:
    /**
     * @typedef PostData
     * 
     * @brief   std::multimap of POST data indexed by key
     */
    typedef std::multimap<std::string, const char *> PostData;

private:
    std::vector<std::string>        headers_;           // Header lines
    std::size_t                     body_offset_;       // Body offset in input string
    std::size_t                     body_size_;         // Body size
    char                            *body_;             // Pointer to body string
    PostData                        post_data_;         // POST data
    std::string                     user_data_;         // User data

    bool get_post();
    bool get_post_urlencoded();
    bool get_post_multipart(std::string &content_type);

public:
    /**
     * @brief   Constructor
     * 
     * @param   rqst    Reference to string containing request data
     * 
     * @see     parseRequest
     */
    HTTPRequest() : body_offset_(0), body_size_(0), body_(nullptr) {}
    HTTPRequest(std::string &rqst): body_offset_(0), body_size_(0), body_(nullptr) { parseRequest(rqst); }

    /**
     * @brief   Destructor
     */
    ~HTTPRequest() {}

    /**
     * @brief   Parse a request
     * 
     * @param   rqst    Reference to request string. This string must remain in scope
     *                  while this object is in use. The string is modified.
     * @param   parsePostData   If true, ny POST data will be parsed
     * 
     * @return  true if parse successful
     */
    bool parseRequest(std::string &rqst, bool parsePostData=true);

    /**
     * @brief   Parse POST data in request
     * 
     * This method must be called if not performed by parseRequest before accessing
     * any POST data in thee request. The original request strig will be modified.
     * 
     * @return  true if successful parse
     */
    bool parsePost();

    /**
     * @brief   Return the request type (GET or POST)
     * 
     * @return  GET or POST or a blank string if an error
     */
    std::string type() const;

    /**
     * @brief   Return the URL of the request
     */
    std::string url() const;

    /**
     * @brief   Return the path portion of the URL (up to any question mark)
     */
    std::string path() const;

    /**
     * @brief   Return the root portion of the path (up to the first period)
     */
    std::string root() const;

    /**
     * @brief   Return the file type of the path (after the period or 'html')
     */
    std::string filetype() const;

    /**
     * @brief   Return the value of an item in the query portion of thee URL
     * 
     * @param   key     Key of the item to fetch (portion before equal sign)
     * 
     * @return  Value of the query item (portion after equal sign) or empty string
     */
    std::string query(const std::string &key) const;
    
    /**
     * @brief   Replace the URL portion of the request
     * 
     * @param   newurl  New URL string
     */
    void setURL(const std::string &newurl);

    /**
     * @brief   Return the next occurrence of an HTTP header
     * 
     * @param   name    Header name
     * @param   from    Starting index of search
     * 
     * @return  Index at or after 'from' matching header name (-1 if not found)
     */
    int headerIndex(const std::string &name, int from=0) const;

    /**
     * @brief   Return a header by index
     * 
     * @param   index   Index of header
     * 
     * @return  std::pair with header name as first and value as second
     */
    std::pair<std::string, std::string> header(int index) const;

    /**
     * @brief   Return the value of the first header matching a name
     * 
     * @param   name    Header name
     * 
     * @return  Value of first header matching name or empty string if not found
     */
    std::string header(const std::string &name) const;

    /**
     * @brief   Return the value of a cookie
     * 
     * @param   name    Name of cookie
     * @param   defval  Value to return if cookie not found (defaule is empty string)
     */
    std::string cookie(const std::string &name, const std::string &defval=std::string()) const;

    /**
     * @brief   Return offset from original request string where HTML body begins
     */
    const std::size_t bodyOffset() const { return body_offset_; }

    /**
     * @brief   Return size of HTML body
     */
    const std::size_t bodySize() const { return body_size_; }

    /**
     * @brief   Return pointer to HTML body of request
     */
    const char *body() const { return body_; }

    /**
     * @brief   Return total size of request
     */
    const std::size_t size() const { return body_offset_ + body_size_; }

    /**
     * @brief   Return reference to POST data
     */
    const PostData &postData() const { return post_data_; }

    /**
     * @brief   Get pointer to value of POST item
     * 
     * @param   key     Key to POST item
     * 
     * @return  Pointer to value corresponding to key or null pointer
     */
    const char *postValue(const std::string &key) const;
    char *postValue(const std::string &key);

    /**
     * @brief   Return an array of POST values for a key
     * 
     * @param   key     key to POST data
     * @param   array   vector of pointers to array values
     * 
     * @return  size of returned array
     */
    int postArray(const std::string &key, std::vector<const char *> &array) const;

    /**
     * @brief   Print POST data to stdout
     */
    void printPostData() const;

    /**
     * @brief   Return URI decoded value of strig
     * 
     * @param   uri     URI encoded string
     * 
     * @return  decoded string
     */
    static std::string uri_decode(const std::string &uri);

    /**
     * @brief   Replace the header portio of an HTTP request
     * 
     * @param   rqst        HTTP request string
     * @param   newheader   New header string
     */
    static void replaceHeader(std::string &rqst, const std::string &newHeader = std::string("HTTP/1.1 200 OK\r\nContent-Type: text/html"));
    static void replaceHeader(TXT &rqst, const char *newHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html");

    /**
     * @brief   Replace the response header with one containing the content length
     * 
     * @param   rqst        HTTP message to be updated
     */
    static void setHTMLLengthHeader(std::string &rqst);
    static void setHTMLLengthHeader(TXT &rqst);

    /**
     * @brief   Return true if request string contains a complete HTTP request
     */
    bool isComplete() const { return body_ != nullptr; }

    /**
     * @brief   Reset the object
     */
    void clear() { headers_.clear(); body_offset_ = 0; body_size_ = 0; body_ = nullptr; post_data_.clear(); }

    /**
     * @brief   Return user data string
     */
    const std::string &userData() const { return user_data_; }

    /**
     * @brief   Set user data string
     * 
     * The user data is a string the application can use to pass data
     * along with the request. A typical use is in a POST processing
     * function that calls the GET function to return the response to
     * the caller but wants to send additional data to the GET method.
     */
    void setUserData(const std::string &data) { user_data_ = data; }
};

#endif
