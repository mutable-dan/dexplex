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

#include <string>
#include <optional>


class dexcom_share final
{
   public:
      using returnType = std::optional<std::string>;

   private:
      const std::string       m_strShareUrlbase       = "https://share2.dexcom.com/";
      const std::string       m_strShareLogin         = "ShareWebServices/Services/General/LoginPublisherAccountByName";
      const std::string       m_strShareGetBG         = "ShareWebServices/Services/Publisher/ReadPublisherLatestGlucoseValues";

      std::string             m_strUserName;
      std::string             m_strPassword;
      std::string             m_strAccoundId;

      returnType login();
      returnType getBloodSugar();
      returnType 

   public:
      dexcom_share( dexcom_share& )  = delete;
      dexcom_share( dexcom_share&& ) = delete;

      void userName( const std::string& a_strUserName )   { m_strUserName  = a_strUserName; }
      void password( const std::string& a_strPassword )   { m_strPassword  = a_strPassword; }
      void accoundId( const std::string& a_strAccountId ) { m_strAccoundId = a_strAccountId; }
      
};
