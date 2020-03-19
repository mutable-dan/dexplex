#pragma once

#include "../include/dex-rest.h"
#include "../include/common.h"

#include <mutlib/config.h>
#include <functional>
#include <atomic>
#include <memory>
#include <thread>

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
      std::shared_ptr<sync_tools::monitor> m_sp;
      std::atomic_bool      m_bReaderReady    = false;    // is reader ready to recieve bg
      std::atomic_bool      m_breaderStop     = false;    // stop the reader

      void reader( std::function< void( const std::string &) >                             a_log_bg,
                   std::function< void( const std::string &, const logging::logLevel_t ) > a_log_level );


    public:
      bool start( mutlib::config &a_cfg,
                  std::function< void( const std::string &) >                             &a_log_bg,
                  std::function< void( const std::string &, const logging::logLevel_t ) > &a_log_level );
      void stop();
      void wait();



};
