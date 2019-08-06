
//class [[eosio::contract("eosio.system")]] system_contract : public native {

   public:
      [[eosio::action]]
      void vtxo2prod( const struct txo& txo, const name& producer );

   private:
      template< typename TableIndex >
      auto findByTxo( const TableIndex& tbl_index, const struct txo& txo );

//};
