// test rest connecti-lboost_system -lboost_filesystem -L../bin -lmuxons
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Weffc++"
   #include <cpr/cpr.h>
#pragma GCC diagnostic pop
//#include <cpprest/http_client.h>
//#include <cpprest/filestream.h>

// #include <cpprest/filestream>

using namespace std;

int main( int argc, char *argv[] )
{
   (void)argc;
   (void)argv;


    auto r = cpr::Get( cpr::Url{"https://shawanga.com"},
                       cpr::Authentication{"user", "pass"},
                       cpr::Parameters{{"anon", "true"}, {"key", "value"}});
    cout << r.status_code << endl;                  // 200
    cout << r.header["content-type"].c_str() << endl;       // application/json; charset=utf-8
    cout << r.text << endl;                         // JSON text string

    return 0;
}
