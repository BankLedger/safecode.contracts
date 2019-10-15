#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/printkit.hpp>

namespace eosiosystem {

   using eosio::check;

    global4vote system_contract::get_default_global4vote()
    {
        global4vote ret;
        ret.last_sch_ver = 0;
        ret.active_timestamp = 1571103531;//XJTODO,for test,need be set to 0;
        ret.last_calc_rewards_timestamp = 0;
        ret.last_set_proposed_producers_timestamp = 0;
        ret.last_unpaid_rewards = 0;
        ret.last_unpaid_block = 0;
        ret.last_claim_week = 0;
        ret.last_top40bp_votes_change = false;
        ret.sf_atom_id = 0;
        ret.sf_block_num = 0;
        ret.sf_tx_index = 0;
        ret.soft_trigger_calc_reward = false;
        ret.curr_prods_count = 0;
        ret.str_log = "";
        return (ret);
    }

   void system_contract::init_year_reward()
   {
       uint8_t ynr = 0;
       auto itr = _year3rewards.find(ynr);
       if(itr != _year3rewards.end()){
           return;
       }
       std::vector<uint64_t> amount_vec={COIN,2*COIN};
       for(auto itr = amount_vec.begin(); itr != amount_vec.end(); ++itr)
       {
           _year3rewards.emplace(get_self(), [&]( auto& row ) {
               row.ynr = ynr;
               row.amount = *itr;
            });
           ++ynr;
       }
   }

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

   template< typename TableIndex,typename Index>
   auto system_contract::findByUniqueIdx( const TableIndex& tbl_index, const Index& index )
   {
       auto itr_find_index = tbl_index.lower_bound(index);
       auto itr_find_index_up = tbl_index.upper_bound(index);
       return std::make_tuple(itr_find_index != itr_find_index_up,itr_find_index);
   }

   bool system_contract::dvalue_bigger_than_zero(const double& dvalue)
   {
       if(dvalue>=0.00000001){
           return true;
       }
       return false;
   }

   bool system_contract::negative_dvalue_equals_zero(const double& dvalue)
   {
       if(dvalue>=-0.00000001&&dvalue<0){
           return true;
       }
       return false;
   }

   void system_contract::sf5regprod( const struct sf5key& sfkey, const struct txokey& rptxokey, const struct sfreginfo& ri )
   {
      auto found_ret = findByTxo(_sf5producers.get_index<"by3txid"_n>(), rptxokey);
      auto found = std::get<0>(found_ret);
      check( found == false, "error,sf5regprod:rptxo has exists at table sf5producers" );
      check( ri.dvdratio >= 0 && ri.dvdratio <= 100, "error, ri.dvdratio must be in range [0, 100]" );
      auto found_pubkey_ret = findByUniqueIdx(_sf5producers.get_index<"by3pubkey"_n>(),eosio::sha256(ri.sc_pubkey.data.begin(), ri.sc_pubkey.data.size()));
      auto found_pubkey = std::get<0>(found_pubkey_ret);
      check(found_pubkey == false, "error,sf5regprod:pubkey has exists at table sf5producers");

      _sf5producers.emplace(get_self(), [&]( auto& row ) {
         row.prmrid     = _sf5producers.available_primary_key();
         row.rptxokey   = rptxokey;
         row.ri         = ri;
         row.owner      = name(0);
         row.sf_vtotal  = 0;
         row.vtotal     = 0;
         row.enable     = true;
      });

      setnext(sfkey);
      eosio::print("sf5regprod:regist bp [",rptxokey.txid,",",rptxokey.outidx,"]\n");
   }

   void system_contract::sf5unregprod( const struct sf5key& sfkey, const struct txokey& rptxokey )
   {
       auto txid_idx = _sf5producers.get_index<"by3txid"_n>();
       auto found_ret = findByTxo(txid_idx, rptxokey);
       auto found = std::get<0>(found_ret);
       check( found, "error, sf5unregprod:rptxo has not exists at table sf5producers" );

       auto itr = std::get<1>(found_ret);
       txid_idx.modify(itr, get_self(), [&]( auto& row ) {
            row.enable = false;
       });

       //unregprod top40,update flag
       auto prods_txid_ret = findByTxo(_f3sf5prods.get_index<"by3txid"_n>(),rptxokey);
       auto exists = std::get<0>(prods_txid_ret);
       if(exists){
           _gstate4vote.last_top40bp_votes_change = true;
       }

       setnext(sfkey);
       eosio::print("sf5unregprod:unregist bp:[",rptxokey.txid,",",rptxokey.outidx,"],top40:",exists,"\n");
   }

   void system_contract::sf5updprodri(const struct sf5key& sfkey, const struct txokey& rptxokey, const struct sfupdinfo& updri)
   {
       auto txid_idx = _sf5producers.get_index<"by3txid"_n>();
       auto found_ret = findByTxo(txid_idx, rptxokey);
       auto found = std::get<0>(found_ret);
       check( found, "error, sf5updprodri:rptxo has not exists at table sf5producers" );
       check( updri.has__dvdratio || updri.has__location,"error,sf5updprodri:all flag is false,no need update");
       check( updri.dvdratio >= 0 && updri.dvdratio <= 100, "error,sf5updprodri:ri.dvdratio must be in range [0, 100]" );

       //only dvdratio change,update last_top40bp_votes_change
       auto itr = std::get<1>(found_ret);
       txid_idx.modify(itr, get_self(), [&]( auto& row ) {
           if(updri.has__dvdratio)
           {
               if(row.ri.dvdratio != updri.dvdratio)
               {
                   row.ri.dvdratio = updri.dvdratio;
                   _gstate4vote.last_top40bp_votes_change = true;
               }
           }
           if(updri.has__location)
           {
               if(row.ri.location != updri.location){
                   row.ri.location = updri.location;
               }
           }
       });

       setnext(sfkey);
       eosio::print("sf5unregprod:update dvdration ",updri.has__dvdratio,",",updri.dvdratio,",location ",updri.has__location,","
                    ,updri.location,",last_top40bp_votes_change:",_gstate4vote.last_top40bp_votes_change);
   }

   void system_contract::update_bp_votes(bool add, const struct txokey& rptxokey,const double& votes,const struct txo& vtxo)
   {
       //1.update producer total vote,disabled bp also allow vote
       auto txid_idx = _sf5producers.get_index<"by3txid"_n>();
       auto found_txid_ret = findByTxo(txid_idx, rptxokey);
       auto producer_found = std::get<0>(found_txid_ret);
       check( producer_found, "error,update_bp_votes:rptxo has not exists at table sf5producers" );

       auto itr = std::get<1>(found_txid_ret);
       double producer_sf_vtotal = 0.0,producer_vtotal = 0.0;
       txid_idx.modify(itr, get_self(), [&]( auto& row ) {
           if(add)
           {
               row.sf_vtotal += votes;
               row.vtotal += votes;
           }else
           {
               row.sf_vtotal -= votes;
               row.vtotal -= votes;
               if(negative_dvalue_equals_zero(row.sf_vtotal)){
                   row.sf_vtotal = 0;
               }
               if(negative_dvalue_equals_zero(row.vtotal)){
                   row.vtotal = 0;
               }
               check(row.sf_vtotal>=0,"row.sf_vtotal less than 0");
               check(row.vtotal>=0,"row.sf_vtotal less than 0");
           }
           producer_sf_vtotal = row.sf_vtotal;
           producer_vtotal = row.vtotal;
       });

       //2.bp exists in top40,then update flag
       if(!_gstate4vote.last_top40bp_votes_change)
       {
            auto prods_txid_ret = findByTxo(_f3sf5prods.get_index<"by3txid"_n>(),rptxokey);
            auto exists = std::get<0>(prods_txid_ret);
            if(exists){
                _gstate4vote.last_top40bp_votes_change = true;
            }
       }

       eosio::print(add?"sf5vote":"sf5unvote"," [",rptxokey.txid,",",rptxokey.outidx,"],quantity:",vtxo.quantity,",type:",vtxo.type,
                    ",votes:",votes,",sf_vtotal:",producer_sf_vtotal,",vtotal:",producer_vtotal,",last_top40bp_votes_change:",
                    _gstate4vote.last_top40bp_votes_change,"\n");
   }

   void system_contract::sf5vote( const struct sf5key& sfkey, const struct txokey& rptxokey, const struct txo& vtxo )
   {
       auto found_ret = findByTxo(_sf5vtxo.get_index<"by3txid"_n>(), vtxo.key);
       auto found = std::get<0>(found_ret);
       check( found == false , "error,sf5vote:vtxo has exists at table sf5vtxo" );
       check( vtxo.type >=txo_type::type_mn && vtxo.type <= txo_type::type_other,"error,sf5vote:vtxo.type must be in range [0,1]" );
       check( vtxo.quantity.symbol == core_symbol(), "error,sf5vote:must use core symbol" );
       check( vtxo.quantity.is_valid(), "error,sf5vote:invalid quantity" );

       //1.insert safe vote info
       double vtotal = 0;
       if(vtxo.type == txo_type::type_mn)
       {
           eosio::print("amount:",vtxo.quantity.amount);
           check(vtxo.quantity.amount == 100000000000,"error,sf5vote:masternode vote amount must be 1000");
           vtotal = vtxo.quantity.amount*1.5;
       }else if(vtxo.type == txo_type::type_other){
           vtotal = vtxo.quantity.amount;
       }

       _sf5vtxo.emplace(get_self(), [&]( auto& row ) {
          row.prmrid     = _sf5vtxo.available_primary_key();
          row.rptxokey   = rptxokey;
          row.vtxo       = vtxo;
          row.vtotal     = vtotal;
       });

       //2.update bp vote
       update_bp_votes(true,rptxokey,vtotal,vtxo);

       setnext(sfkey);
   }

   void system_contract::sf5unvote( const struct sf5key& sfkey, const struct txokey& vtxokey )
   {
       auto txid_idx = _sf5vtxo.get_index<"by3txid"_n>();
       auto found_ret = findByTxo(txid_idx, vtxokey);
       auto found = std::get<0>(found_ret);
       check( found, "error,sf5unvote:vtxokey has not exists at table sf5vtxo" );

       //1.delete safe vote info
       auto itr = std::get<1>(found_ret);
       struct txo vtxo = itr->vtxo;
       double vtotal = itr->vtotal;
       struct txokey rptxokey = itr->rptxokey;
       txid_idx.erase(itr);

       //2.update bp vote
       update_bp_votes(false,rptxokey,vtotal,vtxo);

       setnext(sfkey);
   }

   void system_contract::sf5bindaccnt( const struct sf5key& sfkey, const struct sfaddress& sfaddr, const name& account )
   {
       require_auth(account);
       auto found_ret = findByUniqueIdx(_sfaddr2accnt.get_index<"by3sfaddr"_n>(),eosio::sha256( sfaddr.str.c_str(), sfaddr.str.length() ));
       auto found = std::get<0>(found_ret);
       check (found==false,("error, sf5bindaccnt:repeat bind,sfaddr " + sfaddr.str +" has exists at table sfaddr2accnt").data());

       _sfaddr2accnt.emplace(get_self(), [&]( auto& row ) {
           row.prmrid = _sfaddr2accnt.available_primary_key();
           row.sfaddr = sfaddr;
           row.account = account;
       });

       auto rewards_addr_idx = _rewards4v.get_index<"by3sfaddr"_n>();
       auto found_rewards_ret = findByUniqueIdx(rewards_addr_idx,eosio::sha256( sfaddr.str.c_str(), sfaddr.str.length() ));
       auto found_rewards = std::get<0>(found_rewards_ret);
       if(found_rewards)
       {
           auto itr = std::get<1>(found_rewards_ret);
           rewards_addr_idx.modify(itr, get_self(), [&]( auto& row ) {
               if(row.owner != account)
               {
                   row.owner = account;
                   eosio::print("update rewards4v owner succ.");
               }
           });
       }

       setnext(sfkey);
       eosio::print("sf5bindaccnt:bind safe addr ",sfaddr.str," to account ",account,".\n");
   }

   void system_contract::setnext( const struct sf5key& sfkey )
   {
       //check ( sfkey.atom_id == _gstate4vote.sf_atom_id +1,"sf5setnext,atom_id is not equal global sf_atom_id"); //XJTODO,remove annote
       ++_gstate4vote.sf_atom_id;
       _gstate4vote.sf_block_num = sfkey.next_block_num;
       _gstate4vote.sf_tx_index = sfkey.next_tx_index;

       eosio::print("sf5setnext:sf_atom_id:",_gstate4vote.sf_atom_id,",sf_block_num:",_gstate4vote.sf_block_num,",sf_tx_index:",_gstate4vote.sf_tx_index,";");
   }

   void system_contract::sf5setnext( const struct sf5key& sfkey )
   {
       setnext(sfkey);
   }

   //XJTODO for test
   void system_contract::resetg4vote()
   {
       _gstate4vote = get_default_global4vote();
       eosio::print("resetg4vote");
   }

   void system_contract::sf5pubkhash(const public_key& sc_pubkey)
   {
       eosio::print("pubkey hash:",eosio::sha256(sc_pubkey.data.begin(), sc_pubkey.data.size()),"\n");
   }

   void system_contract::regproducer2( const struct txokey& rptxokey, const name& account, const signature& newsig )
   {
       //1.rptxokey need be unique
       require_auth(account);
       auto txid_idx = _sf5producers.get_index<"by3txid"_n>();
       auto found_ret = findByTxo(txid_idx, rptxokey);
       auto found = std::get<0>(found_ret);
       check( found, "error,regproducer2:rptxo has exists at table sf5producers" );

       //2.account need be unique
       auto account_found_ret = findByUniqueIdx(_sf5producers.get_index<"by3owner"_n>(),account.value);
       auto account_found = std::get<0>(account_found_ret);
       check (account_found==false,"error, regproducer2:account has exists at table sf5producers");

       //3.update owner
       eosio::print("regproducer2:");
       auto itr = std::get<1>(found_ret);
       txid_idx.modify(itr, get_self(), [&]( auto& row ) {
           eosio::print("pubkey ",row.index_by_pubkey(),".");
           check(row.owner==name(0),"error,regproducer2:rptxo has alreay set owner");
           checksign(row.index_by_pubkey(),newsig,row.ri.sc_pubkey);
           row.owner = account;
       });

       eosio::print("bind rptxokey[",rptxokey.txid,",",rptxokey.outidx,"] with account ",account,"\n");
   }

   void system_contract::sc5vote( const name& voter, const name& producer )
   {
       //1.check producer exists
       require_auth(voter);
       auto owner_idx = _sf5producers.get_index<"by3owner"_n>();
       auto found_ret = findByUniqueIdx(owner_idx,producer.value);
       auto found = std::get<0>(found_ret);
       check( found, ( "error,sc5vote:producer " + producer.to_string() + " is not registered" ).data() );

       int64_t curr_staked = 60000000000;

       bool add_vote = true;
       bool repeat_vote = true;
       int64_t last_staked = 0;
       double last_producer_vtotal = 0.0,curr_producer_vtotal = 0.0;
       name last_producer = name(0);
       auto itr = _sc5voters.find(voter.value);
       if(itr == _sc5voters.end())
       {
           //2.add new vote
           _sc5voters.emplace(get_self(), [&]( auto& row ) {
               row.owner = voter;
               row.producer = producer;
               row.vote_tp = eosio::current_time_point();
               row.staked = curr_staked;
               row.vtotal = curr_staked;
           });
           repeat_vote = false;
       }else
       {
           //3.update vote
           add_vote = false;
           _sc5voters.modify(itr, get_self(), [&]( auto& row ) {
               last_producer = row.producer;
               //XJTODO,first stake 100,then vote,finally add new stake 100,may be add a undo_staked
               last_staked = row.staked;
               if(row.staked != curr_staked || last_producer!=producer)
               {
                   row.vote_tp = eosio::current_time_point();
                   row.staked = curr_staked;
                   row.producer = producer;
                   repeat_vote = false;
               }
           });

           //4.old_producer sub old_stake
           if(last_producer!=producer)
           {
               auto last_owner_idx = _sf5producers.get_index<"by3owner"_n>();
               auto last_found_ret = findByUniqueIdx(last_owner_idx,last_producer.value);
               auto last_found = std::get<0>(last_found_ret);
               check( last_found, ( "error,sc5vote:last producer "+last_producer.to_string()+" is not registered" ).data());
               auto itr_owner = std::get<1>(last_found_ret);
               last_owner_idx.modify(itr_owner, get_self(), [&]( auto& row ) {
                   check(row.vtotal>=last_staked,("error,sc5vote:vtotal less than last_staked,last producer:"+last_producer.to_string()).data());
                   row.vtotal -= last_staked;
                   last_producer_vtotal = row.vtotal;
               });
           }else
           {
               auto itr_owner = std::get<1>(found_ret);
               owner_idx.modify(itr_owner, get_self(), [&]( auto& row ) {
                   row.vtotal -= last_staked;
                   if(negative_dvalue_equals_zero(row.vtotal)){
                       row.vtotal = 0;
                   }
                   check(row.vtotal>=0,"row.sf_vtotal less than 0");
                   last_producer_vtotal = row.vtotal;
               });
           }

           if(!_gstate4vote.last_top40bp_votes_change)
           {
               if(!repeat_vote&&
                   (std::get<0>(findByUniqueIdx(_f3sf5prods.get_index<"by3owner"_n>(),last_producer.value))||
                   std::get<0>(findByUniqueIdx(_f3sf5prods.get_index<"by3owner"_n>(),producer.value))))
               {
                   _gstate4vote.last_top40bp_votes_change = true;
               }
           }
       }

       //5.curr_producer add curr_stake
       auto itr_owner = std::get<1>(found_ret);
       owner_idx.modify(itr_owner, get_self(), [&]( auto& row ) {
           row.vtotal += curr_staked;
           curr_producer_vtotal = row.vtotal;
       });

       eosio::print("sc5vote:",add_vote?"add vote":"update vote",",repeat_vote:",repeat_vote,",voter:",voter.to_string(),",last producer:",
                    last_producer.to_string(),",last staked:",last_staked,",vtotal:",last_producer_vtotal,",new producer:",
                    producer.to_string(),",new staked:",curr_staked,",vtotal:",curr_producer_vtotal,",last_top40bp_votes_change:",
                    _gstate4vote.last_top40bp_votes_change,"\n");
   }

   void system_contract::claim4prod( const name& producer )
   {
        require_auth(producer);
        auto found_ret = findByUniqueIdx(_sf5producers.get_index<"by3owner"_n>(),producer.value);
        auto found = std::get<0>(found_ret);
        check( found, ( "producer " + producer.to_string() + " is not registered" ).data() );

        auto itr = _rewards4bp.find(producer.value);
        auto unclaimed = 0;
        auto period = 0;
        if(itr != _rewards4bp.end())
        {
            eosio::token::transfer_action transfer_act{ token_account, { {bpay_account, active_permission} } };
            _rewards4bp.modify(itr, get_self(), [&]( auto& row ) {
                period = row.period;
                if(row.unclaimed>0)
                {
                    transfer_act.send( bpay_account, producer, asset(row.unclaimed, core_symbol()), "fund per-block bucket" );
                    unclaimed = row.unclaimed;
                    row.unclaimed = 0;
                }else
                {
                    eosio::print("claim4prod:producer ",producer," has no unclaimed,the period is ",period,"\n");
                }
            });
        }
        if(unclaimed>0){
            eosio::print("claim4prod:producer claim ",unclaimed,",the period is ",period,"\n");
        }
   }

   void system_contract::claim4vote( const name& voter )
   {
        require_auth(voter);
        auto owner_idx = _rewards4v.get_index<"by3owner"_n>();
        auto found_ret = findByUniqueIdx(owner_idx, voter.value);
        auto found = std::get<0>(found_ret);
        check( found, "error, claim4vote:voter has not exists at table rewards4v" );

        auto unclaimed = 0;
        auto period = 0;
        auto itr = std::get<1>(found_ret);
        owner_idx.modify(itr, get_self(), [&]( auto& row ) {
            period = row.period;
            if(row.unclaimed>0)
            {
                eosio::token::transfer_action transfer_act{ token_account, { {vpay_account, active_permission} } };
                transfer_act.send( vpay_account, voter, asset(row.unclaimed, core_symbol()), "fund per-vote bucket" );
                unclaimed = row.unclaimed;
                row.unclaimed = 0;
            }else
            {
                eosio::print("claim4vote:voter ",voter," has no unclaimed,the period is ",period,"\n");
            }
        });
        if(unclaimed>0){
            eosio::print("claim4vote:voter claim ",unclaimed,",the period is ",period,"\n");
        }
   }

   void system_contract::checksign( const eosio::checksum256& digest, const eosio::signature& sig, const eosio::public_key& pubkey )
   {
      assert_recover_key( digest, sig, pubkey );
   }

   void system_contract::clear_f3_tables()
   {
       for( auto itr = _f3sf5prods.begin(); itr != _f3sf5prods.end(); ){
           itr = _f3sf5prods.erase(itr);
       }
       for( auto itr = _f3sf5vtxo.begin(); itr != _f3sf5vtxo.end(); ){
           itr = _f3sf5vtxo.erase(itr);
       }
       for( auto itr = _f3voters.begin(); itr != _f3voters.end(); ){
           itr = _f3voters.erase(itr);
       }
   }

   void system_contract::copy_data_from_s3_to_p3(const uint32_t& reward_producer_count,std::map<double,eosio::producer_key>& sftop21_producers_map,
                                                 uint32_t& top40_producer_count)
   {
       //1.sort by sf_vtotal,copy top40 from sf5producers to f3sf5prods
       std::set<txokey> producers_key_set;
       std::set<name> producers_name_set;

       const auto& producers_idx = _sf5producers.get_index<"by3sf5vtotal"_n>();
       uint32_t top_21_producer_count=0,block_producer_count=21;
       for(auto itr_producers = producers_idx.rbegin(); itr_producers != producers_idx.rend(); ++itr_producers)
       {
           //no bind bp,unreg bp,no votes bp all filter
           if(itr_producers->owner.value==0||!itr_producers->enable||!dvalue_bigger_than_zero(itr_producers->sf_vtotal)){
               continue;
           }
           _f3sf5prods.emplace(get_self(), [&]( auto& row ) {
               row.owner      = itr_producers->owner;
               row.rptxokey   = itr_producers->rptxokey;
               row.dvdratio   = itr_producers->ri.dvdratio;
               row.vtotal     = itr_producers->vtotal;
            });
           producers_key_set.insert(itr_producers->rptxokey);
           producers_name_set.insert(itr_producers->owner);
           if(++top_21_producer_count<=block_producer_count&&dvalue_bigger_than_zero(itr_producers->vtotal)){
               eosio::print("sf",top_21_producer_count,"[",itr_producers->owner,",",itr_producers->sf_vtotal,"],");
               sftop21_producers_map[itr_producers->vtotal] = {itr_producers->owner,itr_producers->ri.sc_pubkey};
           }
           if(++top40_producer_count>=reward_producer_count){
               break;
           }
       }

       //2.copy votes from sf5vtxo to f3sf5vtxo
       for( auto itr_vtxo = _sf5vtxo.begin(); itr_vtxo != _sf5vtxo.end(); ++itr_vtxo)
       {
           if(producers_key_set.count(itr_vtxo->rptxokey)==0){
               continue;
           }
           _f3sf5vtxo.emplace(get_self(), [&]( auto& row ) {
               row.prmrid   = _f3sf5vtxo.available_primary_key();
               row.rptxokey = itr_vtxo->rptxokey;
               row.vtxokey  = itr_vtxo->vtxo.key;
               row.sfaddr   = itr_vtxo->vtxo.owner;
               row.vote_tp  = itr_vtxo->vtxo.tp;
               row.vtotal   = itr_vtxo->vtotal;
           });
       }

       //3.copy voters from sc5voters to f3voters
       for( auto itr_voters = _sc5voters.begin(); itr_voters != _sc5voters.end(); ++itr_voters)
       {
           if(producers_name_set.count(itr_voters->producer)==0){
               continue;
           }
           _f3voters.emplace(get_self(), [&]( auto& row ) {
               row.owner            = itr_voters->owner;
               row.proxy            = itr_voters->proxy;
               row.producer         = itr_voters->producer;
               row.staked           = itr_voters->staked;
               row.vote_tp          = itr_voters->vote_tp;
               row.vtotal           = itr_voters->vtotal;
               row.proxied_vtotal   = itr_voters->proxied_vtotal;
               row.is_proxy         = itr_voters->is_proxy;
               row.flags1           = itr_voters->flags1;
               row.reserved2        = itr_voters->reserved2;
               row.reserved3        = itr_voters->reserved3;
           });
       }
   }

   void system_contract::set_sc_proposed_producers(const std::map<double,eosio::producer_key>& sftop21_producers_map,
                                                   const uint32_t& top40_producer_count,bool& soft_trigger_calc_reward)
   {
       if(top40_producer_count>0)
       {
           std::vector<eosio::producer_key> producers;
           producers.reserve(sftop21_producers_map.size());
           uint32_t index = 0;
           for(auto itr_producer = sftop21_producers_map.cbegin();itr_producer!=sftop21_producers_map.cend();++itr_producer)
           {
               eosio::print("sc",++index,"[",itr_producer->second.producer_name,",",itr_producer->first,"],");
               producers.push_back(itr_producer->second);
           }

           std::optional<uint64_t> ret = set_proposed_producers( producers );
           eosio::print("set proposed sc producers ret:",ret?*ret:0,",");
           if(ret){
               _gstate4vote.curr_prods_count = sftop21_producers_map.size();
           }
           if( _gstate4vote.last_top40bp_votes_change && !ret )
           {
               _gstate4vote.soft_trigger_calc_reward = true;
               soft_trigger_calc_reward = true;
               eosio::print("producers not changed，but votes or dvdratio change,set _gstate4vote.soft_trigger_calc_reward true,");
           }
       }else
       {
           eosio::print("no sc producer.may be first loop.");
       }

       _gstate4vote.last_top40bp_votes_change = false;
   }

   void system_contract::update_sc_elected_producers(const uint32_t& reward_producer_count,bool& soft_trigger_calc_reward)
   {
       //1.clear f3sf5 tables
       clear_f3_tables();

       //2.copy data from s3 to f3
       std::map<double,eosio::producer_key> sftop21_producers_map;
       uint32_t top40_producer_count=0;
       copy_data_from_s3_to_p3(reward_producer_count,sftop21_producers_map,top40_producer_count);

       //3.set proposed producers
       set_sc_proposed_producers(sftop21_producers_map,top40_producer_count,soft_trigger_calc_reward);
   }

   void system_contract::update_s3sf5(const uint32_t& block_time,bool& soft_trigger_calc_reward)
   {
       //3 240;5 360;21 900
       uint32_t wait_interval = _gstate4vote.curr_prods_count*3*12+30;//+150
       wait_interval = 0;
       if(block_time - _gstate4vote.last_set_proposed_producers_timestamp < wait_interval){
           return;
       }

       bool need_new_sch_prods = false;
       uint32_t reward_producer_count=40;
       auto last_top40bp_votes_change_tmp = _gstate4vote.last_top40bp_votes_change;
       eosio::print("update_s3sf5:");
       if(_gstate4vote.last_top40bp_votes_change)
       {
           need_new_sch_prods = true;
           eosio::print("last_top40bp_votes_change no change.");
       }else
       {
           //compare sf5producers with f3sf5prods top40 bp
           const auto& producers_idx = _sf5producers.get_index<"by3vtotal"_n>();
           auto itr_producers = producers_idx.rbegin();
           const auto& prods_idx = _f3sf5prods.get_index<"by3vtotal"_n>();
           auto itr_prods = prods_idx.rbegin();
           uint32_t same_producer_count = 0;
           while(itr_producers!=producers_idx.rend()&&itr_prods!=prods_idx.rend())
           {
               if(itr_producers->owner.value==0||!itr_producers->enable||!dvalue_bigger_than_zero(itr_producers->sf_vtotal))
               {
                   ++itr_producers;
                   continue;
               }
               if(itr_prods->owner.value==0||!dvalue_bigger_than_zero(itr_producers->vtotal))//in normal,owner can't be 0
               {
                   ++itr_prods;
                   continue;
               }
               if(itr_producers->owner!=itr_prods->owner)
               {
                   need_new_sch_prods = true;
                   eosio::print("top40 sf5producers not equals to f3sf5prods.");
                   break;
               }
               ++itr_producers;
               ++itr_prods;
               if(++same_producer_count>=reward_producer_count){
                   break;
               }
           }
           if(same_producer_count==0)
           {
               need_new_sch_prods = true;
               eosio::print("same bp count is 0.");
           }
           if(!need_new_sch_prods)
           {
               //example:sf5producers bp(3,2,1),f3sf5prods(3,2)
               while(itr_producers!=producers_idx.rend())
               {
                   if(itr_producers->owner.value==0||!itr_producers->enable||!dvalue_bigger_than_zero(itr_producers->sf_vtotal))
                   {
                       ++itr_producers;
                       continue;
                   }
                   need_new_sch_prods = true;
                   eosio::print("f3sf5prods is subset of sf5producers,but sf5producers has more valid producers.");
                   break;
               }
           }
       }

       if(need_new_sch_prods){
           update_sc_elected_producers(reward_producer_count,soft_trigger_calc_reward);
       }

       _gstate4vote.last_set_proposed_producers_timestamp = block_time;
       eosio::print("last_set_proposed_producers_timestamp:",_gstate4vote.last_set_proposed_producers_timestamp,",last_top40bp_votes_change_tmp:",
                    last_top40bp_votes_change_tmp,",need_new_sch_prods:",need_new_sch_prods);
   }

   void system_contract::record_block_rewards(const name& producer,const uint32_t& block_time)
   {
       auto owner_idx = _p3sf5prods.get_index<"by3owner"_n>();
       auto found_ret = findByUniqueIdx(owner_idx,producer.value);
       auto found = std::get<0>(found_ret);
       if(found)
       {
           auto itr_owner = std::get<1>(found_ret);
           auto unpaid_block = 0;
           owner_idx.modify(itr_owner, get_self(), [&]( auto& row ) {
               row.unpaid_block += 1;
               unpaid_block = row.unpaid_block;
           });
           _gstate4vote.last_unpaid_block += 1;
           auto ynr = (block_time - _gstate4vote.active_timestamp)/(3600*24*360);
           auto itr = _year3rewards.find(ynr);
           if(itr != _year3rewards.end()){
               _gstate4vote.last_unpaid_rewards += itr->amount;
           }
       }
   }

   void system_contract::clear_p3_tables()
   {
       for( auto itr = _p3sf5prods.begin(); itr != _p3sf5prods.end(); ){
           itr = _p3sf5prods.erase(itr);
       }
       for( auto itr = _p3sf5vtxo.begin(); itr != _p3sf5vtxo.end(); ){
           itr = _p3sf5vtxo.erase(itr);
       }
       for( auto itr = _p3voters.begin(); itr != _p3voters.end(); ){
           itr = _p3voters.erase(itr);
       }
   }

   void system_contract::copy_data_from_f3_to_p3()
   {
       //1.copy prods from f3sf5prods to p3sf5prods
       for( auto itr_prods = _f3sf5prods.begin(); itr_prods != _f3sf5prods.end(); ++itr_prods)
       {
           _p3sf5prods.emplace(get_self(), [&]( auto& row ) {
               row.owner        = itr_prods->owner;
               row.rptxokey     = itr_prods->rptxokey;
               row.dvdratio     = itr_prods->dvdratio;
               row.unpaid_block = 0;
           });
       }

       //2.copy vtxo from f3sf5vtxo to p3sf5vtxo
       for( auto itr_vtxo = _f3sf5vtxo.begin(); itr_vtxo != _f3sf5vtxo.end(); ++itr_vtxo)
       {
           _p3sf5vtxo.emplace(get_self(), [&]( auto& row ) {
               row.prmrid   = itr_vtxo->prmrid;
               row.rptxokey = itr_vtxo->rptxokey;
               row.vtxokey  = itr_vtxo->vtxokey;
               row.sfaddr   = itr_vtxo->sfaddr;
               row.vote_tp  = itr_vtxo->vote_tp;
               row.vtotal   = itr_vtxo->vtotal;
           });
       }

       //3.copy voters from f3voters to p3voters
       for( auto itr_voters = _f3voters.begin(); itr_voters != _f3voters.end(); ++itr_voters)
       {
           _p3voters.emplace(get_self(), [&]( auto& row ) {
               row.owner            = itr_voters->owner;
               row.proxy            = itr_voters->proxy;
               row.producer         = itr_voters->producer;
               row.staked           = itr_voters->staked;
               row.vote_tp          = itr_voters->vote_tp;
               row.vtotal           = itr_voters->vtotal;
               row.proxied_vtotal   = itr_voters->proxied_vtotal;
               row.is_proxy         = itr_voters->is_proxy;
           });
       }
   }

   void system_contract::update_bpay_rewards(name owner,const double& bpay,const bool& reward_unclaimed_ret)
   {
       auto itr_rewards4bp = _rewards4bp.find(owner.value);
       if(itr_rewards4bp == _rewards4bp.end())
       {
           _rewards4bp.emplace(get_self(), [&]( auto& row ) {
               row.owner = owner;
               if(reward_unclaimed_ret){
                   row.period = 0;
                   row.unclaimed = bpay;
               }else{
                   row.period = bpay;
               }
           });
       }else
       {
           _rewards4bp.modify(itr_rewards4bp, get_self(), [&]( auto& row ) {
               row.period += bpay;
               if(reward_unclaimed_ret)
               {
                   row.unclaimed += row.period;
                   row.period = 0;
               }
           });
       }
   }

   void system_contract::update_vpay_rewards(name owner,const double& vpay,const bool& reward_unclaimed_ret,const vote_pay_type& vpay_type,const sfaddress& sfaddr)
   {

       bool found_vote_rewards = false;
       bool found_account = true;
       if(vpay_type == vote_pay_type::type_sf_voter)
       {
           auto sfaddr_idx = _rewards4v.get_index<"by3sfaddr"_n>();
           auto found_sfaddr_ret = findByUniqueIdx(sfaddr_idx,eosio::sha256( sfaddr.str.c_str(), sfaddr.str.length()));
           found_vote_rewards = std::get<0>(found_sfaddr_ret);
           found_account = false;
           if(found_vote_rewards)
           {
               //update sf vote rewards
               auto itr_rewards4v = std::get<1>(found_sfaddr_ret);
               sfaddr_idx.modify(itr_rewards4v, get_self(), [&]( auto& row ) {
                   row.period += vpay;
                   if(reward_unclaimed_ret)
                   {
                       row.unclaimed += row.period;
                       row.period = 0;
                   }
                   eosio::print("update sf voter:",sfaddr.str,".");
               });
               return;
           }
       }else
       {
           auto owner_idx = _rewards4v.get_index<"by3owner"_n>();
           auto found_owner_ret = findByUniqueIdx(owner_idx,owner.value);
           found_vote_rewards = std::get<0>(found_owner_ret);
           if(found_vote_rewards)
           {
               //update prod or voter vote rewards
               auto itr_rewards4v = std::get<1>(found_owner_ret);
               owner_idx.modify(itr_rewards4v, get_self(), [&]( auto& row ) {
                   row.period += vpay;
                   if(reward_unclaimed_ret)
                   {
                       row.unclaimed += row.period;
                       row.period = 0;
                   }
                   eosio::print("update sc voter:",owner,".");
               });
               return;
           }
           if(vpay_type == vote_pay_type::type_sc_voter)
           {
               auto found_account_ret = findByUniqueIdx(_sfaddr2accnt.get_index<"by3account"_n>(),owner.value);
               found_account = std::get<0>(found_account_ret);
           }
       }

       eosio::print("add voter:",(uint32_t)vpay_type,".");
       //add new vote pay
       _rewards4v.emplace(get_self(), [&]( auto& row ) {
          row.prmrid = _rewards4v.available_primary_key();
          row.sfaddr = sfaddr;
          if(found_account){
              row.owner = owner;
          }else{
              row.owner = name(0);
          }

          if(reward_unclaimed_ret)
          {
              row.unclaimed = vpay;
              row.period = 0;
          }else
          {
              row.period = vpay;
              row.unclaimed = 0;
          }
       });
   }

   template<typename T>
   void system_contract::calculate_voters_reward(T& sc_voters_pay_map,std::map<name,std::pair<double,double>>& pay_to_voters_map)
   {
       for( auto itr = sc_voters_pay_map.begin(); itr != sc_voters_pay_map.end(); ++itr )
       {
           voters_pay& voter_pay = itr->second;
           if(pay_to_voters_map.count(voter_pay.producer)<=0)
           {
               eosio::print("calculate voters pay,producer:",voter_pay.producer," not found");
               continue;
           }
           double prod_pay_to_voters = pay_to_voters_map[voter_pay.producer].first;
           double prod_coinage = pay_to_voters_map[voter_pay.producer].second;
           voter_pay.voter_vpay = 0.0;
           if(prod_coinage>0){
               voter_pay.voter_vpay = prod_pay_to_voters * voter_pay.vote_coinage / prod_coinage;
           }
           eosio::print("voter:",itr->first,":voter_vpay:",(uint64_t)voter_pay.voter_vpay/COIN,",vote_age:",voter_pay.vote_age,",vote_coinage:",
                        (uint64_t)voter_pay.vote_coinage/COIN,",prod_coinage:",(uint64_t)prod_coinage/COIN,".");
       }
   }

   void system_contract::calculate_rewards(const uint32_t& block_time,totals_pay& total_pay,std::map<std::string,voters_pay>& sf_voters_pay_map
                                           ,std::map<txokey,prods_pay>& prods_bpay_map,std::map<name,voters_pay>& sc_voters_pay_map)
   {
       //total block pay
       total_pay.total_bpay = _gstate4vote.last_unpaid_rewards * 0.25;
       //total vote pay
       total_pay.total_vpay = _gstate4vote.last_unpaid_rewards * 0.75;
       //per block pay
       double per_bpay = total_pay.total_bpay / _gstate4vote.last_unpaid_block;

       eosio::print(",last_unpaid_rewards:",_gstate4vote.last_unpaid_rewards/COIN,",last_unpaid_block:",_gstate4vote.last_unpaid_block,
                    ",total_bpay:",(uint64_t)total_pay.total_bpay/COIN,",total_vpay:",(uint64_t)total_pay.total_vpay/COIN,",per_bpay:",
                    per_bpay/COIN,".");

       //calculate prods block pay
       for( auto itr = _p3sf5prods.begin(); itr != _p3sf5prods.end(); ++itr )
       {
           auto& prod_pay = prods_bpay_map[itr->rptxokey];
           prod_pay.prod_bpay = itr->unpaid_block * per_bpay;
           prod_pay.dvdratio = itr->dvdratio;
           prod_pay.owner = itr->owner;
           prod_pay.unpaid_block = itr->unpaid_block;
       }

       //calculate prods and unbind sf_voters coinage
       double total_coinage=0.0;
       for( auto itr = _p3sf5vtxo.begin(); itr != _p3sf5vtxo.end(); ++itr )
       {
           auto vote_time = itr->vote_tp.sec_since_epoch();
           check(block_time>=vote_time,"error,vote_time bigger than block_time");
           uint32_t vote_age = block_time-vote_time;
           double coinage = vote_age*itr->vtotal;
           if(prods_bpay_map.count(itr->rptxokey)<=0){
               continue;
           }
           prods_pay& prod_pay = prods_bpay_map[itr->rptxokey];
           prod_pay.prod_coinage += coinage;
           //prod_pay.sfaddr = itr->sfaddr;
           voters_pay& voter_pay = sf_voters_pay_map[itr->sfaddr.str];
           voter_pay.producer = prod_pay.owner;
           voter_pay.vote_coinage = coinage;
           voter_pay.vote_age = vote_age;

           total_coinage += coinage;
       }

       //calculate voters coinage
       std::map<name,double> prods_sc_coinage_map;
       for( auto itr = _p3voters.begin(); itr != _p3voters.end(); ++itr )
       {
           auto last_vote_time = itr->vote_tp.sec_since_epoch();
           check(block_time>=last_vote_time,"error,last_vote_time bigger than block_time");
           uint32_t vote_age = block_time-last_vote_time;
           double vote_coinage = vote_age*itr->vtotal;
           total_coinage += vote_coinage;
           voters_pay& voter_pay = sc_voters_pay_map[itr->owner];
           voter_pay.producer = itr->producer;
           voter_pay.vote_coinage = vote_coinage;
           voter_pay.vote_age = vote_age;
           if(prods_sc_coinage_map.count(itr->producer)==0){
               prods_sc_coinage_map[itr->producer] = vote_coinage;
           }else{
               prods_sc_coinage_map[itr->producer] += vote_coinage;
           }
       }

       //calculate prods pay
       std::map<name,std::pair<double,double>> pay_to_voters_map;
       for( auto itr = prods_bpay_map.begin(); itr != prods_bpay_map.end(); ++itr )
       {
           prods_pay& prod_pay = itr->second;
           prod_pay.prod_vpay = 0.0;
           //update sc coinage
           if(prods_sc_coinage_map.count(prod_pay.owner)>0){
               prod_pay.prod_coinage +=  prods_sc_coinage_map[prod_pay.owner];
           }
           if(total_coinage>0){
               prod_pay.prod_vpay = total_pay.total_vpay*prod_pay.prod_coinage/total_coinage;
           }
           prod_pay.prod_total_pay = prod_pay.prod_bpay + prod_pay.prod_vpay;
           prod_pay.prod_bpay_to_self = prod_pay.prod_bpay*(100-prod_pay.dvdratio)/100;
           total_pay.total_prod_bpay_to_self += prod_pay.prod_bpay_to_self;
           prod_pay.prod_vpay_to_self = prod_pay.prod_vpay*(100-prod_pay.dvdratio)/100;
           total_pay.total_prod_vpay_to_self += prod_pay.prod_vpay_to_self;
           prod_pay.prod_pay_to_voters = prod_pay.prod_total_pay - prod_pay.prod_bpay_to_self - prod_pay.prod_vpay_to_self;
           if(negative_dvalue_equals_zero(prod_pay.prod_pay_to_voters)){
               prod_pay.prod_pay_to_voters = 0;
           }
           check(prod_pay.prod_pay_to_voters>=0,"prod_pay_to_voters less than 0");
           total_pay.total_prod_pay_to_voters += prod_pay.prod_pay_to_voters;
           pay_to_voters_map[prod_pay.owner] = std::make_pair(prod_pay.prod_pay_to_voters,prod_pay.prod_coinage);
           eosio::print(prod_pay.owner,":unpaid block:",prod_pay.unpaid_block,",prod_total_pay:",(uint64_t)prod_pay.prod_total_pay/COIN,",bpay_to_self:",
                        (uint64_t)prod_pay.prod_bpay_to_self/COIN,",vpay_to_self:",(uint64_t)prod_pay.prod_vpay_to_self/COIN,
                        ",pay_to_voters:",(uint64_t)prod_pay.prod_pay_to_voters/COIN,",dvdratio:",prod_pay.dvdratio,".");
       }

       //calculate unbind voters pay
       calculate_voters_reward(sf_voters_pay_map,pay_to_voters_map);

       //calculate voters pay
       calculate_voters_reward(sc_voters_pay_map,pay_to_voters_map);

       eosio::print(",total_coinage:",(uint64_t)total_coinage/COIN,".");
   }

   void system_contract::update_prods_rewards(std::map<txokey,prods_pay>& prods_bpay_map,const bool& reward_unclaimed_ret,
                                              const totals_pay& total_pay)
   {
       uint32_t prods_bpay_size = prods_bpay_map.size(),prods_count = 0;
       double tmp_prod_bpay_to_self = 0.0,tmp_prod_vpay_to_self = 0.0;
       for( auto itr = prods_bpay_map.begin(); itr != prods_bpay_map.end(); ++itr )
       {
           ++prods_count;
           prods_pay& prod_pay = itr->second;
           eosio::print("bpvpay:",prod_pay.owner,",");
           double prod_bpay_to_self = 0.0,prod_vpay_to_self = 0.0;
           if(prods_count<prods_bpay_size)
           {
               prod_bpay_to_self = prod_pay.prod_bpay_to_self;
               prod_vpay_to_self = prod_pay.prod_vpay_to_self;
               tmp_prod_bpay_to_self += prod_bpay_to_self;
               tmp_prod_vpay_to_self += prod_vpay_to_self;
           }else
           {
               prod_bpay_to_self = total_pay.total_prod_bpay_to_self - tmp_prod_bpay_to_self;
               if(negative_dvalue_equals_zero(prod_bpay_to_self)){
                   prod_bpay_to_self = 0;
               }
               check(prod_bpay_to_self>=0,"prod_bpay_to_self less than 0");
               prod_vpay_to_self = total_pay.total_prod_vpay_to_self - tmp_prod_vpay_to_self;
               if(negative_dvalue_equals_zero(prod_vpay_to_self)){
                   prod_vpay_to_self = 0;
               }
               if(prod_vpay_to_self<0){
                   eosio::print("total_prod_bpay_to_self:",total_pay.total_prod_bpay_to_self,",tmp_prod_bpay_to_self:",tmp_prod_bpay_to_self,
                                ",prod_bpay_to_self:",prod_bpay_to_self,",prod_pay.prod_bpay_to_self:",prod_pay.prod_bpay_to_self,
                                ",total_prod_vpay_to_self:",total_pay.total_prod_vpay_to_self,",tmp_prod_vpay_to_self:",tmp_prod_vpay_to_self,
                                ",prod_vpay_to_self:",prod_vpay_to_self,",prod_pay.prod_vpay_to_self:",prod_pay.prod_vpay_to_self);
               }
               check(prod_vpay_to_self>=0,"prod_vpay_to_self less than 0");
           }
           update_bpay_rewards(prod_pay.owner,prod_bpay_to_self,reward_unclaimed_ret);
           update_vpay_rewards(prod_pay.owner,prod_vpay_to_self,reward_unclaimed_ret,vote_pay_type::type_prod,prod_pay.sfaddr);
       }
   }

   void system_contract::update_sf_voters_rewards(const std::map<std::string,voters_pay>& sf_voters_pay_map,const bool& reward_unclaimed_ret,
                          const totals_pay& total_pay)
   {
       uint32_t voters_pay_size = sf_voters_pay_map.size(),voters_count = 0;
       double tmp_prod_pay_to_voters = 0.0;
       for( auto itr = sf_voters_pay_map.begin(); itr != sf_voters_pay_map.end(); ++itr )
       {
           ++voters_count;
           sfaddress sfaddr;
           sfaddr.str = itr->first;
           const voters_pay& voter_pay = itr->second;
           double voter_vpay = 0.0;
           if(voters_count<voters_pay_size)
           {
               tmp_prod_pay_to_voters += voter_pay.voter_vpay;
               voter_vpay = voter_pay.voter_vpay;
           }else
           {
               voter_vpay = total_pay.total_prod_pay_to_voters - tmp_prod_pay_to_voters;
               if(negative_dvalue_equals_zero(voter_vpay)){
                   voter_vpay = 0;
               }
               if(voter_vpay<0){
                   eosio::print("total_prod_pay_to_voters:",total_pay.total_prod_pay_to_voters,",tmp_prod_pay_to_voters:",tmp_prod_pay_to_voters,
                                ",voter_vpay:",voter_vpay);
               }
               check(voter_vpay>=0,"voter_vpay less than 0");
           }
           update_vpay_rewards(name(0),voter_vpay,reward_unclaimed_ret,vote_pay_type::type_sf_voter,sfaddr);
       }
   }

   void system_contract::update_sc_voters_rewards(const std::map<name,voters_pay>& sc_voters_pay_map,const bool& reward_unclaimed_ret,const totals_pay& total_pay)
   {
       uint32_t voters_pay_size = sc_voters_pay_map.size(),voters_count = 0;
       double tmp_prod_pay_to_voters = 0.0;
       for( auto itr = sc_voters_pay_map.begin(); itr != sc_voters_pay_map.end(); ++itr )
       {
           ++voters_count;
           name owner = itr->first;
           const voters_pay& voter_pay = itr->second;
           double voter_vpay = 0.0;
           if(voters_count<voters_pay_size)
           {
               tmp_prod_pay_to_voters += voter_pay.voter_vpay;
               voter_vpay = voter_pay.voter_vpay;
           }else
           {
               voter_vpay = total_pay.total_prod_pay_to_voters - tmp_prod_pay_to_voters;
               if(negative_dvalue_equals_zero(voter_vpay)){
                   voter_vpay = 0;
               }
               if(voter_vpay<0){
                   eosio::print("total_prod_pay_to_voters:",total_pay.total_prod_pay_to_voters,",tmp_prod_pay_to_voters:",tmp_prod_pay_to_voters,
                                ",voter_vpay:",voter_vpay);
               }
               check(voter_vpay>=0,"voter_vpay less than 0");
           }
           update_vpay_rewards(owner,voter_vpay,reward_unclaimed_ret,vote_pay_type::type_sc_voter,sfaddress());
       }
   }

   void system_contract::transfer_pay(const totals_pay& total_pay)
   {
       if(_gstate4vote.last_unpaid_rewards<=0){
           return;
       }
       {
           //issue last_unpaid_rewards
           eosio::token::issue_action issue_act{ token_account, { {get_self(), active_permission} } };
           issue_act.send( get_self(), asset(_gstate4vote.last_unpaid_rewards, core_symbol()), "issue tokens for block pay and vote" );
       }
       {
           //transfer total_bpay and total_vpay
           eosio::token::transfer_action transfer_vpay_act{ token_account, { {get_self(), active_permission} } };
           transfer_vpay_act.send( get_self(), bpay_account, asset(total_pay.total_bpay, core_symbol()), "fund per-block bucket" );
           eosio::token::transfer_action transfer_bpay_act{ token_account, { {get_self(), active_permission} } };
           transfer_bpay_act.send( get_self(), vpay_account, asset(total_pay.total_vpay, core_symbol()), "fund per-vote bucket" );
       }
   }

   void system_contract::settlement_rewards(const uint32_t& schedule_version,const uint32_t& block_time,const bool& soft_trigger_calc_reward)
   {
       eosio::print(" settlement_rewards. ");
       bool sch_ver_ret = _gstate4vote.last_sch_ver != schedule_version;
       bool trigger_calc_ret = !soft_trigger_calc_reward&&_gstate4vote.soft_trigger_calc_reward;
       bool calc_rewards_ret = _gstate4vote.last_calc_rewards_timestamp>0 && block_time - _gstate4vote.last_calc_rewards_timestamp > WEEK_SEC;
       bool reward_unclaimed_ret = false;
       auto last_claim_week = (block_time - _gstate4vote.active_timestamp) / WEEK_SEC;
       if(_gstate4vote.last_claim_week != last_claim_week)
       {
           if(last_claim_week<_gstate4vote.last_claim_week){
               eosio::print("block_time:",block_time,",active_timestamp:",_gstate4vote.active_timestamp,".");
           }
           check(last_claim_week>=_gstate4vote.last_claim_week,"last_claim_week less than _gstate4vote.last_claim_week");
           _gstate4vote.last_claim_week = last_claim_week;
           reward_unclaimed_ret = true;
       }

       bool start_settlement = sch_ver_ret || trigger_calc_ret || calc_rewards_ret || reward_unclaimed_ret;
       if(!start_settlement){
           return;
       }
       eosio::print("calculate pay,version_ret:",sch_ver_ret,",trigger_calc_ret:",trigger_calc_ret,",calc_rewards_ret:",reward_unclaimed_ret,
                    ",reward_unclaimed_ret:",reward_unclaimed_ret,",schedule_version:",schedule_version);

       //1.calculate total,prods,voters rewards
       std::map<std::string,voters_pay> sf_voters_pay_map;
       std::map<txokey,prods_pay> prods_bpay_map;
       std::map<name,voters_pay> sc_voters_pay_map;
       totals_pay total_pay;
       calculate_rewards(block_time,total_pay,sf_voters_pay_map,prods_bpay_map,sc_voters_pay_map);

       //2.clear p3sf5 tables
       clear_p3_tables();

       //3.code data from f3sf5 to p3sf5 tables
       copy_data_from_f3_to_p3();

       //4.update prods pay rewards
       update_prods_rewards(prods_bpay_map,reward_unclaimed_ret,total_pay);

       //5.update sf_unbind voter vote pay rewards
       update_sf_voters_rewards(sf_voters_pay_map,reward_unclaimed_ret,total_pay);

       //6.update voter vote pay rewards
       update_sc_voters_rewards(sc_voters_pay_map,reward_unclaimed_ret,total_pay);

       //7.issue and transfer pay
       transfer_pay(total_pay);

       _gstate4vote.last_sch_ver = schedule_version;
       _gstate4vote.last_calc_rewards_timestamp = block_time;
       if(trigger_calc_ret){
           _gstate4vote.soft_trigger_calc_reward = false;
       }
       _gstate4vote.last_unpaid_rewards = 0;
   }

   void system_contract::update_p3sf5(const uint32_t& schedule_version,const uint32_t& block_time,const bool& soft_trigger_calc_reward,
                                      const name& producer)
   {
       record_block_rewards(producer,block_time);
       settlement_rewards(schedule_version,block_time,soft_trigger_calc_reward);
   }

   //XJTODO,for test,debug
   void system_contract::sc3onblock(const uint32_t& schedule_version)
   {
       uint32_t block_time = eosio::current_time_point().sec_since_epoch();
       bool soft_trigger_calc_reward = false;
       update_s3sf5(block_time,soft_trigger_calc_reward);
       eosio::name producer{"bp1"_n};
       update_p3sf5(schedule_version,block_time,soft_trigger_calc_reward,producer);
   }



} /// namespace eosiosystem
