
//class [[eosio::contract("eosio.system")]] system_contract : public native {
   public:

      //######################################################
      ////////////////////////////////////////////////////////
      //[[eosio::action]] driven by safe-chain
      ////////////////////////////////////////////////////////
      //######################################################

      /**
       * 
       *
       * @details 
       *
       * @param rptxo - 
       * @param sfri - 
       *
       * @pre rptxo(unlocked) is never used to register as any producer
       * @pre sfri.dvdratio is in range [0, 100]
       * @pre sfri verify infohash/sc_sig/sc_pubkey successfully
       * 
       * @post do action sf5vote
       * @post do action sf5unregprod
       * @post do action regproducer
       * 
       */
      [[eosio::action]]
      void sf5regprod( const struct sf5key& sfkey, const struct txokey& rptxokey, const struct sfreginfo& ri );

      [[eosio::action]]
      void sf5unregprod( const struct sf5key& sfkey, const struct txokey& rptxokey );

      [[eosio::action]]
      void sf5updprodri( const struct sf5key& sfkey, const struct txokey& rptxokey, const struct sfupdinfo& updri );

      /**
       * 
       *
       * @details 
       *
       * @param rptxokey - 
       * @param vtxo - 
       *
       * @pre txo(locked or unlocked) is never used to vote to any producer.
       * 
       * @post 
       * 
       */   
      [[eosio::action]]
      void sf5vote( const struct sf5key& sfkey, const struct txokey& rptxokey, const struct txo& vtxo );

      [[eosio::action]]
      void sf5unvote( const struct sf5key& sfkey, const struct txokey& vtxokey );

      [[eosio::action]]
      void sf5bindaccnt( const struct sf5key& sfkey, const struct sfaddress& sfaddr, const name& account );

      [[eosio::action]]
      void sf5setnext( const struct sf5key& sfkey );

      [[eosio::action]]
      void sf5pubkhash( const public_key& sc_pubkey );


      //######################################################
      ////////////////////////////////////////////////////////
      //[[eosio::action]] called by safecode-side
      ////////////////////////////////////////////////////////
      //######################################################

      [[eosio::action]]
      void resetg4vote();

      [[eosio::action]]
      void regproducer2( const struct txokey& rptxokey, const name& account, const signature& newsig );

      [[eosio::action]]
      void sc5vote( const name& voter, const name& producer );

      [[eosio::action]]
      void claim4prod( const name& producer );

      [[eosio::action]]
      void claim4vote( const name& voter );

      //######################################################
      ////////////////////////////////////////////////////////
      //[[eosio::action]] misc
      ////////////////////////////////////////////////////////
      //######################################################
      [[eosio::action]]
      void checksign( const eosio::checksum256& digest, const eosio::signature& sig, const eosio::public_key& pubkey );

      [[eosio::action]]
      void sc3onblock(const uint32_t& schedule_version);

   private:
      void init_year_reward();

      template< typename TableIndex >
      auto findByTxo( const TableIndex& tbl_index, const struct txokey& txokey );

      template< typename TableIndex,typename Index>
      auto findByUniqueIdx( const TableIndex& tbl_index, const Index& index );

      bool dvalue_bigger_than_zero(const double& dvalue);

      bool negative_dvalue_equals_zero(const double& dvalue);

      void update_bp_votes(bool insert, const struct txokey& rptxokey,const double& votes,const struct txo& vtxo);

      void setnext(const struct sf5key& sfkey);

      void clear_f3_tables();

      void copy_data_from_s3_to_p3(const uint32_t& reward_producer_count,std::map<double,eosio::producer_key>& sftop21_producers_map,
                                   uint32_t& top40_producer_count);

      void set_sc_proposed_producers(const std::map<double,eosio::producer_key>& sftop21_producers_map,const uint32_t& top40_producer_count,
                                     bool& soft_trigger_calc_reward);

      void update_sc_elected_producers(const uint32_t& reward_producer_count,bool& soft_trigger_calc_reward);

      void update_s3sf5(const uint32_t& block_time,bool& soft_trigger_calc_reward);

      void update_p3sf5(const uint32_t& schedule_version,const uint32_t& block_time,const bool& soft_trigger_calc_reward,
                        const name& producer);

      void settlement_rewards(const uint32_t& schedule_version,const uint32_t& block_time,const bool& soft_trigger_calc_reward);

      void calculate_rewards(const uint32_t& block_time,totals_pay& total_pay,std::map<txokey,prods_pay>& prods_bpay_map,
                             std::map<name,voters_pay>& voters_pay_map);

      void record_block_rewards(const name& producer);

      void clear_p3_tables();

      void copy_data_from_f3_to_p3();

      void update_prods_rewards(std::map<txokey,prods_pay>& prods_bpay_map,const bool& reward_week_ret,const totals_pay& total_pay);

      void update_voters_rewards(const std::map<name,voters_pay>& voters_pay_map,const bool& reward_week_ret,const totals_pay& total_pay);

      void update_bpay_rewards(name owner,const double& bpay,const bool& reward_week_ret);

      void update_vpay_rewards(name owner,const double& vpay,const bool& reward_week_ret,const bool& is_voter,const sfaddress& sfaddr);

      void transfer_pay(const totals_pay& total_pay);

//};
