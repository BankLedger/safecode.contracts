#pragma once

#include <eosio/asset.hpp>

namespace eosiosystem {

   using eosio::asset;
   using eosio::block_timestamp;
   using eosio::check;
   using eosio::const_mem_fun;
   using eosio::datastream;
   using eosio::indexed_by;
   using eosio::name;
   using eosio::same_payer;
   using eosio::symbol;
   using eosio::symbol_code;
   using eosio::time_point;
   using eosio::time_point_sec;
   using eosio::unsigned_int;

   static constexpr int      block_interval_ms = 3000;
   static constexpr int      block_interval_us = block_interval_ms*1000;
   static constexpr int      block_interval_s = block_interval_ms/1000;

   static constexpr uint32_t seconds_per_year      = 52 * 7 * 24 * 3600;
   static constexpr uint32_t seconds_per_day       = 24 * 3600;
   static constexpr int64_t  useconds_per_year     = int64_t(seconds_per_year) * 1000'000ll;
   static constexpr int64_t  useconds_per_day      = int64_t(seconds_per_day) * 1000'000ll;
   static constexpr uint32_t blocks_per_day        = seconds_per_day / block_interval_s;

   static constexpr int64_t  min_activated_stake   = 150'000'000'0000;
   static constexpr int64_t  ram_gift_bytes        = 1400;
   static constexpr int64_t  min_pervote_daily_pay = 100'0000;
   static constexpr double   continuous_rate       = 0.04879;          // 5% annual rate
   static constexpr int64_t  inflation_pay_factor  = 5;                // 20% of the inflation
   static constexpr int64_t  votepay_factor        = 4;                // 25% of the producer pay
   static constexpr uint32_t refund_delay_sec      = 3 * seconds_per_day;

} /// eosiosystem
