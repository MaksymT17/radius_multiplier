#pragma once
#include <stddef.h>
#include <vector>

enum MessageType : size_t
{
    HANDSHAKE = 0,
    HANDSHAKE_OK,
    HANDSHAKE_FAIL,
    SET_CONFIG,
    SET_CONFIG_OK,
    SET_CONFIG_FAIL,
    COMPARE_REQUEST,
    COMPARE_RESULT,
    COMPARE_FAIL,
    DISCONNECT,
    DISCONNECT_OK,
    DISCONNECT_FAIL
};

struct Message
{
    size_t id;
    MessageType type;
};

struct MessageSetConfig: public Message{
    int conf;
};

struct MessageCompareRequest : public Message{
    int f1,f2;
};
struct MessageCompareResult : public Message{
    int res;
};

