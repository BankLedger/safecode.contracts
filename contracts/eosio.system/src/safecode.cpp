#include <eosio.system/eosio.system.hpp>

#include <eosio.system/printkit.hpp>

namespace eosiosystem {

   using eosio::check;

   void system_contract::vtxo2prod( const struct txo& txo, const name& producer )
   {
      DEBUG_PRINT_VAR(txo);
      DEBUG_PRINT_VAR(producer);
   }

} /// namespace eosiosystem
