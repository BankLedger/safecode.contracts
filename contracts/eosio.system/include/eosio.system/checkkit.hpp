#pragma once

#ifndef __FILE_CHECKKIT_HPP__
#define __FILE_CHECKKIT_HPP__

#include <eosio/eosio.hpp>

namespace eosio { namespace danX {

void __check(bool pred, const char* file, const char* func, int line, int errcode, const char* msg);

void check(bool pred, const char* file, const char* func, int line, int errcode, const char *fmt, ...);

void check(bool pred, const char* file, const char* func, int line, int errcode, const std::string& str);

}} //namespace danX, eosio

////////////////////////////////////////////////////////////

//(pred, errcode, fmt, args...)
//(pred, errcode, sz)
//(pred, errcode, str)
#define CHECK_MORE(pred, errcode, x, y...) ::eosio::danX::check(pred, __FILE__, __FUNCTION__, __LINE__, errcode, x, ##y);

#endif  //__FILE_CHECKKIT_HPP__
