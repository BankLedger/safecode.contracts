
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
}
