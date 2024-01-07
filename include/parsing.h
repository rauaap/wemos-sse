#ifndef PARSING_H
#define PARSING_H

struct Request {
    char method[7];
    char endpoint[20];
    char body[500];
};

Request parseRequest(const char* requestString);

#endif
