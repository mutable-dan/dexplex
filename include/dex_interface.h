#pragma once
#include "../include/common.h"
#include <memory>

class dex_interface
{
   public:
      using vector_BG  = std::vector<data::bg_data>;
      virtual ~dex_interface() = default;
      virtual bool start( std::shared_ptr<sync_tools::monitor> a_pSync, logging::log& a_log, int64_t a_lastReadDate ) = 0;
      virtual void stop() = 0;
      virtual void wait() = 0;
      virtual bool isNewDataReady() = 0;
      //virtual bool getBG_Reading( vector_BG& a_vBg ) = 0;
};
