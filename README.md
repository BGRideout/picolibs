# Raspberry PICO Application Library

This is a set of C++ libraries useful in developing applications to be run on the Raspberry
pico and pico-w series microcontrollers.

## Libraries
The libraries are included in a program's CMakeLists.txt file with the directives (in this
example picolibs is in a directory parallel to the application's and pico-filesystem and
tiny-json are parallel to the pico-sdk):

```
# Add libraries
add_subdirectory(${PICO_SDK_PATH}/../pico-filesystem pfs)
add_subdirectory(${PICO_SDK_PATH}/../tiny-json tiny-json)

# Add the utility libraries
add_subdirectory(../picolibs picolibs)
```

The pico-filesystem and tiny-json libraries are needed by some of the classes. They may be
omitted if not being used. If used, the application's CMakeLists.txt file must include them
in a target_link_libraries directive (flash_filesystem tiny-json).

The libraries are divided into three groups:

### Utility

This group includes general purpose support classes for:

1.  Library **bgr_util**
    1.  Control of LED's (onboard and connected to GPIO)
    2.  Receiving input from push buttons via GPIO
    3.  Control of PWM GPIO outputs
    4.  Control of servo motors through PWM GPIO outputs
    5.  Output of sound by GPIO output
    6.  Manage dynamic text strings
    7.  Control and log debug output
    8.  Monitor battery power
2.  Library **bgr_json**
    1.  Support functions for managing JSON data along with tiny-json library

### Network

This group provides networking services on the pico-w

1.  Library **bgr_webserver**
    1.  Implementation of a web server

### IR (Infrared)

This group provides sending and receiving IR data

1.  Library **bgr_ir**
    1.  Base classes for sending and receiving data via GPIO
2.  Library **bgr_ir_protocols**
    1.  Implementations of various IR protocols

##  Third Party Libraries

### pico-filesystem

https://github.com/Memotech-Bill/pico-filesystem

Used when persistent storage on the microSD card is used.  Class FileLogger requires
this library (flash-filesystem) to be used and for the application to initialize the
filesystem on startup with code like:

```
//  File system definition
#define ROOT_OFFSET 0x110000
#if ENABLE_BLE
#include <pico/btstack_flash_bank.h>
#define ROOT_SIZE   (PICO_FLASH_SIZE_BYTES - ROOT_OFFSET - PICO_FLASH_BANK_TOTAL_SIZE)       // Leave 8K for bluetooth
#else
#define ROOT_SIZE   (PICO_FLASH_SIZE_BYTES - ROOT_OFFSET)
#endif

...

    struct pfs_pfs *pfs;
    struct lfs_config cfg;
    ffs_pico_createcfg (&cfg, ROOT_OFFSET, ROOT_SIZE);
    pfs = pfs_ffs_create (&cfg);
    pfs_mount (pfs, "/");
```

#### License
The software in the "littlefs" and "fatfs" folders comes from third parties and 
is subject to the licenses therein.

All other software in this folder is Copyright 2023 Memotech-Bill

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this 
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
may be used to endorse or promote products derived from this software without 
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

### tiny-json

https://github.com/rafagafe/tiny-json

For applications that use data in the JSON format, the tiny-json library can
be used. None of the libraries in this package make direct use of JSON data
but the classes in the bgr-json library provide additional functionality on
top off tiny-json.

#### License
MIT License

Copyright (c) 2018 Rafa Garcia

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

### dhcpserver

https://github.com/micropython/micropython/tree/master/shared/netutils

The DHCP server supplies IP addresses to clients when the WEB class is running
in access point (AP) mode.  This module is taken directly from the micropython
source.

#### License
This file is part of the MicroPython project, http://micropython.org/

The MIT License (MIT)

Copyright (c) 2018-2019 Damien P. George

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

### picow-websocket

https://github.com/samjkent/picow-websocket

The WS class packs and unpacks websocket messages

#### License
None stated
