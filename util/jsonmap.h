//                  *****  JSONMap Class  *****

#ifndef JSONMAP_H
#define JSONMAP_H

#include <tiny-json.h>
#include <map>
#include <string>

/**
 * @class JSONMap
 * 
 * Manage a map of a single level JSON clsss definition.
 */
class JSONMap
{
private:
    json_t          *json_;             // JSON properties
    char            *data_;             // Data buffer

    JSONMap(const JSONMap &other);
    const JSONMap &operator = (const JSONMap &other);

    bool load();
    const json_t *findProperty(const char *name) const;

public:
    /**
     * @brief   Constructors
     * 
     * @param   json    Pointr to JSON string
     */
    JSONMap() : json_(nullptr), data_(nullptr) {}
    JSONMap(const char *json) : json_(nullptr), data_(nullptr) { loadString(json); }
    ~JSONMap() { clear(); }

    /**
     * @brief Load class from file
     * 
     * @param   filename    Name of file containing JSON data
     * 
     * @return  true if successful
     */
    bool loadFile(const char *filename);

    /**
     * @brief Load class from string
     * 
     * @param   str         String containing JSON data
     * 
     * @return  true if successful
     */
    bool loadString(const char *str);

    /**
     * @brief   Test if object has a property
     * 
     * @param   name    Property name
     * 
     * @return  true if property exists
     */
    bool hasProperty(const char *name) const;

    /**
     * @brief   Get the value of a property (string, integer, real) or boolean
     * 
     * @param   name    Property name
     * @param   defval  Default value if property not defined
     * 
     * @return  Property value or default value if not defined
     */
    const char *strValue(const char *name, const char *defVal=nullptr) const;
    int intValue(const char *name, int defVal=0) const;
    double realValue(const char *name, double defVal=0.0) const;
    bool boolValue(const char *name, bool defVal=false) const;

    /**
     * @brief   Create JSON string from a std::map of keys and values
     * 
     * @param   jmap    Map of keys and values
     * @param   str     String to receive JSON
     * 
     * @return  true if successful
     */
    typedef std::map<std::string, std::string> JMAP;
    static bool fromMap(const JMAP &jmap, std::string &str);

    /**
     * @brief   Return number of JSON items in a string
     * 
     * Useful in calculating the size of the JSON properties array
     * for the tiny-json json_create function. This function is not
     * retricted to single level JSON objects.
     * 
     * @param   jsonstr String contaiing JSON object description
     * 
     * @return  Number of items contained in the string
     */
    static int itemCount(const char *jsonstr);

    /**
     * @brief   Reset the object
     */
    void clear();
};

#endif
