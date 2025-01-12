#ifndef TXT_H
#define TXT_H

#include <string>
#include <vector>
#include <stdint.h>

/**
 * @class   TXT
 * 
 * This class provides some functionality similar to the std::string class but
 * has different memory management useful to the WEB class.
 * 
 * There are also some specialized methods for std::string onjects
 */
class TXT
{
private:
    char            *buffer_;               // Buffer pointer
    uint32_t        bufsiz_;                // Buffer size
    uint32_t        endptr_;                // End of data offset

    char *expand(uint32_t offset, uint32_t amount);
    char *contract(uint32_t offset, uint32_t amount);
    
public:
    /**
     * @brief   Construct an empty TXT object
     */
    TXT() : buffer_(nullptr), bufsiz_(0), endptr_(0) {}

    /**
     * @brief   Construct a TXT object with preallocated buffer
     * 
     * @param   size    Size of preallocated buffer
     */
    TXT(uint32_t size) : buffer_(new char[size]), bufsiz_(size), endptr_(0) { if (size > 0) *buffer_ = 0; }

    /**
     * @brief   Construct a TXT object with an initial string
     * 
     * @param   data    Pointr to initial string
     * @param   datalen Length of initial string
     * @param   size    Initial size of data buffer (if larger than datalen)
     */
    TXT(const char *data, uint32_t datalen, uint32_t size=0);

    /**
     * @brief   Destructor
     */
    ~TXT() { if (buffer_) delete [] buffer_; }

    /**
     * @brief   Return pointer to string
     */
    const char *data() const { return buffer_; }
    char *data() { return buffer_; }

    /**
     * @brief   Return string length
     */
    uint32_t datasize() const { return endptr_; }

    /**
     * @brief   Release ownership of string
     * 
     * This method resets the TXT object to empty but does not release the
     * buffer allocated to contain the string. The string should have been
     * accessed by the data() method before calling this method and it is
     * then the responsibilty of the application to delete[] it.  This is
     * useful with the WEB class's send_data() method thth the Allocation
     * parametr set to WEB::PREALL where that class will free the buffer
     * after it has been sent
     */
    void release() { buffer_ = nullptr; bufsiz_ = 0; endptr_ = 0; }

    /**
     * @brief   Assign new value to the string
     */
    TXT &operator =(const char *assign);
    TXT &operator =(const std::string &assign);

    /**
     * @brief   Append data to the string
     */
    TXT &operator +=(const char *append);
    TXT &operator +=(const std::string &append);

    /**
     * @brief   Find a substring
     * 
     * @param   str     Substring to be found
     * 
     * @return  Offset in string or std::string::npos if not found
     */
    uint32_t find(const char *str) const;

    /**
     * @brief   Insert substring
     * 
     * @param   offset  Offset into contained string to begin insertion
     * @param   str     String to be inserted
     * @param   len     Length of string (use strlen(str) if zero)
     * 
     * @return  Offset to next character after end of inserted string
     */
    uint32_t insert(uint32_t offset, const char *str, size_t len=0);

    /**
     * @brief   Replace characters in string
     * 
     * @param   offset      Beginning offset of characters to be replaced
     * @param   size        Number of characters to replace
     * @param   replacement String to be inserted in place of indicated characters
     */
    void replace(uint32_t offset, uint32_t size, const char *replacement);

    /**
     * @brief   Replace a substring
     * 
     * @param   target      String on which replacement is to be performed rather than TXT
     * @param   placeholder String to be found and replaced
     * @param   replacement String to replace the found placeholder
     * @param   value       Integer value to be converted to replacement string
     * 
     * @return  true if replaced
     */
    bool substitute(const char *placeholder, const char *replacement);
    bool substitute(const char *placeholder, const std::string &replacement);
    bool substitute(const char *placeholder, int value);
    static bool substitute(std::string &target, const std::string &placeholder, const std::string &replacement);
    static bool substitute(std::string &target, const std::string &placeholder, int value);

    /**
     * @brief   Split string into tokens
     * 
     * @param   src         Source string to be split
     * @param   separators  String of separators as in std::string::find_first_of()
     * @param   tokens      Vector to receive split substrings
     * 
     * Multiple adjacent separators each create a separate empty token
     */
    static void split(const std::string &src, const std::string &separators, std::vector<std::string> &tokens);

    /**
     * @brief   Join tokens into a string
     * 
     * @param   tokens      Vector of strings
     * @param   separator   String to be inserted between tokens
     * 
     * @return  Joined string
     */
    static std::string join(const std::vector<std::string> &tokens, const std::string &separator);

    /**
     * @brief   Trim whitespace from end of string in place
     */
    static void trim_back(std::string &str);

    /**
     * @brief   Trim whitespacee from beginnning of string in place
     */
    static void trim_front(std::string &str);
};

#endif
