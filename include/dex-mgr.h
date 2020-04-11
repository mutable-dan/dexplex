#pragma once

#include "../include/loggingInterface.h"
#include "../include/dex-rest.h"
#include "../include/bg_cache.h"
#include "../include/common.h"

#include <mutlib/config.h>
#include <mutlib/util.h>
#include <functional>
#include <atomic>
#include <memory>
#include <thread>
#include <sstream>
#include <string.h>

//
// start and manage rest client
// start and manage writers
//      bg log writer
//      bg database writer
// start and manage rest server - if rest server will be embedded
class dexshareManager
{
   private:
      dexcom_share          m_ds;
      std::thread           m_thdReader;
      std::atomic_bool      m_bReaderReady    = false;    // is reader ready to recieve bg
      std::atomic_bool      m_bReaderStop     = false;    // stop the reader
      logging::log          m_appLogger;
      data::bg_cache        m_cache;

      std::shared_ptr<sync_tools::monitor> m_spMonitor;          // monitor class used to sync reader with new values from dexcom_share

      void reader( std::function< void( const std::string &) > a_log_bg );

    public:
      bool start( mutlib::config                                                          &a_cfg,
                  logging::log                                                            &a_log,
                  std::function< void( const std::string &) >                             &a_logbg );
      void stop();
      void wait();
};

// parse  log file for cached data on startup
const auto processCachline = [] ( const std::string &a_strLine, data::bg_data &a_bgData ) -> bool
{
    // sample string
    // [2020-04-10 20:11:22.209] [bg] [info] systime:2020-04-11 00:09:12+0000,dt:1586549352000,st:1586563752000,wt:1586563752000,bg:118,trend:4
    // dt:1586452448000,st:1586466848000,wt:1586466848000,bg:99,trend:4

    try
    {
    const uint32_t cnStart = 71;
    std::string strSubLine = a_strLine.substr( cnStart );
    mutlib::splitString ss1;
    ss1.split( strSubLine, "," );
    std::string strDt = ss1[0];
    std::string strSt = ss1[1];
    std::string strWt = ss1[2];
    std::string strBg = ss1[3];
    std::string strTr = ss1[4];
    mutlib::splitString ss2;
    ss2.split( strDt, ":" );
    a_bgData.DT = std::stoll( ss2[1] );
    ss2.clear();

    ss2.split( strSt, ":" );
    a_bgData.ST = std::stoll( ss2[1] );
    ss2.clear();

    ss2.split( strWt, ":" );
    a_bgData.WT = std::stoll( ss2[1] );
    ss2.clear();

    ss2.split( strBg, ":" );
    a_bgData.value = std::stoll( ss2[1] );
    ss2.clear();

    ss2.split( strTr, ":" );
    a_bgData.trend = std::stoll( ss2[1] );
    ss2.clear();
    } catch( ... )
    {
        memset( (void*)&a_bgData, 0, sizeof(data::bg_data) );
        return false;
    }
    return true;
};
