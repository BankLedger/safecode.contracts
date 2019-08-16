
//class [[eosio::contract("eosio.system")]] system_contract : public native {

   public:

      /**
       * 
       *
       * @details 
       *
       * @param txo - 
       * @param producer - 
       *
       * @pre txo(locked or unlocked) is never used to vote to any producer.
       * 
       * @post 
       * 
       */   
      // [[eosio::action]]
      // void sf5vote( const struct txo& txo, const name& producer );

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
       * @post 
       * 
       */
      [[eosio::action]]
      void sf5regprod( const struct txo& rptxo, const struct sfreginfo& sfri );

      [[eosio::action]]
      void sf5unregprod( const struct txokey& rptxokey );



      [[eosio::action]]
      void checksign( const eosio::checksum256& digest, const eosio::signature& sig, const eosio::public_key& pubkey );

   private:
      template< typename TableIndex >
      auto findByTxo( const TableIndex& tbl_index, const struct txo& txo );

//};
