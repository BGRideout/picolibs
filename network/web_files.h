#ifndef WEB_FILES_H
#define WEB_FILES_H

#include <string>
#include <stdint.h>
#include <map>
#include <utility>
/**
 * @class   WEB_FILES
 * 
 * This singleton class provides for WEB resources to be compiled into
 * the program using items in the CMakeLists.txt for the program like:
 * 
 *  # Link the web server library
 *  target_link_libraries(${PROJECT_NAME} PUBLIC bgr_webserver)
 * 
 *  ...
 * 
 *  # Add the static web resource files
 *  set(WEB_RESOURCE_FILES
 *      data/config.html
 *      data/config.js
 *      data/index.html
 *      data/webmouse.css
 *      data/webmouse.js
 *      data/favicon.ico)
 *	
 *  web_files(FILES ${WEB_RESOURCE_FILES} WEBSOCKET)
 * 
 * @see picolibs/network/CMakeLists.txt for web_files function
 *
 */
class WEB_FILES
{
private:
    std::map<std::string, std::pair<const char *, int> > files_;

    static WEB_FILES *singleton_;
    WEB_FILES();

public:
    /**
     * @brief   Get pointer to the WEB_FILES singleton
     */
    static WEB_FILES *get() { if (!singleton_) singleton_ = new WEB_FILES(); return singleton_; }

    /**
     * @brief   Get precompiled file static data
     * 
     * @param   name    File name
     * @param   data    Pointer to receive pointer to file data (null if not found)
     * @param   datalen Variable to receive length of file
     * 
     * @return  true if file name found
     */
    bool get_file(const std::string &name, const char * &data, uint16_t &datalen);
};

#endif
