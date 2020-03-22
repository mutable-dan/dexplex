#include "../include/bg_cache.h"
#include <tuple>


tools::bg_cache::bg_cache( int32_t a_nCapacity ): m_nCapacity( a_nCapacity )
{
    m_bg_ring.resize( m_nCapacity );
}

void tools::bg_cache::push( const int64_t a_nDT, const int64_t a_nST, const int64_t a_nWT, const int64_t a_nValue, const int64_t a_nTrend )
{
    data item;
    item.DT    = a_nDT;
    item.ST    = a_nST;
    item.WT    = a_nWT;
    item.value = a_nValue;
    item.trend = a_nTrend;
    m_bg_ring.push_front( item );
}

void tools::bg_cache::push( const data &a_data )
{
    m_bg_ring.push_front( a_data );
}

std::tuple<bool, tools::data> tools::bg_cache::front()
{
    if( m_bg_ring.empty() )
    {
        return std::make_tuple( false, data() );
    }
    return std::make_tuple( true, m_bg_ring.front() );
//    return { true, m_bg_ring.front() };
}
