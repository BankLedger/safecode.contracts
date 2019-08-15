
#include "es.safecode_tester.hpp"

namespace eosio_system {

   void es_safecode_tester::basic_setup() {
      produce_blocks( 2 );

      create_accounts({ N(eosio.token), N(eosio.ram), N(eosio.ramfee), N(eosio.stake),
               N(eosio.bpay), N(eosio.vpay), N(eosio.saving), N(eosio.names), N(eosio.rex),
               N(safe), N(safe.oracle) });


      produce_blocks( 100 );
      set_code( N(eosio.token), contracts::token_wasm());
      set_abi( N(eosio.token), contracts::token_abi().data() );
      {
         const auto& accnt = control->db().get<account_object,by_name>( N(eosio.token) );
         abi_def abi;
         BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
         token_abi_ser.set_abi(abi, abi_serializer_max_time);
      }
   }

   void es_safecode_tester::create_core_token( symbol core_symbol ) {
      FC_ASSERT( core_symbol.decimals() == 8, "create_core_token assumes core token has 8 digits of precision" );
      create_currency( N(eosio.token), config::system_account_name, asset(4500000'00000000, core_symbol) );
      //issue( asset(0, core_symbol) );
      //BOOST_REQUIRE_EQUAL( asset(0, core_symbol), get_balance( "eosio", core_symbol ) );
      BOOST_REQUIRE_EQUAL( asset(4500000'00000000, core_symbol), get_token_max_supply() );
   }

   void es_safecode_tester::deploy_contract( bool call_init ) {
      set_code( config::system_account_name, contracts::system_wasm() );
      set_abi( config::system_account_name, contracts::system_abi().data() );
      if( call_init ) {
         base_tester::push_action(config::system_account_name, N(init),
                                               config::system_account_name,  mutable_variant_object()
                                               ("version", 0)
                                               ("core", CORE_SYM_STR)
         );
      }

      {
         const auto& accnt = control->db().get<account_object,by_name>( config::system_account_name );
         abi_def abi;
         BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
         abi_ser.set_abi(abi, abi_serializer_max_time);
      }
   }

   void es_safecode_tester::remaining_setup() {
      produce_blocks();

      // // Assumes previous setup steps were done with core token symbol set to CORE_SYM
      // create_account_with_resources( N(alice1111111), config::system_account_name, core_sym::from_string("1.0000"), false );
      // create_account_with_resources( N(bob111111111), config::system_account_name, core_sym::from_string("0.4500"), false );
      // create_account_with_resources( N(carol1111111), config::system_account_name, core_sym::from_string("1.0000"), false );

      // BOOST_REQUIRE_EQUAL( core_sym::from_string("1000000000.0000"), get_balance("eosio")  + get_balance("eosio.ramfee") + get_balance("eosio.stake") + get_balance("eosio.ram") );
   }

   es_safecode_tester::es_safecode_tester( setup_level l ) {
      if( l == setup_level::none ) return;

      basic_setup();
      if( l == setup_level::minimal ) return;

      create_core_token();
      if( l == setup_level::core_token ) return;

      deploy_contract();
      if( l == setup_level::deploy_contract ) return;

      remaining_setup();
   }

   template<typename Lambda>
   es_safecode_tester::es_safecode_tester(Lambda setup) {
      setup(*this);

      basic_setup();
      create_core_token();
      deploy_contract();
      remaining_setup();
   }

   fc::sha256 es_safecode_tester::inverted(const checksum256_type& p) const
   {
      fc::sha256 r;
      r._hash[0] = p._hash[1];
      r._hash[1] = p._hash[0];

      r._hash[2] = p._hash[3];
      r._hash[3] = p._hash[2];

      for(int i = 0; i < 4; ++i){
         uint64_t v = r._hash[i];
         uint64_t b0 = (v & 0x00000000000000ffU);
         uint64_t b1 = (v & 0x000000000000ff00U);
         uint64_t b2 = (v & 0x0000000000ff0000U);
         uint64_t b3 = (v & 0x00000000ff000000U);
         uint64_t b4 = (v & 0x000000ff00000000U);
         uint64_t b5 = (v & 0x0000ff0000000000U);
         uint64_t b6 = (v & 0x00ff000000000000U);
         uint64_t b7 = (v & 0xff00000000000000U);

         uint64_t _b0 = b0 << 56;
         uint64_t _b1 = b1 << 40;
         uint64_t _b2 = b2 << 24;
         uint64_t _b3 = b3 << 8;
         uint64_t _b4 = b4 >> 8;
         uint64_t _b5 = b5 >> 24;
         uint64_t _b6 = b6 >> 40;
         uint64_t _b7 = b7 >> 56;

         r._hash[i] = _b0 | _b1 | _b2 | _b3 | _b4 | _b5 | _b6 | _b7;
      }
      return r;
   }

   es_safecode_tester::action_result es_safecode_tester::sf5regprod( const struct txo& rptxo, const struct sfreginfo& sfri ) {
      return push_action( config::system_account_name, N(sf5regprod), mutable_variant_object()
                                ("rptxo",    rptxo)
                                ("sfri",     sfri )
                                );
   }
}
