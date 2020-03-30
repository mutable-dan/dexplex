// MIT License
// Copyright (c) 2019 G. Dan
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once
#include "../include/common.h"
#include "../include/bg_cache.h"
#include "../include/loggingInterface.h"
#include <string>
#include <optional>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>

class dexcom_share final
{
   public:
      using returnType = std::optional<std::string>;
      using vector_BG  = std::vector<data::bg_data>;

   private:
      const std::string        m_strShareUrlbase       = "https://share2.dexcom.com/";
      const std::string        m_strShareLogin         = "ShareWebServices/Services/General/LoginPublisherAccountByName";
      const std::string        m_strShareGetBG         = "ShareWebServices/Services/Publisher/ReadPublisherLatestGlucoseValues";

      const int32_t            m_cnHttpOk              = 200;
      int32_t                  m_nReqTimeout_sec       = 120;
      int32_t                  m_nShareCheckInterval   = 5;   // every 5 min

      // see param defs for rest api calls: https://github.com/nightscout/share2nightscout-bridge
      // firstFetchCount - Changes maxCount during the very first update only
      int32_t                  m_nMinutes              = 1440;   // time window to search for data, default is one day
      int32_t                  m_nMaxCount             = 1;      // max record to get per call
      std::vector<std::string> m_errorList;

      std::string              m_strUserName;
      std::string              m_strPassword;
      std::string              m_strAccoundId;

      // login status
      bool                     m_bLoggedIn             = false;
      int32_t                  m_nLoginStatusCode      = 0;
      std::string              m_strSessionId;

      // get BC status
      uint64_t                 m_ulLastReading         = 0;
      vector_BG                m_vReadings;
      mutable std::mutex       m_muxBG;


      // start thread
      std::atomic_bool         m_bStop                 = false;
      std::atomic_bool         m_bIsDataAvail          = false;  // set to false when read by parent, set to true when new data is avail from dexcom
      std::thread              m_thd;


      bool login();
      bool dexcomShareData();
      void _start( std::shared_ptr<sync_tools::monitor> a_pSync, logging::log a_log );
      void error( const std::string &a_strError ) { m_errorList.push_back( a_strError ); }
      void error( const char* a_pszError  )       { m_errorList.push_back( a_pszError ); }

   public:
      dexcom_share() {}
      dexcom_share( dexcom_share& )  = delete;
      dexcom_share( dexcom_share&& ) = delete;

      void userName  ( const std::string& a_strUserName    ) { m_strUserName  = a_strUserName; }
      void password  ( const std::string& a_strPassword    ) { m_strPassword  = a_strPassword; }
      void accoundId ( const std::string& a_strAccountId   ) { m_strAccoundId = a_strAccountId; }
      void setTimeout( const int32_t      a_nTimoutSeconds ) { m_nReqTimeout_sec = a_nTimoutSeconds; }

      bool getBG_Reading( vector_BG& a_vBg );

      bool start( std::shared_ptr<sync_tools::monitor> a_pSync, logging::log &a_log );
      void stop()           { m_bStop = true; }
      void wait()           { if( m_thd.joinable() == true ) m_thd.join(); }
      bool isNewDataReady() { return m_bIsDataAvail; }
      
      bool isError() const{ return m_errorList.size() > 0; }
      const auto& errors() const { return m_errorList; }
      void clearErrors()   { m_errorList.clear(); }
};
