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
    else
    {
        data_ = new char[8];
        strncpy(data_, "{}", 8);
        load();
        ret = false;
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
    const char *str = jsonstr;
    char startchar = *str++;
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
        printf("JSON string '%.16s ...'does not start with '{' or '[' found 0x'%2.2x'\n", jsonstr, startchar);
        return 0;
    }
    int depth = 1;
    bool quoted = false;
    ret += 2;
    while (*str != '\0' && depth > 0)
    {
        if (!quoted && (*str == '{' || *str == '['))
        {
            ++depth;
            ret += 1;
        }
        else if (!quoted && (*str == '}' || *str == ']'))
        {
            --depth;
        }
        else if (!quoted && *str == ',')
        {
            ret += 1;
        }
        else if (!quoted && *str == '"')
        {
            quoted == true;
        }
        else if (quoted && *str == '"')
        {
            if (str [-1] != '\\')
            {
                quoted = false;
            }
        }
        ++str;
    }

    if (depth == 0)
    {
        while (*str != '\0' && isspace(*str))
        {
            ++str;
        }
    }

    if (*str != '\0' || depth > 0)
    {
        int ll = strlen(jsonstr) - 16;
        if (ll < 0) ll = 0;
        printf("Suspicious character '%c' after close of JSON object '... %.16s'\n", *str, &jsonstr[ll]);
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
