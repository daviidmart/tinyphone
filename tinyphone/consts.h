#pragma once

#ifndef CONSTS_HEADER_FILE_H
#define CONSTS_HEADER_FILE_H

#include <string.h>

#define FOREGROUND_WHITE (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN)
#define FOREGROUND_YELLOW 14
#define SIP_ACCOUNT_NAME(username, domain)  username + "@" + domain

#define UNUSED_ARG(arg)  (void)arg

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define STRING_REMOVE_CHAR(str, ch) str.erase(std::remove(str.begin(), str.end(), ch), str.end())

#define REMOTE_CONFIG_URL "http://ekrt.it/tpconf"
#define REMOTE_CONFIG_URL_SECONDARY "https://raw.githubusercontent.com/voiceip/tinyphone/%s/config.json"

#define HEADER_SECURITY_CODE "X-SECURITY-CODE"

#define SECURITY_SALT "St$%C1aNrV$D"

#define SIP_LOG_FILE "tinyphone"
#define HTTP_LOG_FILE "tinyphone-http"

#endif