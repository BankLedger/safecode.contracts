
#include <eosio.system/printkit.hpp>

namespace eosio { namespace danX {

template<>
void PrintAux<uint16_t>::print() const
{
    eosio::print(m_x);
}

template<>
void PrintAux<uint64_t>::print() const
{
    eosio::print(m_x);
}

template<>
void PrintAux<uint8_t>::print() const
{
    eosio::print(m_x);
}

}} //namespace danX, eosio
