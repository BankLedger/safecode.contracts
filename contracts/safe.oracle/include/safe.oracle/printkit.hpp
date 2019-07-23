#pragma once

#ifndef __FILE_PRINTKIT_HPP__
#define __FILE_PRINTKIT_HPP__

#include <eosio/eosio.hpp>

/*
//奇葩的printf
    //uint16_t size = 0;
    //DEBUG_PRINTF("tbl_group size = %u", size);
    //输出符合预期
    //printf输出为0,不是真实值;printui输出真实的值
    //DEBUG_PRINTF("grp_amp_min.amount = %u", grp_amp_min.amount);
    //print_f("grp_amp_min.amount = "); printui(grp_amp_min.amount); prints("\n");
//结论：避免使用printf,使用api文档提及的printXX系列函数
#define DEBUG_PRINTF(fmt, ...) printf("%s %s %d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
//解决办法：模板+实例化+宏；详见代码
*/

namespace eosio { namespace danX {

template<typename T>
class PrintAux
{
    private:
        const T   &m_x;

    public:
        PrintAux(const T &x):m_x(x){};
        void print() const
        {
            eosio::print(m_x);
        }
};

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

////////////////////////////////////////////////////////////

#define DEBUG_PRINT_POS() eosio::print_f("% % %\n", __FILE__, __FUNCTION__, __LINE__)
#define DEBUG_PRINT_VAR(x) eosio::print_f("% % %: "#x": %\n", __FILE__, __FUNCTION__, __LINE__, eosio::danX::PrintAux(x))

#endif  //__FILE_PRINTKIT_HPP__
