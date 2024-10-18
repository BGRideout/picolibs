//                  *****  JSONMap Implementation  *****

#include "jsonmap.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

bool JSONMap::loadFile(const char *filename)
{
    clear();
    bool ret = false;
    FILE *f = fopen(filename, "r");
    if (f)
    {
        struct stat sb;
        stat(filename, &sb);
        data_ = new char[sb.st_size + 1];
        fread(data_, sb.st_size, 1, f);
        data_[sb.st_size] = 0;
        fclose(f);
        ret = load();
    }
    return ret;
}

bool JSONMap::loadString(const char *str)
{
    clear();
    int len = strlen(str);
    data_ = new char[len + 1];
    strncpy(data_, str, len + 1);
    return load();
}

bool JSONMap::load()
{
    int jsz = itemCount(data_) + 1;
    json_ = new json_t[jsz];
    json_t const* json = json_create(data_, json_, jsz);
    bool ret = json != nullptr;
    if (!ret)
    {
        delete [] data_;
        data_ = new char[8];
        strncpy(data_, "{}", 8);
        json = json_create(data_, json_, jsz);
    }
    return ret;
}

const json_t *JSONMap::findProperty(const char *name) const
{
    return json_getProperty(json_, name);
}

bool JSONMap::hasProperty(const char *name) const
{
    return findProperty(name) != nullptr;
}

const char *JSONMap::strValue(const char *name, const char *defVal) const
{
    const char *ret = defVal;
    const json_t *prop = findProperty(name);
    if (prop)
    {
        ret = json_getValue(prop);
    }
    return ret;
}

int JSONMap::intValue(const char *name, int defVal) const
{
    int ret = defVal;
    const json_t *prop = findProperty(name);
    if (prop)
    {
        ret = json_getInteger(prop);
    }
    return ret;
}

double JSONMap::realValue(const char *name, double defVal) const
{
    double ret = defVal;
    const json_t *prop = findProperty(name);
    if (prop)
    {
        ret = json_getReal(prop);
    }
    return ret;
}

bool JSONMap::fromMap(const JMAP &jmap, std::string &str)
{
    bool ret = false;
    str.clear();
    if (jmap.size() > 0)
    {
        ret = true;
        std::string sep("{\"");
        for (auto it = jmap.cbegin(); it != jmap.cend(); ++it)
        {
            str += sep + it->first + "\":\"" + it->second + "\"";
            sep = ",\"";
        }
        str += "}";
    }
    return ret;
}

int JSONMap::itemCount(const char *jsonstr)
{
    int ret = 0;
    char startchar = *jsonstr++;
    char endchar = '\0';
    if (startchar == '{')
    {
        endchar = '}';
    }
    else if (startchar == '[')
    {
        endchar = ']';
    }
    else
    {
        printf("JSON string does not start with '{' or '[' found 0x'%2.2x'\n", startchar);
        return 0;
    }
    int depth = 1;
    bool quoted = false;
    ret += 2;
    while (*jsonstr != '\0' && depth > 0)
    {
        if (!quoted && (*jsonstr == '{' || *jsonstr == '['))
        {
            ++depth;
            ret += 1;
        }
        else if (!quoted && (*jsonstr == '}' || *jsonstr == ']'))
        {
            --depth;
        }
        else if (!quoted && *jsonstr == ',')
        {
            ret += 1;
        }
        else if (!quoted && *jsonstr == '"')
        {
            quoted == true;
        }
        else if (quoted && *jsonstr == '"')
        {
            if (jsonstr [-1] != '\\')
            {
                quoted = false;
            }
        }
        ++jsonstr;
    }

    if (depth == 0)
    {
        while (*jsonstr != '\0' && isspace(*jsonstr))
        {
            ++jsonstr;
        }
    }

    if (*jsonstr != '\0' || depth > 0)
    {
        printf("Suspicious character '%c' after close of JSON object\n", *jsonstr);
    }
    return ret;
}

void JSONMap::clear()
{
    if (data_) delete [] data_;
    data_ = nullptr;
    if (json_) delete [] json_;
    json_ = nullptr;
}