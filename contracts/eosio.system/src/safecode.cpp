#include <eosio.system/eosio.system.hpp>

#include <eosio.system/printkit.hpp>

namespace eosiosystem {

   using eosio::check;

   template< typename TableIndex >
   auto system_contract::findByTxo( const TableIndex& tbl_index, const struct txo& txo )
   {
      bool found = false;
      auto itr_find_tx= tbl_index.lower_bound(txo.txid);
      auto itr_find_tx_up = tbl_index.upper_bound(txo.txid);
      for( ; itr_find_tx != itr_find_tx_up; ++itr_find_tx ) {
         if( itr_find_tx->v_txo.txid == txo.txid && itr_find_tx->v_txo.outidx == txo.outidx ) {
            found = true;
            break;
         }
      };
      return std::make_tuple(found, itr_find_tx);
   }

   void system_contract::vtxo2prod( const struct txo& txo, const name& producer )
   {
      DEBUG_PRINT_VAR(txo);
      DEBUG_PRINT_VAR(producer);

      auto found_ret = findByTxo(_sf5vtxo.get_index<"by3txid"_n>(), txo);
      auto found = std::get<0>(found_ret);
      check( found == false, "error, txo has exists at table vtxo4sc" );

      _sf5vtxo.emplace(get_self(), [&]( auto& row ) {
         row.v_id    = _sf5vtxo.available_primary_key();
         row.v_txo   = txo;
         row.v_bp    = producer;
         row.v_tp    = eosio::current_time_point();
         row.v_weight= 0.0;
      });
   }

   void system_contract::sf5regprod( const struct txo& txo, const struct sfreginfo& sfri )
   {

   }

   void system_contract::sf5unregprod( const struct txo& txo )
   {

   }

   void system_contract::checksign( const eosio::checksum256& digest, const eosio::signature& sig, const eosio::public_key& pubkey )
   {
      assert_recover_key( digest, sig, pubkey );
   }

} /// namespace eosiosystem
