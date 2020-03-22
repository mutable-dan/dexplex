#pragma once

#include <vector>
#include <cinttypes>
#include <boost/circular_buffer.hpp>


namespace tools
{
    // keyed on DT (device date) which is the date from the source whoch collected the value
    // store ST,WT, value, trend
    class bg_data
    {
        public:
            int64_t DT    = 0;
            int64_t ST    = 0;
            int64_t WT    = 0;
            int16_t value = 0;
            int16_t trend = 0;
        public:
            bool operator<  ( const bg_data& a_rhs )   { return DT <  a_rhs.DT; }
            bool operator<= ( const bg_data& a_rhs )   { return DT <= a_rhs.DT; }
            bool operator>  ( const bg_data& a_rhs )   { return DT >  a_rhs.DT; }
            bool operator>= ( const bg_data& a_rhs )   { return DT >= a_rhs.DT; }
            bool operator== ( const bg_data& a_rhs )   { return DT == a_rhs.DT; }
            bool operator!= ( const bg_data& a_rhs )   { return DT != a_rhs.DT; }

    };

    using bg_buffer_t = boost::circular_buffer<bg_data>;

    class bg_cache
    {
        private:
            bg_buffer_t m_bg_ring;
            int32_t     m_nCapacity;    // capacity is defaulted to 1 day of data where data arrives every 5 minutes
        public:
            bg_cache( int32_t a_nCapacity = 288 );
            size_t size() const noexcept { return m_bg_ring.size(); }

            void   push( const int64_t a_nDT, const int64_t a_nST, const int64_t a_nWT, const int64_t a_nValue, const int64_t a_nTrend ) noexcept;
            void   push( const bg_data& a_data ) noexcept;
            auto   front()                  noexcept -> std::tuple<bool, bg_data>;
            auto   front( size_t a_nCount ) noexcept -> std::tuple<bool, std::vector< tools::bg_data> >;

            bg_buffer_t* data() { return &m_bg_ring; }
    };

}
