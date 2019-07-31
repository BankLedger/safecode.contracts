
#include <eosio.system/eosio.system.core.hpp>

namespace eosiosystem {

   struct [[eosio::contract("eosio.system")]] rewards4sc {

      struct year_rewards {
         uint8_t  ynr;        //base from 0
         uint64_t amount;
      };

      block_timestamp      base_bt;
      year_rewards         yrewards_list[2];

      void print() const
      {
         eosio::print("base_bt(sec) = "); eosio::print(base_bt.to_time_point().sec_since_epoch()); eosio::print("\n");
         eosio::print("yrewards_list = ");
         for( int i=0; i<sizeof(yrewards_list); ++i ) {
            eosio::print_f("[%]%|", yrewards_list[i].ynr, yrewards_list[i].amount);
         }
         eosio::print("\n");
      }


      uint64_t get_amount(const block_timestamp& bt1, const block_timestamp& bt2);
   };

   typedef eosio::singleton< "rewards4sc"_n, rewards4sc >   rewards4sc_singleton;



   /* 
   struct address {  //main-chain account obj

   };

   struct txo {
      checksum256       txid;    //txid at safe chain
      uint8_t           outidx;  //out-index of utxo tx's vout array
      uint64_t          quality;
      address           from;
      uint8_t           type;    //masternode-locked, non-masternode-locked, liquid
   };

   struct [[eosio::contract("eosio.system")]] vtxo4sc {
      uint64_t          v_id;
      txo               v_txo;
      name              v_bp;
      block_timestamp   v_bt;
   };

   struct [[eosio::contract("eosio.system")]] addr2account {
      address           addr;
      name              account;
   };
   */

}
