#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>

#include <string>

#include "printkit.hpp"
#include "checkkit.hpp"

namespace eosiosystem {
   class system_contract;
}

namespace eosio {

   using std::string;

   /**
    * @defgroup safeoracle safe.oracle
    * @ingroup eosiocontracts
    *
    * safe.oracle contract
    *
    * @details safe.oracle contract defines the structures and actions that allow users to
    * cc/bcc tokens on eosio based blockchains. Please set safe.oracle@eosio.code permissiom
    * to ABC@crosschain permission, ABC is the contract which will be called by safe.oracle.
    * @{
    */
   class [[eosio::contract("safe.oracle")]] safeoracle : public contract {

      public:     //////////////////////////////////////////////////
      
         using contract::contract;
         typedef eosio::name           name;
         typedef eosio::asset          asset;
         typedef eosio::datastream<const char*>   datastream__const_char;
         typedef std::string           string;

         struct chain_pos {
            uint32_t       block_num;
            uint16_t       tx_index;        //based 0

            void print() const
            {
               eosio::print("block_num = "); eosio::print(block_num); eosio::print("\n");
               eosio::print("tx_index = "); eosio::print(tx_index); eosio::print("\n");
            }

            bool operator < (const struct chain_pos& another) const
            {
               return (
                  this->block_num < another.block_num ||
                  (
                     this->block_num == another.block_num &&
                     this->tx_index < another.tx_index
                  )
               );
            }

            bool operator == (const struct chain_pos& another) const
            {
               return (
                  this->block_num == another.block_num &&
                  this->tx_index == another.tx_index
               );
            }

            bool operator > (const struct chain_pos& another) const
            {
               return (
                  this->block_num > another.block_num ||
                  (
                     this->block_num == another.block_num &&
                     this->tx_index > another.tx_index
                  )
               );
            }
         };

         struct cctx_info {
            uint8_t           type;    //0: common transfer asset; 1: sync vote result from safe-chain
            name              account; //target account who is in safecode chain
            checksum256       txid;    //txid at safe chain
            uint8_t           outidx;  //out-index of utxo tx's vout array
            asset             quantity;//asset(amount and token) at txid to account
            string            detail;  //detail[or memo](json string) at txid-outidx

            void print() const
            {
               eosio::print("type = "); eosio::print(type); eosio::print("\n");
               eosio::print("account = "); eosio::print(account); eosio::print("\n");
               eosio::print("txkey = "); eosio::print(txid); eosio::print("-"); eosio::print(outidx); eosio::print("\n");
               eosio::print("quantity = "); eosio::print(quantity); eosio::print("\n");
               eosio::print("detail = "); eosio::print(detail); eosio::print("\n");
            }
         };

         struct cctx_key {
            checksum256       txid;    //txid at safe chain
            uint8_t           outidx;  //out-index of utxo tx's vout array

            void print() const
            {
               eosio::print("txkey = "); eosio::print(txid); eosio::print("-"); eosio::print(outidx); eosio::print("\n");
            }
         };

      public:     //////////////////////////////////////////////////

         safeoracle( name receiver, name code,  datastream__const_char ds );

         /**
          * Set Chain Position action.
          *
          * @details To set chain position with `pos`.
          * @param pos - descripted by block_number and tx_index
          *
          * Init chain position once safely, or to modify chain position forcely and unsafely.
          */
         [[eosio::action]]
         void setchainpos( const struct chain_pos& pos );

         /**
          * Push CCTXes action.
          *
          * @details Record all `cctxes` between `curpos` and `nextpos` batchly.
          * @param curpos - the begin(include) chain pos of current batch.
          * @param nextpos - the end(exclude) chain pos of current batch.
          * @param cctxes - all cctx info, it may be empty.
          * 
          * @pre `curpos` of current batch must be equal to `nextpos` of last batch.
          *
          * Record all `cctxes` between `curpos` and `nextpos` batchly.
          */
         [[eosio::action]]
         void pushcctxes( const struct chain_pos& curpos, const struct chain_pos& nextpos, 
                          const std::vector< struct cctx_info >& cctxes );

         /**
          * Draw assets action.
          *
          * @details Allows caller account to draw all cross-chain assets batchly.
          * @param txkeys - some txkey which descript cctx.
          * 
          * @pre any txkeys is existed and do not be drawed.
          * @pre any account can only draw assets which is recored to himself.
          *
          * Allows caller account to draw all cross-chain assets batchly.
          */
         [[eosio::action]]
         void drawassets( const std::vector< cctx_key >& txkeys);

         using setchainpos_action = eosio::action_wrapper<"setchainpos"_n, &safeoracle::setchainpos>;
         using pushcctxes_action = eosio::action_wrapper<"pushcctxes"_n, &safeoracle::pushcctxes>;
         using drawassets_action = eosio::action_wrapper<"drawassets"_n, &safeoracle::drawassets>;

      private:    //////////////////////////////////////////////////

         struct [[eosio::table]] cctx {
            uint64_t          id;         //auto increament
            name              account;    //target account who is in safecode chain
            checksum256       txid;       //txid at safe chain
            uint8_t           outidx;     //out-index of utxo tx's vout array
            asset             quantity;   //asset(amount and token) at txid to account
            uint8_t           status;     //0: new for being drawed; 1: has been drawed

            uint64_t primary_key() const
            {
               return (id);
            }

            checksum256 get_txid() const
            {
               return (txid);
            }
         };

         typedef eosio::multi_index<"cctx"_n, cctx, 
            indexed_by<"txid"_n, const_mem_fun<cctx, checksum256, &cctx::get_txid>>
         > type_table__cctx;

         //////////////////////////////

         struct [[eosio::table]] globalkv {
            uint32_t       block_num;
            uint16_t       tx_index;        //based 0

            void print() const
            {
               eosio::print("block_num = "); eosio::print(block_num); eosio::print("\n");
               eosio::print("tx_index = "); eosio::print(tx_index); eosio::print("\n");
            }

            bool operator == (const struct chain_pos& pos) const
            {
               return (
                  this->block_num == pos.block_num &&
                  this->tx_index == pos.tx_index
               );
            }
         };

         typedef eosio::singleton<"globalkv"_n, globalkv>     type_table__globalkv;

         //////////////////////////////

         void init_globalkv( type_table__globalkv &tbl_globalkv );
         void push_each_cctx( type_table__cctx& tbl_cctx, const cctx_info& txinfo );
         void draw_each_asset( type_table__cctx& tbl_cctx, const cctx_key& txkey );
         template< typename TableIndex >
         auto findByTxkey( const TableIndex& tbl_index, const struct cctx_key& txkey );
         template< typename TableIndex >
         auto findByTxkey( const TableIndex& tbl_index, const checksum256& txid, uint8_t outidx );

         static uint32_t   dft__last_safed_block_num;
         static string checksum256_to_string( const checksum256& m );
         static char hex_to_char( uint8_t hex );
   };
   /** @}*/ // end of @defgroup safeoracle safe.oracle
} /// namespace eosio
