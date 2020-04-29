#pragma once

#include "../include/common.h"
#include "../include/bg_cache.h"

#include <string>
#include <set>

#undef DEBUG
#include <restbed>


namespace restServer
{
    namespace rest = restbed;

    class restHttpServer
    {
        private:
            using resource_t = std::set< std::string >;
            rest::Service   m_service;
            data::bg_cache *m_pCache    = nullptr;

            const std::string m_strDexShareLogin   =  "/ShareWebServices/Services/General/LoginPublisherAccountByName";
            const std::string m_strDexShareBgValue =  "/ShareWebServices/Services/Publisher/ReadPublisherLatestGlucoseValues";
            resource_t        m_setNighScout       =  { "/api/v1/entries.json", "api/v1/entries", "api/v1/entries/sgv", "api/v1/entries/{id: [a-z]}" };

            void entry_handler( const std::shared_ptr< rest::Session > session );


        public:
            void startRestServer( const uint16_t a_unPort, data::bg_cache *a_pCache, logging::log a_log );
            void stopRestServer();
            void getGlucose( uint32_t a_nNumberOfEvents );

    };

}
