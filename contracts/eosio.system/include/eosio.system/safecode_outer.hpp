
#include <eosio.system/eosio.system.core.hpp>

namespace eosiosystem {

    static const uint64_t COIN = 100000000;
    static const uint32_t WEEK_SEC = 1000;//7*86400 604800 XJTODO

   //######################################################
   ////////////////////////////////////////////////////////
   //common data type
   ////////////////////////////////////////////////////////
   //######################################################

   /**
    * sfaddress: safe-chain account(address format) obj
    * @details
    * - `str` address string, like: XbFwuS1mnMhDNq276ncvmbxayHthZkqfh1
    */
   struct sfaddress {
      std::string       str;

      void print() const
      {
         eosio::print("[struct address]obj:\n");
         eosio::print("\tstr = "); eosio::print(str); eosio::print("\n");
         eosio::print("[struct address]end of obj\n");
      }

      EOSLIB_SERIALIZE( sfaddress, (str) )
   };

   /**
    * txokey: safe-chain txo's key
    * @details
    * - `txid` transaction id(or named hash), like: c3bf493d8b2764748af87fb8bcf534b6c819ea913fe140251c5eabde3ff3ce74
    * - `outidx` out-index of txo.VOUT array, base from 0
    */
   struct txokey {
      checksum256       txid;
      uint8_t           outidx;

      bool operator < (const struct txokey& another) const
      {
          if(txid==another.txid)
            return outidx < another.outidx;
          return txid < another.txid;
      }

      EOSLIB_SERIALIZE( txokey, (txid)(outidx) )
   };

   enum txo_type{
      type_mn = 0,
      type_other = 1
   };

   /**
    * txo: txo with key and some attributes fields
    * @details
    * - `key` txo's key
    * - `quantity` txo's SAFE(only support SAFE) quantity
    * - `owner` txo's owner(address format)
    * - `type` assert type; 0:masternode-relation(lock or unlock); 1:otherwise
    * - `tp` when gen transaction; get it from the block timestamp.
    */
   struct txo {
      txokey            key;
      asset             quantity;
      sfaddress         owner;
      uint8_t           type;
      time_point        tp;

      void print() const
      {
         eosio::print("[struct txo]obj:\n");
         eosio::print("\ttxid = "); eosio::print(key.txid); eosio::print("\n");
         eosio::print("\toutidx = "); eosio::print(key.outidx); eosio::print("\n");
         eosio::print("\tquantity = "); eosio::print(quantity); eosio::print("\n");
         eosio::print("\towner = "); eosio::print(owner.str); eosio::print("\n");
         eosio::print("\ttype = "); eosio::print(type); eosio::print("\n");
         eosio::print("\ttp(sec) = "); eosio::print(tp.sec_since_epoch()); eosio::print("\n");
         eosio::print("[struct txo]end of obj\n");
      }

      EOSLIB_SERIALIZE( txo, (key)(quantity)(owner)(type)(tp) )
   };

   /**
    * sf5key: parameters for calling action driven by safe-chain
    * @details
    * - `atom_id` help to determining current call is in order
    * - `next_block_num` after this call, get safe-chain blocks to parse from position{`next_block_num`-`next_tx_index`}
    * - `next_tx_index` refer to `next_block_num`
    */
   struct sf5key {
      uint32_t          atom_id;
      uint32_t          next_block_num;
      uint32_t          next_tx_index;

      EOSLIB_SERIALIZE( sf5key, (atom_id)(next_block_num)(next_tx_index) )
   };

   ////////////////////////////////////////////////////////

   /**
    * sfreginfo: register-bp-info, submitted by safe-chain
    * @details
    * - `sc_pubkey` safecode-chain's bp pubkey
    * - `dvdratio` bp's dividend percent, [0,100]
    */
   struct sfreginfo {
      public_key        sc_pubkey;
      uint8_t           dvdratio;   //int: [0,100]
      uint16_t          location;

      void print() const
      {
         // eosio::print("[struct sfreginfo]obj:\n");
         // eosio::print("\tsc_pubkey = "); eosio::print(sc_pubkey); eosio::print("\n");
         // eosio::print("\tdvdratio = "); eosio::print(dvdratio); eosio::print("\n");
         // eosio::print("[struct sfreginfo]end of obj\n");
      }

      EOSLIB_SERIALIZE( sfreginfo, (sc_pubkey)(dvdratio)(location) )
   };

   /**
    * sfupdinfo: update-dvdration-info, submitted by safe-chain
    * @details
    * - `has__dvdratio` whether udpate dvdration
    * - `dvdratio` bp's dividend percent, [0,100]
    * - `has__location` whethe update location
    * - `location` ISO 3166 Country Code
    */
   struct sfupdinfo {
       bool         has__dvdratio;
       uint8_t      dvdratio;   //int: [0,100]
       bool         has__location;
       uint16_t     location;

       EOSLIB_SERIALIZE( sfupdinfo, (has__dvdratio)(dvdratio)(has__location)(location) )
   };

   /**
    * totals_pay: totals pay of every schedule,by safecode-chain
    * @details
    * - `total_bpay` total block pay
    * - `total_vpay` total vote pay
    * - `total_prod_bpay_to_self` total producer block pay,after cal dvdratio,which pay to self
    * - `total_prod_vpay_to_self` total producer vote pay,after cal dvdratio,which pay to self
    * - `total_prod_pay_to_voters` total producer pay,after cal dvdratio,pay to voters who vote the producer
    */
   struct totals_pay {
       double       total_bpay = 0.0;
       double       total_vpay = 0.0;
       double       total_prod_bpay_to_self = 0.0;
       double       total_prod_vpay_to_self = 0.0;
       double       total_prod_pay_to_voters = 0.0;

       EOSLIB_SERIALIZE( totals_pay, (total_bpay)(total_vpay)(total_prod_bpay_to_self)(total_prod_vpay_to_self)(total_prod_pay_to_voters) )
   };

   /**
    * prods_pay: producers pay,by safecode-chain
    * @details
    * - `owner` bp's safecode account
    * - `sfaddr` bp's safe address
    * - `prod_bpay` producer block pay
    * - `prod_vpay` producer vote pay
    * - `prod_total_pay` prod_bpay+prod_vpay
    * - `prod_bpay_to_self` prod_bpay*(100-dvdratio)/100
    * - `prod_vpay_to_self` prod_vpay*(100-dvdratio)/100
    * - `prod_coinage` producer coin age
    * - `unpaid_block` produce a block,unpaid block + 1
    * - `dvdratio` bp's dividend percent, [0,100]
    */
   struct prods_pay {
       name         owner;
       sfaddress    sfaddr;
       double       prod_bpay = 0.0;
       double       prod_vpay = 0.0;
       double       prod_total_pay = 0.0;
       double       prod_bpay_to_self = 0.0;
       double       prod_vpay_to_self = 0.0;
       double       prod_pay_to_voters = 0.0;
       double       prod_coinage = 0.0;
       uint64_t     unpaid_block = 0;
       uint8_t      dvdratio = 0;

       EOSLIB_SERIALIZE( prods_pay, (owner)(sfaddr)(prod_bpay)(prod_vpay)(prod_total_pay)(prod_bpay_to_self)(prod_vpay_to_self)(prod_pay_to_voters)
                         (prod_coinage)(unpaid_block)(dvdratio))
   };

   /**
    * voters_pay: voters pay,by safecode-chain
    * @details
    * - `producer` vote which bp account
    * - `vote_age` current block time - vote time
    * - `vote_coinage` vote coin age
    * - `voter_vpay` voter vote pay
    */
   struct voters_pay
   {
       voters_pay() {}
       name     producer;
       uint32_t vote_age = 0;
       double   vote_coinage = 0.0;
       double   voter_vpay = 0.0;

       EOSLIB_SERIALIZE( voters_pay,(producer)(vote_age)(vote_coinage)(voter_vpay))
   };

   //######################################################
   ////////////////////////////////////////////////////////
   //struct and table
   ////////////////////////////////////////////////////////
   //######################################################

   /**
    * year3rewards: per-block rewards calculated by year order number
    * @details
    * - `ynr` year number, based from `global4vote.active_timestamp`
    * - `amount` per-block rewards, unit: 1E-8 (SAFE)
    */
   struct [[eosio::table("year3rewards"),eosio::contract("eosio.system")]] year3rewards {
      uint8_t  ynr;
      uint64_t amount;

      uint64_t primary_key() const
      {
         return (ynr);
      }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( year3rewards, (ynr)(amount) )
   };
   typedef eosio::multi_index<"year3rewards"_n, year3rewards> type_table__year3rewards;

   ////////////////////////////////////////////////////////

   /**
    * global4vote: config infoes
    * @details
    * - `last_sch_ver` last block_header.schedule_version
    * - `active_timestamp` when mainnet is actived first time
    * - `last_calc_rewards_timestamp` when last calculating voting rewards
    * - `last_set_proposed_producers_timestamp` when last set proposed producers
    * - `last_unpaid_rewards` per-block rewards, unit: 1E-8 (SAFE)
    * - `last_unpaid_block` amount of last unpaid block
    * - `last_claim_week` last claim week order number, based from `active_timestamp`
    * - `last_top40bp_votes_change` last round if top40bp votes detail changed, including bp's dvdratio
    * - `sf_atom_id` record `sf5key.atomid`
    * - `sf_block_num` record `sf5key.next_block_num`
    * - `sf_tx_index` record `sf5key.next_tx_index`
    * - `soft_trigger_calc_reward` no producer change trigger calculate reward
    */
   struct [[eosio::table("global4vote"),eosio::contract("eosio.system")]] global4vote {
      global4vote(){}

      uint32_t    last_sch_ver;
      uint32_t    active_timestamp;
      uint32_t    last_calc_rewards_timestamp;
      uint32_t    last_set_proposed_producers_timestamp;
      uint64_t    last_unpaid_rewards;
      uint32_t    last_unpaid_block;
      uint32_t    last_claim_week;//XJTODO give value
      bool        last_top40bp_votes_change;
      uint32_t    sf_atom_id;
      uint32_t    sf_block_num;
      uint16_t    sf_tx_index;
      bool        soft_trigger_calc_reward;
      uint16_t    curr_prods_count;//XJTODO for test
      std::string str_log;//XJTODO for some test

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( global4vote, (last_sch_ver)(active_timestamp)(last_calc_rewards_timestamp)
         (last_set_proposed_producers_timestamp)(last_unpaid_rewards)(last_unpaid_block)
         (last_claim_week)(last_top40bp_votes_change)(sf_atom_id)(sf_block_num)(sf_tx_index)
         (soft_trigger_calc_reward)(curr_prods_count)(str_log))
   };
   typedef eosio::singleton< "global4vote"_n, global4vote >   global4vote_singleton;

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

   /**
    * sf5producers: all bp(real-time)'s voted and enable info
    * @details
    * - `prmrid` primary key, owner is not set when creating
    * - `rptxokey` register bp's txokey
    * - `ri` register infoes
    * - `owner` bp's safecode account, set after creating
    * - `sf_vtotal` total voting amount by safe-chain; = asset.amount * ration(1 or 1.5); unit: 1E-8 (SAFE)
    * - `vtotal` total voting amount by safe-chain and safecode-chain; unit: 1E-8 (SAFE)
    * - `enable` false when unreg current bp
    */
   struct [[eosio::table,eosio::contract("eosio.system")]] sf5producers {
      uint64_t          prmrid;
      txokey            rptxokey;
      sfreginfo         ri;
      name              owner;
      double            sf_vtotal;
      double            vtotal;
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

      double index_by_sf5vtotal() const
      {
         return (sf_vtotal);
      }

      double index_by_vtotal() const
      {
         return (vtotal);
      }

      uint8_t get_tx_outidx() const
      {
         return (rptxokey.outidx);
      }

      EOSLIB_SERIALIZE( sf5producers, (prmrid)(rptxokey)(ri)(owner)(sf_vtotal)(vtotal)(enable) )
   };

   typedef eosio::multi_index<"sf5producers"_n, sf5producers, 
      indexed_by<"by3txid"_n, const_mem_fun<sf5producers, checksum256, &sf5producers::index_by_txid>>,
      indexed_by<"by3pubkey"_n, const_mem_fun<sf5producers, checksum256, &sf5producers::index_by_pubkey>>,
      indexed_by<"by3owner"_n, const_mem_fun<sf5producers, uint64_t, &sf5producers::index_by_owner>>,
      indexed_by<"by3sf5vtotal"_n, const_mem_fun<sf5producers, double, &sf5producers::index_by_sf5vtotal>>,
      indexed_by<"by3vtotal"_n, const_mem_fun<sf5producers, double, &sf5producers::index_by_vtotal>>
   > type_table__sf5producers;

   ////////////////////////////////////////////////////////

   /**
    * sf5vtxo: all bp(real-time)'s vote infoes by safe-chain
    * @details
    * - `prmrid` primary key
    * - `rptxokey` voted target, register bp's txokey
    * - `vtxo` vote detail info
    * - `owner` bp's safecode account, set after creating
    * - `vtotal` total voting amount by safe-chain; = asset.amount * ration(1 or 1.5); unit: 1E-8 (SAFE)
    */
   struct [[eosio::table,eosio::contract("eosio.system")]] sf5vtxo {
      uint64_t          prmrid;
      txokey            rptxokey;
      txo               vtxo;
      double            vtotal;

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

      checksum256 index_by_rptxid() const
      {
          return (rptxokey.txid);
      }

      uint8_t get_rptx_outidx() const
      {
          return (rptxokey.outidx);
      }

      EOSLIB_SERIALIZE( sf5vtxo, (prmrid)(rptxokey)(vtxo)(vtotal) )
   };

   typedef eosio::multi_index<"sf5vtxo"_n, sf5vtxo, 
      indexed_by<"by3txid"_n, const_mem_fun<sf5vtxo, checksum256, &sf5vtxo::index_by_txid>>,
      indexed_by<"by3rptxid"_n, const_mem_fun<sf5vtxo, checksum256, &sf5vtxo::index_by_rptxid>>
   > type_table__sf5vtxo;

   ////////////////////////////////////////////////////////

   /**
    * sc5voters: all bp(real-time)'s vote infoes by safecode-chain
    * @details
    * - `owner` voter, safecode account
    * - `producer` voted target
    * - `staked` amount of SAFE staked to vote
    * - `vote_tp` when do the last vote action
    * - `vtotal` total voting amount by safecode-chain; unit: 1E-8 (SAFE)
    */
   struct [[eosio::table, eosio::contract("eosio.system")]] sc5voters {
      name              owner;     /// the voter
      name              proxy;     /// the proxy set by the voter, if any
      name              producer;  /// the producer approved by this voter if no proxy set
      int64_t           staked = 0;

      time_point        vote_tp;
      double            vtotal = 0; /// the vote weight cast the last time the vote was updated

      double            proxied_vtotal= 0; /// the total vote weight delegated to this voter as a proxy
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
      EOSLIB_SERIALIZE( sc5voters, (owner)(proxy)(producer)(staked)(vote_tp)(vtotal)(proxied_vtotal)(is_proxy)(flags1)(reserved2)(reserved3) )
   };
   typedef eosio::multi_index< "sc5voters"_n, sc5voters >  type_table__sc5voters;

   ////////////////////////////////////////////////////////

   /**
    * f3sf5prods: top40 bp(forward-rotation)'s voting amount
    * @details
    * - `owner` bp's safecode account
    * - `rptxokey` register bp's txokey
    * - `dvdratio` bp's dividend percent, [0,100]
    * - `vtotal` total voting amount by safe-chain and safecode-chain; unit: 1E-8 (SAFE)
    */
   struct [[eosio::table,eosio::contract("eosio.system")]] f3sf5prods {
      name              owner;
      txokey            rptxokey;
      uint8_t           dvdratio;
      double            vtotal;

      uint64_t primary_key() const
      {
         return (owner.value);
      }

      checksum256 index_by_txid() const
      {
         return (rptxokey.txid);
      }

      uint8_t get_tx_outidx() const
      {
         return (rptxokey.outidx);
      }

      uint64_t index_by_owner() const
      {
         return (owner.value);
      }

      double index_by_vtotal() const
      {
         return (vtotal);
      }

      EOSLIB_SERIALIZE( f3sf5prods, (owner)(rptxokey)(dvdratio)(vtotal) )
   };

   typedef eosio::multi_index<"f3sf5prods"_n, f3sf5prods, 
      indexed_by<"by3txid"_n, const_mem_fun<f3sf5prods, checksum256, &f3sf5prods::index_by_txid>>,
      indexed_by<"by3owner"_n, const_mem_fun<f3sf5prods, uint64_t, &f3sf5prods::index_by_owner>>,
      indexed_by<"by3vtotal"_n, const_mem_fun<f3sf5prods, double, &f3sf5prods::index_by_vtotal>>
   > type_table__f3sf5prods;

   ////////////////////////////////////////////////////////

   /**
    * f3sf5vtxo: top40 bp(forward-rotation)'s vote infoes by safe-chain
    * @details
    * - `prmrid` primary key
    * - `rptxokey` voted target, register bp's txokey
    * - `vtxokey` vtxo key
    * - `sfaddr` vtxo's owner(address format)
    * - `vote_tp` when do the last vote action
    * - `vtotal` total voting amount by safe-chain; = asset.amount * ration(1 or 1.5); unit: 1E-8 (SAFE)
    */
   struct [[eosio::table,eosio::contract("eosio.system")]] f3sf5vtxo {
      uint64_t          prmrid;
      txokey            rptxokey;
      txokey            vtxokey;
      sfaddress         sfaddr;
      time_point        vote_tp;
      double            vtotal;

      uint64_t primary_key() const
      {
         return (prmrid);
      }

      checksum256 index_by_txid() const
      {
         return (vtxokey.txid);
      }

      uint8_t get_tx_outidx() const
      {
         return (vtxokey.outidx);
      }

      EOSLIB_SERIALIZE( f3sf5vtxo, (prmrid)(rptxokey)(vtxokey)(sfaddr)(vote_tp)(vtotal) )
   };

   typedef eosio::multi_index<"f3sf5vtxo"_n, f3sf5vtxo, 
      indexed_by<"by3txid"_n, const_mem_fun<f3sf5vtxo, checksum256, &f3sf5vtxo::index_by_txid>>
   > type_table__f3sf5vtxo;

   ////////////////////////////////////////////////////////

   /**
    * f3voters: top40 bp(forward-rotation)'s vote infoes by safecode-chain
    * @details
    * - `owner` voter, safecode account
    * - `producer` voted target
    * - `staked` amount of SAFE staked to vote
    * - `vote_tp` when do the last vote action
    * - `vtotal` total voting amount by safecode-chain; unit: 1E-8 (SAFE)
    */
   struct [[eosio::table, eosio::contract("eosio.system")]] f3voters {
      name              owner;     /// the voter
      name              proxy;     /// the proxy set by the voter, if any
      name              producer;  /// the producer approved by this voter if no proxy set
      int64_t           staked = 0;

      time_point        vote_tp;
      double            vtotal = 0; /// the vote weight cast the last time the vote was updated

      double            proxied_vtotal= 0; /// the total vote weight delegated to this voter as a proxy
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
      EOSLIB_SERIALIZE( f3voters, (owner)(proxy)(producer)(staked)(vote_tp)(vtotal)(proxied_vtotal)(is_proxy)(flags1)(reserved2)(reserved3) )
   };
   typedef eosio::multi_index< "f3voters"_n, f3voters >  type_table__f3voters;

   ////////////////////////////////////////////////////////

   /**
    * p3sf5prods: top40 bp(post-rotation)'s voting amount
    * @details
    * - `owner` bp's safecode account
    * - `rptxokey` register bp's txokey
    * - `dvdratio` bp's dividend percent, [0,100]
    * - `unpaid_block` unpaid block number within current settlement period
    */
   struct [[eosio::table,eosio::contract("eosio.system")]] p3sf5prods {
      name              owner;
      txokey            rptxokey;
      uint8_t           dvdratio;
      uint64_t          unpaid_block;

      uint64_t primary_key() const
      {
         return (owner.value);
      }

      checksum256 index_by_txid() const
      {
         return (rptxokey.txid);
      }

      uint64_t index_by_owner() const
      {
         return (owner.value);
      }

      EOSLIB_SERIALIZE( p3sf5prods, (owner)(rptxokey)(dvdratio)(unpaid_block) )
   };

   typedef eosio::multi_index<"p3sf5prods"_n, p3sf5prods, 
      indexed_by<"by3txid"_n, const_mem_fun<p3sf5prods, checksum256, &p3sf5prods::index_by_txid>>,
      indexed_by<"by3owner"_n, const_mem_fun<p3sf5prods, uint64_t, &p3sf5prods::index_by_owner>>
   > type_table__p3sf5prods;

   ////////////////////////////////////////////////////////

   /**
    * p3sf5vtxo: top40 bp(post-rotation)'s vote infoes by safe-chain
    * @details
    * - `prmrid` primary key
    * - `rptxokey` voted target, register bp's txokey
    * - `vtxokey` vtxo key
    * - `sfaddr` vtxo's owner(address format)
    * - `vote_tp` when do the last vote action
    * - `vtotal` total voting amount by safe-chain; = asset.amount * ration(1 or 1.5); unit: 1E-8 (SAFE)
    */
   struct [[eosio::table,eosio::contract("eosio.system")]] p3sf5vtxo {
      uint64_t          prmrid;
      txokey            rptxokey;
      txokey            vtxokey;
      sfaddress         sfaddr;
      time_point        vote_tp;
      double            vtotal;

      uint64_t primary_key() const
      {
         return (prmrid);
      }

      checksum256 index_by_txid() const
      {
         return (vtxokey.txid);
      }

      uint8_t get_tx_outidx() const
      {
         return (vtxokey.outidx);
      }

      EOSLIB_SERIALIZE( p3sf5vtxo, (prmrid)(rptxokey)(vtxokey)(sfaddr)(vote_tp)(vtotal) )
   };

   typedef eosio::multi_index<"p3sf5vtxo"_n, p3sf5vtxo, 
      indexed_by<"by3txid"_n, const_mem_fun<p3sf5vtxo, checksum256, &p3sf5vtxo::index_by_txid>>
   > type_table__p3sf5vtxo;

   ////////////////////////////////////////////////////////

   /**
    * p3voters: top40 bp(post-rotation)'s vote infoes by safecode-chain
    * @details
    * - `owner` voter, safecode account
    * - `producer` voted target
    * - `staked` amount of SAFE staked to vote
    * - `vote_tp` when do the last vote action
    * - `vtotal` total voting amount by safecode-chain; unit: 1E-8 (SAFE)
    */
   struct [[eosio::table, eosio::contract("eosio.system")]] p3voters {
      name              owner;     /// the voter
      name              proxy;     /// the proxy set by the voter, if any
      name              producer;  /// the producer approved by this voter if no proxy set
      int64_t           staked = 0;

      time_point        vote_tp;
      double            vtotal = 0; /// the vote weight cast the last time the vote was updated

      double            proxied_vtotal= 0; /// the total vote weight delegated to this voter as a proxy
      bool              is_proxy = 0; /// whether the voter is a proxy for others

      uint64_t primary_key() const
      {
         return owner.value;
      }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( p3voters, (owner)(proxy)(producer)(staked)(vote_tp)(vtotal)(proxied_vtotal)(is_proxy) )
   };
   typedef eosio::multi_index< "p3voters"_n, p3voters >  type_table__p3voters;

   ////////////////////////////////////////////////////////

   /**
    * rewards4bp: bp(current or history)'s bpay rewards
    * @details
    * - `owner` bp account
    * - `period` rewards within current period; unit: 1E-8 (SAFE)
    * - `unclaimed` unclaimed rewards from all periods; unit: 1E-8 (SAFE)
    */
   struct [[eosio::table, eosio::contract("eosio.system")]] rewards4bp {
      name              owner;
      uint64_t          period;
      uint64_t          unclaimed;

      uint64_t primary_key() const
      {
         return (owner.value);
      }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( rewards4bp, (owner)(period)(unclaimed) )
   };
   typedef eosio::multi_index<"rewards4bp"_n, rewards4bp> type_table__rewards4bp;

   ////////////////////////////////////////////////////////

   /**
    * rewards4v: bp or voter(current or history)'s vpay rewards
    * @details
    * - `prmrid` primary key
    * - `sfaddr` vtxo's owner(address format)
    * - `owner` bp or voter's safecode-chain account
    * - `period` rewards within current period; unit: 1E-8 (SAFE)
    * - `unclaimed` unclaimed rewards from all periods; unit: 1E-8 (SAFE)
    * 
    * for bp: sfaddr="" and owner="bbb"_n
    * for voter, before binding safecode-chain account, sfaddr="xxx" and owner=0
    * for voter, after binding safecode-chain account, sfaddr="xxx" and owner="vvv"_n
    */
   struct [[eosio::table, eosio::contract("eosio.system")]] rewards4v {
      uint64_t          prmrid;
      sfaddress         sfaddr;
      name              owner;
      uint64_t          period;
      uint64_t          unclaimed;

      uint64_t primary_key() const
      {
         return (prmrid);
      }

      checksum256 index_by_sfaddr() const
      {
         return eosio::sha256( sfaddr.str.c_str(), sfaddr.str.length() );
      }

      uint64_t index_by_owner() const
      {
         return (owner.value);
      }

      // explicit serialization macro is not necessary, used here only to improve compilation time
      EOSLIB_SERIALIZE( rewards4v, (prmrid)(sfaddr)(owner)(period)(unclaimed) )
   };
   typedef eosio::multi_index<"rewards4v"_n, rewards4v,
      indexed_by<"by3sfaddr"_n, const_mem_fun<rewards4v, checksum256, &rewards4v::index_by_sfaddr>>,
      indexed_by<"by3owner"_n, const_mem_fun<rewards4v, uint64_t, &rewards4v::index_by_owner>>
   > type_table__rewards4v;

   ////////////////////////////////////////////////////////

   /**
    * sfaddr2accnt: safe-chain account map to safecode-chain account
    * @details
    * - `prmrid` primary key
    * - `sfaddr` vtxo's owner(address format)
    * - `account` safecode-chain account
    * 
    * `sfaddr`:`account` = n:1
    */
   struct [[eosio::table,eosio::contract("eosio.system")]] sfaddr2accnt {
      uint64_t          prmrid;
      sfaddress         sfaddr;
      name              account;

      uint64_t primary_key() const
      {
         return (prmrid);
      }

      checksum256 index_by_sfaddr() const
      {
         return eosio::sha256( sfaddr.str.c_str(), sfaddr.str.length() );
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
