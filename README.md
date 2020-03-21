# dexplex
Dexcom produces a CGM (continuous glucose monitor) and has a 'share' interface to allow caregivers to monitor a user.
Opensource projects such as xDrip+ https://jamorham.github.io/#xdrip-plus provide excellent alternative monitoring solutions for android.  xDrip can connect directly to dexcom share or you can setup your own server with https://github.com/nightscout

The advantage of setting up your own server:
* nightscout give you a many analytics, like xDrip
* reduce the load on dexcom by having many devices connect to your server
* you have your data

There are a few things about the nighscout server that I didn't like, so this project will do one thing only.  It will conect to dexcom share and allow many devices to connect to it, the being a good citizen of dexcom.

1. connect to dexcom
2. cache BG data (blood glucose)
3. provide a rest API for xdrip connections



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
 has to handle re-login  - in progress
 handle when to request new data based on system time of last received
 handle missing data requests
cache for BG
 will read from logs at start (if avail)
write bg to logs and db - done
manage rest reader and writer - in progress
notifications of problems- - in progress
handle more than one user
encode pwd in config
handle errors in classes under mgr class


