#pragma once

#ifndef __FILE_CHECKKIT_HPP__
#define __FILE_CHECKKIT_HPP__

#include <eosio/eosio.hpp>

namespace eosio { namespace danX {

void __check(bool pred, const char* file, const char* func, int line, int errcode, const char* msg)
{
    if(pred) { return; }

    char    buffer[1024] = {};
    snprintf(buffer, sizeof(buffer)-1, "%d|%s %s %d|%s",
                errcode, file, func, line, msg);
    eosio::check(pred, buffer);
}

void check(bool pred, const char* file, const char* func, int line, int errcode, const char *fmt, ...)
{
    if(pred) { return; }

    char    buffer[512] = {};

	va_list ap;
	va_start(ap, fmt);
    vsnprintf(buffer, sizeof(buffer)-1, fmt, ap);
    va_end(ap);

    __check(pred, file, func, line, errcode, buffer);
}


void check(bool pred, const char* file, const char* func, int line, int errcode, const std::string& str)
{
    if(pred) { return; }
    __check(pred, file, func, line, errcode, str.c_str());
}

}} //namespace danX, eosio

////////////////////////////////////////////////////////////

//(pred, errcode, fmt, args...)
//(pred, errcode, sz)
//(pred, errcode, str)
#define CHECK_MORE(pred, errcode, x, y...) ::eosio::danX::check(pred, __FILE__, __FUNCTION__, __LINE__, errcode, x, ##y);

#endif  //__FILE_CHECKKIT_HPP__
