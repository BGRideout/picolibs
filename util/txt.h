#ifndef TXT_H
#define TXT_H

#include <string>
#include <vector>
#include <stdint.h>

class TXT
{
private:
    char            *buffer_;               // Buffer pointer
    uint32_t        bufsiz_;                // Buffer size
    uint32_t        endptr_;                // End of data offset

    char *expand(uint32_t offset, uint32_t amount);
    char *contract(uint32_t offset, uint32_t amount);
    
public:
    TXT() : buffer_(nullptr), bufsiz_(0), endptr_(0) {}
    TXT(uint32_t size) : buffer_(new char[size]), bufsiz_(size), endptr_(0) { if (size > 0) *buffer_ = 0; }
    TXT(const char *data, uint32_t datalen, uint32_t size=0);
    ~TXT() { if (buffer_) delete [] buffer_; }

    const char *data() const { return buffer_; }
    uint32_t datasize() const { return endptr_; }
    void release() { buffer_ = nullptr; bufsiz_ = 0; endptr_ = 0; }

    TXT &operator =(const char *assign);
    TXT &operator +=(const char *append);

    uint32_t find(const char *str) const;
    uint32_t insert(uint32_t offset, const char *str);
    void replace(uint32_t offset, uint32_t size, const char *replacement);

    bool substitute(const char *placeholder, const char *replacement);
    bool substitute(const char *placeholder, const std::string &replacement);
    bool substitute(const char *placeholder, int value);
    static bool substitute(std::string &target, const std::string &placeholder, const std::string &replacement);
    static bool substitute(std::string &target, const std::string &placeholder, int value);

    static void split(const std::string &src, const std::string &separators, std::vector<std::string> &tokens);
    static std::string join(const std::vector<std::string> &tokens, const std::string &separator);
    static void trim_back(std::string &str);
    static void trim_front(std::string &str);
};

#endif
