#pragma once

#ifndef __FILE_CHECKKIT_HPP__
#define __FILE_CHECKKIT_HPP__

#include <eosio/eosio.hpp>

namespace eosio { namespace danX {

}} //namespace danX, eosio

////////////////////////////////////////////////////////////

#define CHECK_FMT(pred, code, fmt, args...) ({                      \
    if(!pred){                                                      \
        char    msg[1024] = {};                                     \
        snprintf(msg, sizeof(msg)-1, "%d|%s %s %d|" fmt,            \
                 code, __FILE__, __FUNCTION__, __LINE__, ##args);   \
        eosio::check(pred, msg);                                    \
    }                                                               \
})

#define CHECK_STR(pred, code, str) ({                               \
    if(!pred){                                                      \
        char    msg[1024] = {};                                     \
        snprintf(msg, sizeof(msg)-1, "%d|%s %s %d|%s", code,        \
                 __FILE__, __FUNCTION__, __LINE__, str.c_str());    \
        eosio::check(pred, msg);                                    \
    }                                                               \
})

#define CHECK_SZ(pred, code, sz) ({                                 \
    if(!pred){                                                      \
        char    msg[1024] = {};                                     \
        snprintf(msg, sizeof(msg)-1, "%d|%s %s %d|%s", code,        \
                 __FILE__, __FUNCTION__, __LINE__, sz);             \
        eosio::check(pred, msg);                                    \
    }                                                               \
})

#endif  //__FILE_CHECKKIT_HPP__
