//                  *****  WEB_FILES::send_websocket  *****

#include "web_files.h"

bool WEB_FILES::send_websocket_js(WEB *web, ClientHandle client, HTTPRequest &rqst, bool &close, const std::string &wspath)
{
    bool ret = false;
    std::string url = rqst.path();
    if (url.substr(1) == "websocket.js")
    {
        const char *data;
        u16_t datalen;
        if (url.length() > 0 && WEB_FILES::get()->get_file(url.substr(1), data, datalen))
        {
            if (wspath.empty() || wspath == "/ws/")
            {
                ret = web->send_data(client, data, datalen, WEB::STAT);
                close = !ret;
            }
            else
            {
                TXT js(data, datalen, datalen + 2 * wspath.length());
                while (js.substitute("/ws/", "<ws>")); // In case wspath constains /ws/
                while (js.substitute("<ws>", wspath));

                uint32_t ii = js.find("Content-Length:") + 16;
                uint32_t ll = strstr(js.data() + ii, "\r\n") - (js.data() + ii);
                uint32_t jj = js.find("\r\n\r\n") + 4;
                char numbuf[16];
                snprintf(numbuf, sizeof(numbuf), "%d", js.datasize() - jj);
                js.replace(ii, ll, numbuf);
                ret = web->send_data(client, js.data(), js.datasize(), WEB::PREALL);
                js.release();
                close = !ret;
            }
        }
    }
    return ret;
}

