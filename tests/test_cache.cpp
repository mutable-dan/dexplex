//#define CATCH_CONFIG_MAIN
//#include "catch/catch.hpp"
#include "../../../catchsi/catch.hpp"
#include "../include/bg_cache.h"

#include <vector>

TEST_CASE( "test cache", "[cache]" )
{
    tools::bg_cache ch( 4 );

    SECTION( "pushing to container" )
    {
        REQUIRE( ch.size() == 0 );
        ch.push( 1000, 1000, 1000, 103, 4 );
        ch.push( 1001, 1001, 1001, 104, 4 );
        ch.push( 1002, 1002, 1002, 105, 5 );
        REQUIRE( ch.size() == 3 );
    }

    SECTION( "check front element" )
    {
        ch.push( 1000, 1000, 1000, 103, 4 );
        ch.push( 1001, 1001, 1001, 104, 4 );
        ch.push( 1002, 1002, 1002, 105, 5 );

        const auto [bOk, item] = ch.front();
        REQUIRE( bOk == true );
        REQUIRE( item.DT == 1002 );
        REQUIRE( item.value == 105 );
    }

    SECTION( "check getting top n elements with none" )
    {
        auto [bOk, vItem ] = ch.front( 2 );
        REQUIRE( bOk == false );
        REQUIRE( vItem.size() == 0 );
    }

    SECTION( "check getting top n elements with 2" )
    {
        ch.push( 1000, 1000, 1000, 103, 4 );
        ch.push( 1001, 1001, 1001, 104, 4 );
        ch.push( 1002, 1002, 1002, 105, 5 );

        auto [bOk, vItem ] = ch.front( 2 );
        REQUIRE( bOk == true );
        REQUIRE( vItem.size() == 2 );
        INFO( "count:" << vItem.size() );
        INFO( "item[0]:" << vItem[0].DT );
        INFO( "item[1]:" << vItem[1].DT );
        REQUIRE( vItem[0].DT == 1002 );
        REQUIRE( vItem[1].DT == 1001 );
    }
}
