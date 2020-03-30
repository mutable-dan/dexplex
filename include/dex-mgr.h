#pragma once

#include "../include/loggingInterface.h"
#include "../include/dex-rest.h"
#include "../include/bg_cache.h"
#include "../include/common.h"

#include <mutlib/config.h>
#include <functional>
#include <atomic>
#include <memory>
#include <thread>

#include <sstream>

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
      logging::log          m_log;
      data::bg_cache        m_cache;

      std::shared_ptr<sync_tools::monitor> m_sp;          // monitor class used to sync reader with new values from dexcom_share

      void reader( std::function< void( const std::string &) > a_log_bg );

    public:
      bool start( mutlib::config                                                          &a_cfg,
                  logging::log                                                            &a_log,
                  std::function< void( const std::string &) >                             &a_logbg );
      void stop();
      void wait();
};
