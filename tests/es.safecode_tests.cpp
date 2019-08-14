#include <boost/test/unit_test.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/chain/wast_to_wasm.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fc/log/logger.hpp>
#include <eosio/chain/exceptions.hpp>
#include <Runtime/Runtime.h>

#include "es.safecode_tester.hpp"
namespace utf = boost::unit_test;

struct _abi_hash {
   name owner;
   fc::sha256 hash;
};
FC_REFLECT( _abi_hash, (owner)(hash) );

struct connector {
   asset balance;
   double weight = .5;
};
FC_REFLECT( connector, (balance)(weight) );
using namespace eosio_system;

BOOST_AUTO_TEST_SUITE(es_safecode_tests, * utf::enabled())

bool within_one(int64_t a, int64_t b) { return std::abs(a - b) <= 1; }

BOOST_FIXTURE_TEST_CASE( vtxo2prod_function, es_safecode_tester ) try {

   //1. insert new row
   struct address addr = {.str_addr = "eeee"};
   struct txo vtxo = {
      .txid = fc::sha256::hash("d1"),
      .outidx = 5,
      .quantity = 0,
      .from = addr,
      .type = 1,
      .tp = fc::time_point::from_iso_string("2010-08-06T08:08:08")
   };

   BOOST_REQUIRE_EQUAL( success(), vtxo2prod(vtxo, "prod11111") );

   auto temp = get_vtxo4sc(0);
   BOOST_REQUIRE_EQUAL( 0, temp["v_id"] );

   vtxo.txid = fc::sha256::hash("d2");
   ++vtxo.outidx;
   std::cout << __FILE__ << __LINE__ << ":" << vtxo.txid.str() << std::endl;
   BOOST_REQUIRE_EQUAL( success(), vtxo2prod(vtxo, "prod11111") );
   temp = get_vtxo4sc(1);
   BOOST_REQUIRE_EQUAL( 1, temp["v_id"] );

   temp = get_vtxo4sc(vtxo.txid);
   //std::cout << temp.is_object() << std::endl;
   BOOST_REQUIRE_EQUAL( vtxo.txid, temp["v_txo"].as<struct txo>().txid );
   BOOST_REQUIRE_EQUAL( 6, temp["v_txo"].as<struct txo>().outidx );

   //auto temp2 = get_vtxo4sc_list(0);
   //std::cout << __FILE__ << " " << __LINE__ << ": " << temp2 << std::endl;

   ++vtxo.outidx;
   std::cout << __FILE__ << __LINE__ << ":" << vtxo.txid.str() << std::endl;
   BOOST_REQUIRE_EQUAL( success(), vtxo2prod(vtxo, "prod11111") );
   temp = get_vtxo4sc(1);
   BOOST_REQUIRE_EQUAL( 1, temp["v_id"] );

   // auto temp2 = get_vtxo4sc_list(fc::sha256());
   // std::cout << __FILE__ << " " << __LINE__ << ": " << temp2["v_txo"].as<struct txo>().txid.str() << std::endl;
   // std::cout << __FILE__ << " " << __LINE__ << ": " << typeid(temp2["v_txo"].as<struct txo>().outidx).name() << std::endl;
   // std::cout << __FILE__ << " " << __LINE__ << ": " << typeid(uint8_t).name() << std::endl;
   // std::cout << __FILE__ << " " << __LINE__ << ": " << temp2["v_txo"]["outidx"].as<uint16_t>() << std::endl;
   // std::cout << __FILE__ << " " << __LINE__ << ": " << temp2["v_txo"].as<struct txo>().from.str_addr << std::endl;


   //2. insert duplicated row


   //3. insert total 20w rows

} FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
