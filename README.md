# dexplex
client multiplexer for dexcom 


 dependancies
 ----------------
 libcurl
  ubuntu 18.04: apt install libcurl4-gnutls-dev 

  cpr
  ----------------------------------
  rest lib
     https://github.com/whoshuu/cpr
     https://github.com/whoshuu/cpr.git
     cd cpr && git submodule update --init --recursive

     apt install libssl-dev

     mkdir build && cd build
     cmake ..
     make
     

     export LD_LIBRARY_PATH=../cpr/lib/

  json lib
  -------------------------------------
  https://github.com/nlohmann/json


  logging
  -------------------------------------
  spdlog - not using hdr only to reduce compile time

  https://github.com/gabime/spdlog



TODO
------------------------------------
get login working - done
get BG call working - done
basic logging - done
basic read config - done
class to manage rest dexcom calls - in progress
 has to handle errors - in progress
 has to handle re-login 
 handle when to request new data based on system time of last received
 handle missing data requests
cache for BG
 will read from logs at start (if avail)
write bg to logs and db - done
manage rest reader and writer - in progress
notifications of problems
handle more than one user
encode pwd in config


