/**
 * \file websocket.js
 * \verbinclude websocket.js
 * 
 * This package provides functions to communicate with a webserver
 * using websockets. It is included by an HTML document along with
 * the Javascript file for the document's application logic. That
 * script then enables websocket communication usng a sequence:
 * 
 * \code
document.addEventListener('DOMContentLoaded', function()
{
  document.addEventListener('ws_state', ws_state_change);
  document.addEventListener('ws_message', process_ws_message);
  ...
  openWS();
});
 * \endcode
 *
 * The **ws_state** event is called when the open/closed state
 * of the websocket changes. The current state can be obtained
 * by inspecting the property *evt.detail.obj['open']* of the
 * event (evt).
 * 
 * The **ws_message** event is called when a message is received
 * on the websocket. The message text is available from the
 * property *evt.detail.message* of the evet (evt).
 */

/// \cond DO_NOT_DOCUMENT

// Websocket variables
var ws = undefined;             // WebSocket object
var timer_ = undefined;         // Reconnect timer
var conchk_ = undefined;        // Connection check timer
var opened_ = false;            // Connection opened flag
var closed_ = true;             // Websocket closed flag
var suspended_ = false;         // I/O suspended flag

/// \endcond

/**
 * @brief Open connection to websocket
 * 
 * Once opened, the connection will automtically continue
 * to be reopened in case of errors or disconnects.
 */
function openWS()
{
    if ('WebSocket' in window)
    {
        closed_ = false;
        if (typeof ws === 'object')
        {
            console.log('Closing old connection');
            ws.close();
            ws = undefined;
        }
        setWSOpened(false);
        let url = 'ws://' + location.host + '/ws/';
        if (location.protocol == 'https:')
        {
            url = 'wss://' + location.host + '/ws/';
        }
        console.log(url);
        ws = new WebSocket(url);
        if (conchk_ === undefined)
        {
            conchk_ = setTimeout(checkOpenState, 250);
        }

        ws.onopen = function(ev)
        {
            console.log('ws open state ' + ws.readyState);
            checkOpenState();
        };

        ws.onclose = function()
        {
            console.log('ws closed');
            setWSOpened(false);
            ws = undefined;
            if (conchk_ !== undefined)
            {
                clearTimeout(conchk_);
                conchk_ = undefined;
            }
        };

        ws.onmessage = function(evt)
        {
            let obj = new Object;
            obj['open'] = opened_;
            const mevt = new CustomEvent('ws_message', { detail: { message: evt.data } });
            document.dispatchEvent(mevt);
        };

        ws.onerror = function(error)
        {
            console.log('WS error:');
            console.error(error);
            if (ws !== undefined)
            {
                ws.close();
                setWSOpened(false);
                retryConnection();
            }
        };

        //  Set interval to reconnect and suspend when hidden
        if (typeof timer_ === 'undefined')
        {
            timer_ = setInterval(retryConnection, 10000);
            if (typeof document.hidden != 'undefined')
            {
                document.onvisibilitychange = function()
                {
                    suspended_ = document.hidden;
                    if (!suspended_)
                    {
                        retryConnection();
                    }
                    else
                    {
                        if (ws !== undefined)
                        {
                            ws.close();
                        }
                        setWSOpened(false);
                    }
                };
            }
        }
    }
}

/**
 * @brief   Test if websocket is open
 * @returns true if open
 */
function isWSOpen()
{
    return opened_;
}

/**
 * @brief   Close websocket
 * 
 * Closes the websocket and stops periodic reconnection
 */
function closeWS()
{
    closed_ = true;
    if (typeof ws === 'object')
    {
        console.log('Closing websocket');
        ws.close();
    }
    setWSOpened(false);
}

/**
 * @brief   Send a text message to the websocket
 * @param   msg Text string to be sent
 */
function sendToWS(msg)
{
    if (typeof ws === 'object')
    {
        ws.send(msg);
        //console.log('Sent: ' + msg);
    }
    else
    {
        alert('No connection to remote! - Refresh browser and try again.');
    }
}

/// \cond DO_NOT_DOCUMENT

function checkOpenState(retries = 0)
{
    clearTimeout(conchk_);
    conchk_ = undefined;

    if (typeof ws == 'object')
    {
        if (ws.readyState == WebSocket.OPEN)
        {
            if (!opened_)
            {
                setWSOpened(true);
                console.log('ws connected after ' + (retries * 250) + ' msec');
            }
        }
    }
}

function setWSOpened(state)
{
  if (state != opened_)
  {
    opened_ = state;
    let obj = new Object;
    obj['open'] = opened_;
    const evt = new CustomEvent('ws_state', { detail: { obj: obj } });
    document.dispatchEvent(evt);
  }
}

function retryConnection()
{
    if (!suspended_ && !closed_)
    {
        if (typeof ws !== 'object')
        {
            openWS();
        }
     }
}

/// \endcond
