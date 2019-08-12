
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
      [[eosio::action]]
      void vtxo2prod( const struct txo& txo, const name& producer );

   private:
      template< typename TableIndex >
      auto findByTxo( const TableIndex& tbl_index, const struct txo& txo );

//};