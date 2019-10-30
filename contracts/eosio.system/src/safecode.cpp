#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>
#include <eosio.system/printkit.hpp>

namespace eosiosystem {

   using eosio::check;

    global4vote system_contract::get_default_global4vote() {
        global4vote ret;
        ret.last_sch_ver = 0;
        ret.active_timestamp = 0;
        ret.last_calc_rewards_timestamp = 0;
        ret.last_set_proposed_producers_timestamp = 0;
        ret.last_unpaid_rewards = 0;
        ret.last_unpaid_block = 0;
        ret.last_claim_week = 0;
        ret.last_top40bp_votes_change = false;
        ret.sf_atom_id = 0;
        ret.sf_block_num = 0;
        ret.sf_tx_index = 0;
        return (ret);
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
        require_auth("safe.ssm"_n);
        DEBUG_PRINT_VAR(this->get_self());

        setnext(sfkey);
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

        eosio::print("sf5regprod:regist bp [",rptxokey.txid,",",rptxokey.outidx,"]\n");
   }

   void system_contract::sf5unregprod( const struct sf5key& sfkey, const struct txokey& rptxokey )
   {
       setnext(sfkey);
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

       eosio::print("sf5unregprod:unregist bp:[",rptxokey.txid,",",rptxokey.outidx,"],top40:",exists,"\n");
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
       setnext(sfkey);
       auto found_ret = findByTxo(_sf5vtxo.get_index<"by3txid"_n>(), vtxo.key);
       auto found = std::get<0>(found_ret);
       check( found == false , "error,sf5vote:vtxo has exists at table sf5vtxo" );
       check( vtxo.type >=txo_type::type_mn && vtxo.type <= txo_type::type_other,"error,sf5vote:vtxo.type must be in range [0,1]" );
       check( vtxo.quantity.symbol == core_symbol(), "error,sf5vote:must use core symbol" );
       check( vtxo.quantity.is_valid(), "error,sf5vote:invalid quantity" );

       //1.insert safe vote info
       double vtotal = 0;
       if(vtxo.type == txo_type::type_mn){
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
   }

   void system_contract::sf5unvote( const struct sf5key& sfkey, const struct txokey& vtxokey )
   {
       setnext(sfkey);
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
   }

   //XJTODO test pre bind,test after bind
   void system_contract::sf5bindaccnt( const struct sf5key& sfkey, const struct sfaddress& sfaddr, const name& account )
   {
       require_auth(account);
       setnext(sfkey);
       auto found_ret = findByUniqueIdx(_sfaddr2accnt.get_index<"by3sfaddr"_n>(),eosio::sha256( sfaddr.str.c_str(), sfaddr.str.length() ));
       auto found = std::get<0>(found_ret);
       check (found==false,("error, sf5bindaccnt:repeat bind,sfaddr " + sfaddr.str +" has exists at table sfaddr2accnt").data());

       _sfaddr2accnt.emplace(get_self(), [&]( auto& row ) {
           row.prmrid = _sfaddr2accnt.available_primary_key();
           row.sfaddr = sfaddr;
           row.account = account;
       });

       //XJTODO,update rewards4v owner

       eosio::print("sf5bindaccnt:bind safe addr ",sfaddr.str," to account ",account,"\n");
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

       int64_t curr_staked = 600'00000000;//XJTODO

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

   //XJTODO
   void system_contract::claim4prod( const name& producer )
   {
/*
        require_auth(producer);
        auto found_ret = findByUniqueIdx(_sf5producers.get_index<"by3owner"_n>(),producer.value);
        auto found = std::get<0>(found_ret);
        check( found, ( "producer " + producer.to_string() + " is not registered" ).data() );

//        //test code
//        _rewards4bp.emplace(get_self(), [&]( auto& row ) {
//            row.owner = producer;
//            row.period = 300;
//            row.unclaimed = 500;
//        });

        auto itr = _rewards4bp.find(producer.value);
        auto unclaimed = 0;
        auto period = 0;
        if(itr != _rewards4bp.end())
        {
            eosio::token::transfer_action transfer_act{ token_account, { {bpay_account, active_permission} } };
            _rewards4bp.modify(itr, get_self(), [&]( auto& row ) {
                transfer_act.send( bpay_account, producer, asset(row.unclaimed, core_symbol()), "fund per-block bucket" );
                unclaimed = row.unclaimed;
                period = row.period;
                row.unclaimed = 0;
            });
        }
        eosio::print("claim4prod:producer claim ",unclaimed,",the period is ",period,"\n");
*/
   }

   void system_contract::claim4vote( const name& voter )
   {

   }

   void system_contract::checksign( const eosio::checksum256& digest, const eosio::signature& sig, const eosio::public_key& pubkey )
   {
      assert_recover_key( digest, sig, pubkey );
   }


   //XJTODO,just for test,debug
   void system_contract::sc3onblock()
   {
       updates3sf5();
   }

   void system_contract::update_sc_elected_producers(const uint32_t& reward_producer_count)
   {
       //1.clear f3sf5 tables
       for( auto itr = _f3sf5prods.begin(); itr != _f3sf5prods.end(); ){
           itr = _f3sf5prods.erase(itr);
       }
       for( auto itr = _f3sf5vtxo.begin(); itr != _f3sf5vtxo.end(); ){
           itr = _f3sf5vtxo.erase(itr);
       }
       for( auto itr = _f3voters.begin(); itr != _f3voters.end(); ){
           itr = _f3voters.erase(itr);
       }

       //2.sort by sf_vtotal,copy top40 from sf5producers to f3sf5prods
       std::set<txokey> producers_key_set;
       std::set<name> producers_name_set;
       std::map<double,eosio::producer_key> sftop21_producers_map;
       const auto& producers_idx = _sf5producers.get_index<"by3sf5vtotal"_n>();
       uint32_t top40_producer_count=0,top_21_producer_count=0,block_producer_count=21;
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

       //3.copy votes from sf5vtxo to f3sf5vtxo
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

       //4.copy voters from sc5voters to f3voters
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

       //5.set proposed producers
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
           auto ret = set_proposed_producers( producers );
           eosio::print("set proposed sc producers ret:",*ret,",");
       }else
       {
           eosio::print("no sc producer.may be first loop.");
       }

       _gstate4vote.last_top40bp_votes_change = false;
   }

   void system_contract::updates3sf5()
   {
       uint32_t curr_time = eosio::current_time_point().sec_since_epoch();
       if(curr_time - _gstate4vote.last_set_proposed_producers_timestamp < 900){
           return;
       }

       bool need_new_sch_prods = false;
       uint32_t reward_producer_count=40;
       auto last_top40bp_votes_change_tmp = _gstate4vote.last_top40bp_votes_change;
       eosio::print("updates3sf5:");
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
       }

       if(need_new_sch_prods){
           update_sc_elected_producers(reward_producer_count);
       }

       _gstate4vote.last_set_proposed_producers_timestamp = curr_time;
       eosio::print("last_set_proposed_producers_timestamp:",_gstate4vote.last_set_proposed_producers_timestamp,",last_top40bp_votes_change_tmp:",
                    last_top40bp_votes_change_tmp,",need_new_sch_prods:",need_new_sch_prods);
   }

} /// namespace eosiosystem
