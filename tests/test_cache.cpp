#define CATCH_CONFIG_MAIN
//#include "catch/catch.hpp"
#include "../../../catch/catch.hpp"
#include "../include/bg_cache.h"

TEST_CASE( "test cache", "[cache]" )
{
    tools::bg_cache ch;
    ch.push( 1000, 1000, 1000, 103, 4 );
    ch.push( 1001, 1001, 1001, 104, 4 );

//    auto& [bOk, item] = ch.front();

}
