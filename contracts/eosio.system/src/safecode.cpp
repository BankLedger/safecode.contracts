#include <eosio.system/eosio.system.hpp>

#include <eosio.system/printkit.hpp>

namespace eosiosystem {

   using eosio::check;

   template< typename TableIndex >
   auto system_contract::findByTxo( const TableIndex& tbl_index, const struct txokey& txokey )
   {
      bool found = false;
      auto itr_find_tx= tbl_index.lower_bound(txokey.txid);
      auto itr_find_tx_up = tbl_index.upper_bound(txokey.txid);
      for( ; itr_find_tx != itr_find_tx_up; ++itr_find_tx ) {
         if( itr_find_tx->index_by_txid() == txokey.txid && itr_find_tx->get_tx_outidx() == txokey.outidx ) {
            found = true;
            break;
         }
      };
      return std::make_tuple(found, itr_find_tx);
   }

   // void system_contract::sf5vote( const struct txo& txo, const name& producer )
   // {
   //    DEBUG_PRINT_VAR(txo);
   //    DEBUG_PRINT_VAR(producer);

   //    auto found_ret = findByTxo(_sf5vtxo.get_index<"by3txid"_n>(), txo);
   //    auto found = std::get<0>(found_ret);
   //    check( found == false, "error, txo has exists at table vtxo4sc" );

   //    _sf5vtxo.emplace(get_self(), [&]( auto& row ) {
   //       row.v_id    = _sf5vtxo.available_primary_key();
   //       row.v_txo   = txo;
   //       row.v_bp    = producer;
   //       row.v_tp    = eosio::current_time_point();
   //       row.v_weight= 0.0;
   //    });
   // }

   void system_contract::sf5regprod( const struct txo& rptxo, const struct sfreginfo& sfri )
   {
      auto found_ret = findByTxo(_sf5producers.get_index<"by3txid"_n>(), rptxo.key);
      auto found = std::get<0>(found_ret);
      check( found == false, "error, rptxo has exists at table sf5producers" );
      check( rptxo.type == 1, "error, sfri.type must is 1(non-locked assert)" );
      check( sfri.dvdratio >= 0 && sfri.dvdratio <= 100, "error, sfri.dvdratio must be in range [0, 100]" );
      assert_recover_key( sfri.infohash, sfri.sc_sig, sfri.sc_pubkey );

      _sf5producers.emplace(get_self(), [&]( auto& row ) {
         row.prmrid  = _sf5producers.available_primary_key();
         row.rptxo   = rptxo;
         row.ri      = sfri;
         row.enable  = true;
      });
   }

   void system_contract::sf5vote( const struct txokey& rptxokey, const struct txo& vtxo )
   {

   }

   void system_contract::sf5unregprod( const struct txokey& rptxokey )
   {

   }

   void system_contract::checksign( const eosio::checksum256& digest, const eosio::signature& sig, const eosio::public_key& pubkey )
   {
      assert_recover_key( digest, sig, pubkey );
   }

} /// namespace eosiosystem
