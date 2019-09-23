
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
      void sc3onblock();

   private:
      template< typename TableIndex >
      auto findByTxo( const TableIndex& tbl_index, const struct txokey& txokey );

      template< typename TableIndex,typename Index>
      auto findByIdx( const TableIndex& tbl_index, const Index& index );

      bool dvalue_bigger_than_zero(const double& dvalue);

      void update_bp_votes(bool insert, const struct txokey& rptxokey,const double& votes,const struct txo& vtxo);

      void setnext(const struct sf5key& sfkey);

      void update_sc_elected_producers(const uint32_t& bp_count_max);

      void updates3sf5();
//};
