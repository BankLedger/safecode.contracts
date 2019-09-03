
#include <eosio.system/eosio.system.core.hpp>

namespace eosiosystem {

   struct [[eosio::contract("eosio.system")]] year3rewards {
      uint8_t  ynr;        //base from 0
      uint64_t amount;     //per block, unit (1E-8 SAFE)

      uint64_t primary_key() const
      {
         return (ynr);
      }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( year3rewards, (ynr)(amount) )
   };
   typedef eosio::multi_index<"year3rewards"_n, year3rewards> type_table__year3rewards;

   ////////////////////////////////////////////////////////

   struct [[eosio::contract("eosio.system")]] global4vote {

      uint32_t    last_sch_ver;
      uint32_t    active_timestamp;
      uint32_t    last_calc_rewards_timestamp;
      uint64_t    last_unpaid_rewards;
      uint32_t    last_unpaid_block;
      uint32_t    last_claim_week;
      bool        last_top40bp_votes_change;
      uint32_t    sf_atom_id;
      uint32_t    sf_block_num;
      uint16_t    sf_tx_index;

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( global4vote, (last_sch_ver)(active_timestamp)(last_calc_rewards_timestamp)
         (last_unpaid_rewards)(last_unpaid_block)(last_claim_week)(last_top40bp_votes_change)
         (sf_atom_id)(sf_block_num)(sf_tx_index) )
   };
   typedef eosio::singleton< "global4vote"_n, global4vote >   global4vote_singleton;

   ////////////////////////////////////////////////////////



   ////////////////////////////////////////////////////////

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

      EOSLIB_SERIALIZE( sfaddress, (str) )
   };

   struct txokey {
      checksum256       txid;    //txid at safe chain
      uint8_t           outidx;  //out-index of utxo tx's vout array, base from 0
   
      EOSLIB_SERIALIZE( txokey, (txid)(outidx) )
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

      EOSLIB_SERIALIZE( txo, (key)(quantity)(from)(type)(tp) )
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

      EOSLIB_SERIALIZE( sfreginfo, (sc_pubkey)(dvdratio)(infohash)(sc_sig) )
   };

   struct [[eosio::table,eosio::contract("eosio.system")]] sf5producers {
      uint64_t          prmrid;     //auto increament
      txokey            rptxokey;
      sfreginfo         ri;
      name              owner;
      uint64_t          sf_total;   //sf only
      uint64_t          total;      //sf + sc
      bool              enable;

      uint64_t primary_key() const
      {
         return (prmrid);
      }

      checksum256 index_by_txid() const
      {
         return (rptxokey.txid);
      }

      checksum256 index_by_pubkey() const
      {
         return (eosio::sha256(ri.sc_pubkey.data.begin(), ri.sc_pubkey.data.size()));
      }

      uint64_t index_by_owner() const
      {
         return (owner.value);
      }

      uint64_t index_by_sf5total() const
      {
         return (sf_total);
      }

      uint64_t index_by_total() const
      {
         return (total);
      }

      uint8_t get_tx_outidx() const
      {
         return (rptxokey.outidx);
      }

      EOSLIB_SERIALIZE( sf5producers, (prmrid)(rptxokey)(ri)(owner)(sf_total)(total)(enable) )
   };

   typedef eosio::multi_index<"sf5producers"_n, sf5producers, 
      indexed_by<"by3txid"_n, const_mem_fun<sf5producers, checksum256, &sf5producers::index_by_txid>>,
      indexed_by<"by3pubkey"_n, const_mem_fun<sf5producers, checksum256, &sf5producers::index_by_pubkey>>,
      indexed_by<"by3owner"_n, const_mem_fun<sf5producers, uint64_t, &sf5producers::index_by_owner>>,
      indexed_by<"by3sf5total"_n, const_mem_fun<sf5producers, uint64_t, &sf5producers::index_by_sf5total>>,
      indexed_by<"by3total"_n, const_mem_fun<sf5producers, uint64_t, &sf5producers::index_by_total>>
   > type_table__sf5producers;

   ////////////////////////////////////////////////////////

   struct [[eosio::table,eosio::contract("eosio.system")]] sf5vtxo {
      uint64_t          prmrid;     //auto increament
      txokey            rptxokey;
      txo               vtxo;
      uint64_t          total;

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

      EOSLIB_SERIALIZE( sf5vtxo, (prmrid)(rptxokey)(vtxo)(total) )
   };

   typedef eosio::multi_index<"sf5vtxo"_n, sf5vtxo, 
      indexed_by<"by3txid"_n, const_mem_fun<sf5vtxo, checksum256, &sf5vtxo::index_by_txid>>
   > type_table__sf5vtxo;

   ////////////////////////////////////////////////////////

   struct [[eosio::table, eosio::contract("eosio.system")]] sc5voters {
      name              owner;     /// the voter
      name              proxy;     /// the proxy set by the voter, if any
      name              producer;  /// the producer approved by this voter if no proxy set
      int64_t           staked = 0;

      time_point        last_vote_tp;
      uint64_t          last_vote_weight = 0; /// the vote weight cast the last time the vote was updated

      uint64_t          proxied_vote_weight= 0; /// the total vote weight delegated to this voter as a proxy
      bool              is_proxy = 0; /// whether the voter is a proxy for others


      uint32_t          flags1 = 0;
      uint32_t          reserved2 = 0;
      eosio::asset      reserved3;

      uint64_t primary_key()const { return owner.value; }

      enum class flags1_fields : uint32_t {
         ram_managed = 1,
         net_managed = 2,
         cpu_managed = 4
      };

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( sc5voters, (owner)(proxy)(producer)(staked)(last_vote_tp)(last_vote_weight)(proxied_vote_weight)(is_proxy)(flags1)(reserved2)(reserved3) )
   };
   typedef eosio::multi_index< "sc5voters"_n, sc5voters >  type_table__sc5voters;

   ////////////////////////////////////////////////////////

   struct [[eosio::table,eosio::contract("eosio.system")]] f3sf5prods {
      name              owner;
      txokey            rptxokey;
      uint8_t           dvdratio;
      uint64_t          total;      //sf + sc

      uint64_t primary_key() const
      {
         return (owner.value);
      }

      checksum256 index_by_txid() const
      {
         return (rptxokey.txid);
      }

      EOSLIB_SERIALIZE( f3sf5prods, (owner)(rptxokey)(dvdratio)(total) )
   };

   typedef eosio::multi_index<"f3sf5prods"_n, f3sf5prods, 
      indexed_by<"by3txid"_n, const_mem_fun<f3sf5prods, checksum256, &f3sf5prods::index_by_txid>>
   > type_table__f3sf5prods;

   ////////////////////////////////////////////////////////

   struct [[eosio::table,eosio::contract("eosio.system")]] sfaddr2accnt {
      uint64_t          prmrid;         //auto increament
      sfaddress         sfaddr;
      name              account;

      uint64_t primary_key() const
      {
         return (prmrid);
      }

      checksum256 index_by_sfaddr() const
      {
         return eosio::sha256(sfaddr.str.c_str(), sfaddr.str.length() );
      }

      uint64_t index_by_account() const
      {
         return (account.value);
      }

      EOSLIB_SERIALIZE( sfaddr2accnt, (prmrid)(sfaddr)(account) )
   };

   typedef eosio::multi_index<"sfaddr2accnt"_n, sfaddr2accnt, 
      indexed_by<"by3sfaddr"_n, const_mem_fun<sfaddr2accnt, checksum256, &sfaddr2accnt::index_by_sfaddr>>,
      indexed_by<"by3account"_n, const_mem_fun<sfaddr2accnt, uint64_t, &sfaddr2accnt::index_by_account>>
   > type_table__sfaddr2accnt;

   ////////////////////////////////////////////////////////

}
