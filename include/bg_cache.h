#pragma once
#include "../include/common.h"

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <cinttypes>
#include <boost/circular_buffer.hpp>

namespace data
{

    // keyed on DT (device date) which is the date from the source whoch collected the value
    // store ST,WT, value, trend
    // https://developer.dexcom.com/get-events-v1
    //   systemTime    Time according to the system clock for the event; nominally UTC
    //   displayTime   Time displayed on the receiving device for the event
    // https://developer.dexcom.com/endpoint-overview
    //   ISO 8601 standard
    //   systemTime and displayTime
    //   systemTime is the UTC time according to the device
    //   displayTime is the time being shown on the device to the user
    //     Depending on the device, this time may be user-configurable, and can therefore change its offset
    //     relative to systemTime.
    //   Note that systemTime is not "true" UTC time because of drift and/or user manipulation of the devices' clock
    // example
    // DT = 1589657198000  local time  7:26
    // ST = 1589671598000  UTC
    // WT = 1589671598000

    class bg_data
    {
        public:
            int64_t DT    = 0; // disp time
            int64_t ST    = 0; // sys time
            int64_t WT    = 0; // ??
            int64_t DTz   = 0; // timezone offset

            std::string strDT;
            std::string strST;
            std::string strWT;
            int16_t     value = 0;
            int16_t     trend = 0;
        public:
            bool operator<  ( const bg_data& a_rhs )   { return DT <  a_rhs.DT; }
            bool operator<= ( const bg_data& a_rhs )   { return DT <= a_rhs.DT; }
            bool operator>  ( const bg_data& a_rhs )   { return DT >  a_rhs.DT; }
            bool operator>= ( const bg_data& a_rhs )   { return DT >= a_rhs.DT; }
            bool operator== ( const bg_data& a_rhs )   { if( (DT == a_rhs.DT) && (ST == a_rhs.ST) && (WT == a_rhs.WT) && (value == a_rhs.value) && (trend == a_rhs.trend) ) return true; else return false; }
            bool operator!= ( const bg_data& a_rhs )   { return !(*this == a_rhs); }
    };


    using bg_buffer_t   = boost::circular_buffer<bg_data>;
    using cache_param_t = std::function< bool( const std::string &, data::bg_data &) >;

    class bg_cache
    {
        private:
            bg_buffer_t m_bg_ring;
            int32_t     m_nCapacity;    // capacity is defaulted to 1 day of data where data arrives every 5 minutes
            int64_t     m_lastReadDate = 0;

            mutable std::mutex  m_mux;
        public:
            bg_cache( int32_t a_nCapacity = 288 );
            size_t size() const noexcept { return m_bg_ring.size(); }
            bool   verify_request( const size_t a_nCount ) const { return (a_nCount <= m_bg_ring.size()) && (a_nCount > 0); }

            void   push( const int64_t a_nDT, const int64_t a_nST, const int64_t a_nWT, const int64_t a_nValue, const int64_t a_nTrend ) noexcept;
            void   push( const bg_data& a_data )  noexcept;
            auto   front()                        noexcept -> std::tuple<bool, bg_data>;
            auto   front( const size_t a_nCount ) noexcept -> std::tuple<bool, std::vector< data::bg_data> >;

            const bg_buffer_t* data() { return &m_bg_ring; }

            bool cashLoad( const std::string a_strPath, const cache_param_t &a_processLLogEntry );
            int64_t lastReadDate() { return m_lastReadDate; }
    };

}
