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

   void system_contract::sf5regprod( const struct sf5key& sfkey, const struct txokey& rptxokey, const struct sfreginfo& ri, double sf_vtotal )
   {
      auto found_ret = findByTxo(_sf5producers.get_index<"by3txid"_n>(), rptxokey);
      auto found = std::get<0>(found_ret);
      check( found == false, "error, rptxo has exists at table sf5producers" );
      check( ri.dvdratio >= 0 && ri.dvdratio <= 100, "error, ri.dvdratio must be in range [0, 100]" );

      _sf5producers.emplace(get_self(), [&]( auto& row ) {
         row.prmrid     = _sf5producers.available_primary_key();
         row.rptxokey   = rptxokey;
         row.ri         = ri;
         row.owner      = name(0);
         row.sf_vtotal  = sf_vtotal;
         row.vtotal     = 0;
         row.enable     = true;
      });
   }

   void system_contract::sf5unregprod( const struct sf5key& sfkey, const struct txokey& rptxokey )
   {

   }

   void system_contract::sf5vote( const struct sf5key& sfkey, const struct txokey& rptxokey, const struct txo& vtxo )
   {

   }

   void system_contract::sf5unvote( const struct sf5key& sfkey, const struct txokey& vtxokey )
   {

   }

   void system_contract::sf5bindaccnt( const struct sf5key& sfkey, const struct sfaddress& sfaddr, const name& account )
   {

   }

   void system_contract::sf5setnext( const struct sf5key& sfkey )
   {

   }

   void system_contract::regproducer2( const struct txokey& rptxokey, const name& account, const signature& newsig )
   {

   }

   void system_contract::sc5vote( const name& voter, const name& producer )
   {

   }

   void system_contract::claim4prod( const name& producer )
   {

   }

   void system_contract::claim4vote( const name& voter )
   {

   }

   void system_contract::checksign( const eosio::checksum256& digest, const eosio::signature& sig, const eosio::public_key& pubkey )
   {
      assert_recover_key( digest, sig, pubkey );
   }

} /// namespace eosiosystem
