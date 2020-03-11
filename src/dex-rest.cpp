#include "../include/dex-rest.h"
#include <sstream>

#include <boost/format.hpp>

using namespace std;

dexcom_share::returnType dexcom_share::login()
{
   string strAuth = R"({"accountName" : "%s", "password" : "%s", "applicationId" : "%s"})";
   stringstream sstr;
   sstr << boost::format( strAuth ) % m_strUserName % m_strPassword % m_strAccoundId;

   string strUrl = m_strShareUrlbase + m_strShareLogin;


   return std::nullopt;
}

dexcom_share::returnType dexcom_share::getBloodSugar()
{

   return std::nullopt;
}
