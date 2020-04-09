//#define CATCH_CONFIG_MAIN
//#include "catch/catch.hpp"
#include "../../../catchsi/catch.hpp"
#include "../include/bg_cache.h"
#include "../include/common.h"

#include <vector>
#include <string>

TEST_CASE( "test cache", "[cache]" )
{
    const int32_t nSize = 4;
    data::bg_cache ch( nSize );

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

        auto [bOk1, vItem1 ] = ch.front( 0 );
        REQUIRE( bOk1 == false );
        REQUIRE( vItem1.size() == 0 );

        auto [bOk2, vItem2 ] = ch.front( 1 );
        REQUIRE( bOk2 == true );
        REQUIRE( vItem2.size() == 1 );

        auto [bOk3, vItem3 ] = ch.front( 3 );
        REQUIRE( bOk3 == true );
        REQUIRE( vItem3.size() == 3 );

        auto [bOk4, vItem4 ] = ch.front( 4 );
        REQUIRE( bOk4 == false );
        REQUIRE( vItem4.size() == 0 );

        REQUIRE( ch.verify_request( 0 ) == false );
        REQUIRE( ch.verify_request( 4 ) == false );
        REQUIRE( ch.verify_request( 5 ) == false );
        REQUIRE( ch.verify_request( 1 ) == true );
        REQUIRE( ch.verify_request( 2 ) == true );
        REQUIRE( ch.verify_request( 3 ) == true );

    }

    SECTION( "check pushing past capacity" )
    {
        // after 4 items, old ones are pushed out
        ch.push( 1000, 1000, 1000, 103, 4 );
        ch.push( 1001, 1001, 1001, 104, 4 );
        ch.push( 1002, 1002, 1002, 105, 5 );

        INFO( "test requesting too many items" );
        REQUIRE( ch.verify_request( 3 ) == true );
        REQUIRE( ch.verify_request( 4 ) == false );
        auto [bOk0, vItem0 ] = ch.front( 4 );
        REQUIRE( bOk0 == false );
        REQUIRE( vItem0.size() == 0 );

        ch.push( 1003, 1003, 1003, 107, 5 );  // q full
        REQUIRE( ch.verify_request( 4 ) == true );
        auto [bOk1, vItem1 ] = ch.front( 1 );
        REQUIRE( bOk1 == true );
        REQUIRE( vItem1.size() == 1 );
        REQUIRE( vItem1[0].DT == 1003 );

        INFO( "test eviction" );
        ch.push( 1004, 1004, 1004, 108, 5 );  // 1000 pushed out
        REQUIRE( ch.verify_request( 4 ) == true );
        auto [bOk2, vItem2 ] = ch.front( 1 );
        REQUIRE( vItem2.size() == 1 );
        REQUIRE( vItem2[0].DT == 1004 );

        INFO( "test eviction" );
        ch.push( 1005, 1005, 1005, 108, 5 );  // 1001 pushed out
        REQUIRE( ch.verify_request( 4 ) == true );
        auto [bOk3, vItem3 ] = ch.front( 1 );
        REQUIRE( bOk3 == true );
        REQUIRE( vItem3.size() == 1 );
        REQUIRE( vItem3[0].DT == 1005 );

        INFO( "test items after eviction" );
        auto [bOk4, vItem4 ] = ch.front( 5 );
        REQUIRE( bOk4 == false );

        auto [bOk5, vItem5 ] = ch.front( 4 );
        REQUIRE( vItem5.size() == 4 );
        REQUIRE( vItem5[0].DT == 1005 );
        REQUIRE( vItem5[1].DT == 1004 );
        REQUIRE( vItem5[2].DT == 1003 );
        REQUIRE( vItem5[3].DT == 1002 );

    }

}


TEST_CASE( "date tick", "[date]" )
{
    SECTION( "convert date tick to string" )
    {
        uint64_t dt = 1586450348000;  // seconds in epoc format
        uint64_t st = 1586464748000;
        uint64_t wt = 1586464748000;
        std::string strDateDt;
        std::string strDateSt;
        std::string strDateWt;
        strDateDt = common::timeTickToString( dt, strDateDt );
        strDateSt = common::timeTickToString( st, strDateSt );
        strDateWt = common::timeTickToString( wt, strDateWt );
        INFO( "DT:" << strDateDt );
        INFO( "ST:" << strDateSt );
        INFO( "WT:" << strDateWt );
        REQUIRE( strDateDt == "09/04/2020 16:39:08+0000" );
        REQUIRE( strDateSt == "09/04/2020 20:39:08+0000" );
        REQUIRE( strDateWt == "09/04/2020 20:39:08+0000" );


    }
}
