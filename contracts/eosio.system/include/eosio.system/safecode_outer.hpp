
#include <eosio.system/eosio.system.core.hpp>

namespace eosiosystem {

   struct [[eosio::contract("eosio.system")]] sc5rewards {

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

   typedef eosio::singleton< "sc5rewards"_n, sc5rewards >   sc5rewards_singleton;

   ////////////////////////////////////////////////////////

   struct sfaddress {  //main-chain account obj
      std::string       str;

      void print() const
      {
         eosio::print("[struct address]obj:\n");
         eosio::print("\tstr = "); eosio::print(str); eosio::print("\n");
         eosio::print("[struct address]end of obj\n");
      }
   };

   struct txokey {
      checksum256       txid;    //txid at safe chain
      uint8_t           outidx;  //out-index of utxo tx's vout array, base from 0
   };

   struct txo {
      txokey            key;
      uint64_t          quantity;
      sfaddress         from;
      uint8_t           type;    //0:masternode-locked; 1:non-masternode-locked, liquid
      time_point        tp;      //when gen transaction

      void print() const
      {
         eosio::print("[struct txo]obj:\n");
         eosio::print("\ttxid = "); eosio::print(key.txid); eosio::print("\n");
         eosio::print("\toutidx = "); eosio::print(key.outidx); eosio::print("\n");
         eosio::print("\tquantity = "); eosio::print(quantity); eosio::print("\n");
         eosio::print("\tfrom = "); eosio::print(from.str); eosio::print("\n");
         eosio::print("\ttype = "); eosio::print(type); eosio::print("\n");
         eosio::print("\ttp(sec) = "); eosio::print(tp.sec_since_epoch()); eosio::print("\n");
         eosio::print("[struct txo]end of obj\n");
      }
   };

   ////////////////////////////////////////////////////////

   struct sfreginfo {
      public_key        sc_pubkey;
      uint8_t           dvdratio;   //int: [0,100]
      checksum256       infohash;
      signature         sc_sig;

      void print() const
      {
         // eosio::print("[struct sfreginfo]obj:\n");
         // eosio::print("\tsc_pubkey = "); eosio::print(sc_pubkey); eosio::print("\n");
         // eosio::print("\tdvdratio = "); eosio::print(dvdratio); eosio::print("\n");
         // eosio::print("\tinfohash = "); eosio::print(infohash); eosio::print("\n");
         // eosio::print("\tsc_sig = "); eosio::print(sc_sig); eosio::print("\n");
         // eosio::print("[struct sfreginfo]end of obj\n");
      }
   };

   struct [[eosio::table,eosio::contract("eosio.system")]] sf5producers {
      uint64_t          prmrid;     //auto increament
      txo               rptxo;
      sfreginfo         ri;
      bool              enable;

      uint64_t primary_key() const
      {
         return (prmrid);
      }

      checksum256 index_by_txid() const
      {
         return (rptxo.key.txid);
      }

      uint8_t get_tx_outidx() const
      {
         return (rptxo.key.outidx);
      }
   };

   typedef eosio::multi_index<"sf5producers"_n, sf5producers, 
      indexed_by<"by3txid"_n, const_mem_fun<sf5producers, checksum256, &sf5producers::index_by_txid>>
   > type_table__sf5producers;


   ////////////////////////////////////////////////////////

   struct [[eosio::table,eosio::contract("eosio.system")]] sf5vtxo {
      uint64_t          prmrid;     //auto increament
      checksum256       rptxid;
      txo               vtxo;
      time_point        tp_vote;    //when voting
      double            weight;

      uint64_t primary_key() const
      {
         return (prmrid);
      }

      checksum256 index_by_txid() const
      {
         return (vtxo.key.txid);
      }

      uint8_t get_tx_outidx() const
      {
         return (vtxo.key.outidx);
      }

   };

   typedef eosio::multi_index<"sf5vtxo"_n, sf5vtxo, 
      indexed_by<"by3txid"_n, const_mem_fun<sf5vtxo, checksum256, &sf5vtxo::index_by_txid>>
   > type_table__sf5vtxo;

   ////////////////////////////////////////////////////////

   struct [[eosio::table,eosio::contract("eosio.system")]] sfaddr2accnt {
      uint64_t          id;         //auto increament
      sfaddress         sfaddr;
      name              account;

      uint64_t primary_key() const
      {
         return (id);
      }

      checksum256 index_by_sfaddr() const
      {
         return eosio::sha256(sfaddr.str.c_str(), sfaddr.str.length() );
      }

      uint64_t index_by_account() const
      {
         return (account.value);
      }
   };

   typedef eosio::multi_index<"sfaddr2accnt"_n, sfaddr2accnt, 
      indexed_by<"by3sfaddr"_n, const_mem_fun<sfaddr2accnt, checksum256, &sfaddr2accnt::index_by_sfaddr>>,
      indexed_by<"by3account"_n, const_mem_fun<sfaddr2accnt, uint64_t, &sfaddr2accnt::index_by_account>>
   > type_table__sfaddr2accnt;

   ////////////////////////////////////////////////////////



}
