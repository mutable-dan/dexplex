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
#include "include/dex_interface.h"
#include "include/common.h"
#include "include/bg_cache.h"
#include <filesystem>
#include <string>
#include <vector>
#include <atomic>
#include <thread>

class dexcom_file final : public virtual dex_interface
{
   public:
      using vector_BG  = std::vector<data::bg_data>;

   private:
      int64_t                 m_lastReadDate    = 0;
      vector_BG               m_bgData;
      std::filesystem::path   m_path;
      bool                    m_bIsValidPath    = false;
      mutable std::mutex      m_muxBG;

      std::atomic_bool  m_bStop              = false;
      std::atomic_bool  m_bIsDataAvail       = false;
      std::thread       m_thdDexcomFile;

      auto dexcomFileData() -> std::tuple<bool, uint64_t, std::string>;
      void _start( std::shared_ptr<sync_tools::monitor> a_pSync, logging::log a_log );


   public:
      dexcom_file() {}
      dexcom_file( dexcom_file& )  = delete;
      dexcom_file( dexcom_file&& ) = delete;
   

      bool getBG_Reading( vector_BG& a_vBg );
      bool setFile( const std::string& a_strPath );

      bool start( std::shared_ptr<sync_tools::monitor> a_pSync, logging::log& a_log, int64_t a_lastReadDate );
      void stop()           { m_bStop = true; }
      void wait()           { if( m_thdDexcomFile.joinable() == true ) m_thdDexcomFile.join(); }
      bool isNewDataReady() { return m_bIsDataAvail; }

};