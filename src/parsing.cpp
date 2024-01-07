#include <string.h>
#include "parsing.h"

//enum Method {
//    GET,
//    POST,
//    DELETE,
//    FAIL
//};

Request parseRequest(const char* requestString) {
    //constexpr char* methodStrs[3] = {
    //    (char*)"GET", (char*)"POST", (char*)"DELETE"
    //};

    Request request;
    char* curPos = (char*)requestString;
    char* newPos;

    auto copyUntilSep = [&curPos, &newPos] (char* buf, uint16_t maxLen, const char* sep, uint8_t lenSep) {
        newPos = strstr(curPos, sep);
        uint16_t len = newPos - curPos;
        len = (maxLen < len ? maxLen : len);
        strncpy(buf, curPos, len);
        buf[len] = '\0';
        curPos = newPos + lenSep;
    };

    copyUntilSep(request.method, 7, " ", 1);
    copyUntilSep(request.endpoint, 20, " ", 1);

    const char sep[] = "\r\n\r\n";
    curPos = strstr(newPos, sep) + 4;
    copyUntilSep(request.body, 500, sep, 4);

    return request;
    //request.method = ([&]() {
    //    int i = 0;
    //    for (; i < 3; i++) {
    //        if (strncmp(methodStrs[i], methodBuf, 7) == 0)
    //            break;
    //    }
    //    return (Method)i;
    //})(); 
}
