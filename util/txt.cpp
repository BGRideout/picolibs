#include "txt.h"
#include <string.h>
#include <stdio.h>

TXT::TXT(const char *data, uint32_t datalen, uint32_t size)
{
    if (size <= datalen)
    {
        size = (datalen + 256) / 256 * 256;
    }
    bufsiz_ = size;
    buffer_ = new char[bufsiz_];
    endptr_ = datalen;
    strncpy(buffer_, data, datalen);
    buffer_[endptr_] = 0;
}

void TXT::expand(uint32_t offset, uint32_t amount)
{
    if (endptr_ + amount >= bufsiz_)
    {
        uint32_t newsz = endptr_ + amount + 2048;
        newsz /= 2048;
        newsz *= 2048;
        printf("TXT buffer expanded from %d to %d for epansion by %d from %d to %d\n", bufsiz_, newsz, amount, endptr_, endptr_ + amount);
        char *newbuf = new char[newsz];
        memcpy(newbuf, buffer_, endptr_ + 1);
        delete [] buffer_;
        buffer_ = newbuf;
        bufsiz_ = newsz;
    }

    char *src = buffer_ + endptr_;
    char *dst = buffer_ + endptr_ + amount;
    uint32_t nn = endptr_ - offset;
    for (uint32_t ii = 0; ii < nn; ii++)
    {
        *--dst = *--src;
    }
    endptr_ += amount;
    buffer_[endptr_] = 0;
}

void TXT::contract(uint32_t offset, uint32_t amount)
{
    char *src = buffer_ + offset + amount; 
    char *dst = buffer_ + offset;
    uint32_t nn = endptr_ - offset - amount;
    for (uint32_t ii = 0; ii < nn; ii++)
    {
        *dst++ = *src++;
    }
    endptr_ -= amount;
    buffer_[endptr_] = 0;
}

TXT &TXT::operator =(const char *assign)
{
    size_t la = strlen(assign);
    endptr_ = 0;
    expand(0, la);
    strncpy(buffer_, assign, la);
    buffer_[endptr_] = 0;
    return *this;
}

TXT &TXT::operator +=(const char *append)
{
    size_t la = strlen(append);
    char *dst = buffer_ + endptr_;
    expand(endptr_, la);
    strncpy(dst, append, la);
    buffer_[endptr_] = 0;
    return *this;
}

uint32_t TXT::find(const char *str) const
{
    char *s = strstr(buffer_, str);
    if (s)
    {
        return s - buffer_;
    }
    return std::string::npos;
}

uint32_t TXT::insert(uint32_t offset, const char *str)
{
    size_t li = strlen(str);
    expand(offset, li);
    memcpy(buffer_ + offset, str, li);
    buffer_[endptr_] = 0;
    return offset + li;
}

void TXT::replace(uint32_t offset, uint32_t size, const char *replacement)
{
    size_t lr = strlen(replacement);
    if (lr > size)
    {
        expand(offset + size, lr - size);
    }
    else if (lr < size)
    {
        contract(offset + lr, size -  lr);
    }
    for (uint32_t ii = 0; ii < lr; ii++)
    {
        buffer_[offset + ii] = replacement[ii];
    }
}

bool TXT::substitute(const char *placeholder, const char *replacement)
{
    bool ret = false;
    char *found = strstr(buffer_, placeholder);
    if (found)
    {
        uint32_t offset = found - buffer_;
        uint32_t size = strlen(placeholder);
        replace(offset, size, replacement);
        ret = true;
    }
    return ret;
}

bool TXT::substitute(const char *placeholder, const std::string &replacement)
{
    return substitute(placeholder, replacement.c_str());
}

bool TXT::substitute(const char *placeholder, int value)
{
    bool ret = false;
    char numbuf[16];
    int nn = snprintf(numbuf, sizeof(numbuf), "%d", value);
    if (nn < sizeof(numbuf))
    {
        ret = substitute(placeholder, numbuf);
    }
    return ret;
}

bool TXT::substitute(std::string &target, const std::string &placeholder, const std::string &replacement)
{
    bool ret = false;
    std::size_t ii = target.find(placeholder);
    if (ii != std::string::npos)
    {
        target.replace(ii, placeholder.length(), replacement);
        ret = true;
    }
    return ret;
}

bool TXT::substitute(std::string &target, const std::string &placeholder, int value)
{
    return substitute(target, placeholder, std::to_string(value));
}

void TXT::split(const std::string &src, const std::string &separators, std::vector<std::string> &tokens)
{
    tokens.clear();
    std::size_t i1 = 0;
    std::size_t i2 = src.find_first_of(separators, i1);
    if (i2 == std::string::npos)
    {
        tokens.push_back(src);
    }
    else
    {
        tokens.push_back(src.substr(i1, i2 - i1));
        do
        {
            i1 = i2 + 1;
            if (i1 < src.length())
            {
                i2 = src.find_first_of(separators, i1);
                tokens.push_back(src.substr(i1, i2 - i1));
            }
            else
            {
                i2 = std::string::npos;
                tokens.push_back("");
            }
        }
        while (i2 != std::string::npos);
    }
}

std::string TXT::join(const std::vector<std::string> &tokens, const std::string &separator)
{
    std::string ret;
    for (auto it = tokens.cbegin(); it != tokens.cend(); ++it)
    {
        if (!ret.empty()) ret += separator;
        ret += *it;
    }
    return ret;
}

void TXT::trim_back(std::string &str)
{
    std::size_t ii = str.find_last_not_of(" \t\r\n");
    if (ii != std::string::npos)
    {
        str.erase(ii + 1);
    }
    else
    {
        str.clear();
    }
}

void TXT::trim_front(std::string &str)
{
    std::size_t ii = str.find_first_not_of(" \t\r\n");
    str.erase(0, ii);
}
