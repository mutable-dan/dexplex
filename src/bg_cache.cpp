#include "../include/bg_cache.h"
#include <fstream>
#include <tuple>
#include <iterator>

using namespace std;

///
/// \brief tools::bg_cache::bg_cache
/// \param a_nCapacity
///
data::bg_cache::bg_cache( int32_t a_nCapacity ): m_nCapacity( a_nCapacity )
{
    m_bg_ring.set_capacity( m_nCapacity );
}

///
/// \brief tools::bg_cache::push
/// \note push one item on, if cap is met, push old one off
/// \param a_nDT
/// \param a_nST
/// \param a_nWT
/// \param a_nValue
/// \param a_nTrend
///
void data::bg_cache::push( const int64_t a_nDT, const int64_t a_nST, const int64_t a_nWT, const int64_t a_nValue, const int64_t a_nTrend ) noexcept
{
    bg_data item;
    item.DT    = a_nDT;
    item.ST    = a_nST;
    item.WT    = a_nWT;
    item.value = a_nValue;
    item.trend = a_nTrend;
    m_bg_ring.push_front( item );
}

///
/// \brief tools::bg_cache::push
/// \note push one item on, if cap is met, push old one off
/// \param a_data
///
void data::bg_cache::push( const bg_data &a_data ) noexcept
{
    m_bg_ring.push_front( a_data );
}

///
/// \brief tools::bg_cache::front
/// \note return most recent item
/// \return
///
auto data::bg_cache::front() noexcept -> std::tuple<bool, data::bg_data>
{
    if( m_bg_ring.empty() )
    {
        return { false, bg_data() };
    }
    return { true, m_bg_ring.front() };
}

///
/// \brief tools::bg_cache::front
/// \note return n most recent items
/// \param a_nCount
/// \return
///
auto data::bg_cache::front( const size_t a_nCount ) noexcept -> std::tuple<bool, std::vector< data::bg_data> >
{
    vector< data::bg_data > vData;
    if( m_bg_ring.empty() || (a_nCount == 0) )
    {
        return { false, vData };
    }
    if( a_nCount <= m_bg_ring.size() )
    {
        size_t cc =  m_bg_ring.size() - a_nCount;
        auto rst  = m_bg_ring.begin();
        auto rend = m_bg_ring.end() - cc;
        for( auto it=rst; it<rend; ++it )
        {
            vData.push_back( *it );
        }
        return std::make_tuple( true, vData );
    }

    return { false, vData };
}

///
/// \brief data::bg_cache::cashLoad
/// \param a_strPath
/// \note load blood glucose log and put bg data in cache
bool data::bg_cache::cashLoad( const std::string a_strPath, const cache_param_t &a_processLLogEntry )
{
    bool bIsError = false;
    string strLogfile = logging::find_log_newest( a_strPath );
    ifstream inf( strLogfile );
    if( inf.is_open() )
    {
        string strLine;
        while( std::getline( inf, strLine ).eof() == false )
        {
            data::bg_data bg;
            if( true == (bool)a_processLLogEntry )
            {
                if( false == a_processLLogEntry( strLine, bg ) )
                {
                    bIsError = true;
                }
                m_bg_ring.push_front( bg );
            }
        }
    }
    inf.close();
    return bIsError? false: true;
}
