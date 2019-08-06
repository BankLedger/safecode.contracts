
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

   ////////////////////////////////////////////////////////

   struct address {  //main-chain account obj
      std::string       str_addr;

      void print() const
      {
         eosio::print("[struct address]obj:\n");
         eosio::print("\tstr_addr = "); eosio::print(str_addr); eosio::print("\n");
         eosio::print("[struct address]end of obj\n");
      }
   };

   struct txo {
      checksum256       txid;    //txid at safe chain
      uint8_t           outidx;  //out-index of utxo tx's vout array, base from 0
      uint64_t          quantity;
      address           from;
      uint8_t           type;    //masternode-locked, non-masternode-locked, liquid
      time_point        tp;      //when gen transaction

      void print() const
      {
         eosio::print("[struct txo]obj:\n");
         eosio::print("\ttxid = "); eosio::print(txid); eosio::print("\n");
         eosio::print("\toutidx = "); eosio::print(outidx); eosio::print("\n");
         eosio::print("\tquantity = "); eosio::print(quantity); eosio::print("\n");
         eosio::print("\tfrom = "); eosio::print(from.str_addr); eosio::print("\n");
         eosio::print("\ttype = "); eosio::print(type); eosio::print("\n");
         eosio::print("\ttp(sec) = "); eosio::print(tp.sec_since_epoch()); eosio::print("\n");
         eosio::print("[struct txo]end of obj\n");
      }
   };

   ////////////////////////////////////////////////////////

   struct [[eosio::contract("eosio.system")]] vtxo4sc {
      uint64_t          v_id;       //auto increament
      txo               v_txo;
      name              v_bp;
      time_point        v_tp;       //when voting
      double            v_weight;

      uint64_t primary_key() const
      {
         return (v_id);
      }

      checksum256 get_txid() const
      {
         return (v_txo.txid);
      }
   };

   typedef eosio::multi_index<"vtxo4sc"_n, vtxo4sc, 
      indexed_by<"txid"_n, const_mem_fun<vtxo4sc, checksum256, &vtxo4sc::get_txid>>
   > type_table__vtxo4sc;

   ////////////////////////////////////////////////////////

   struct [[eosio::contract("eosio.system")]] addr2account {
      uint64_t          id;         //auto increament
      address           addr;
      name              account;

      uint64_t primary_key() const
      {
         return (id);
      }

      checksum256 get_addr() const
      {
         return eosio::sha256(addr.str_addr.c_str(), addr.str_addr.length() );
      }

      uint64_t get_account() const
      {
         return (account.value);
      }
   };

   typedef eosio::multi_index<"addr2account"_n, addr2account, 
      indexed_by<"addr"_n, const_mem_fun<addr2account, checksum256, &addr2account::get_addr>>,
      indexed_by<"account"_n, const_mem_fun<addr2account, uint64_t, &addr2account::get_account>>
   > type_table__addr2account;

   ////////////////////////////////////////////////////////

   /* 
   struct [[eosio::contract("eosio.system")]] vtxoes {

   };
   */

}
