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
  rest lib sending
     https://github.com/whoshuu/cpr
     https://github.com/whoshuu/cpr.git
     cd cpr && git submodule update --init --recursive

     apt install libssl-dev

     mkdir build && cd build
     cmake ..
     make
     export LD_LIBRARY_PATH=../cpr/lib/
     
  rest lib server
  -------------------
    https://github.com/Corvusoft/restbed/
    apt install librestbed-dev



  json lib
  -------------------------------------
  https://github.com/nlohmann/json
  https://nlohmann.github.io/json/


  logging
  -------------------------------------
  spdlog - not using hdr only to reduce compile time
    https://github.com/gabime/spdlog

  hostsetup
  -------------
  set locat time -> ex ln -s /usr/share/zoneinfo/America/New_York /etc/localtime

sample config
-----------------------
account=<your account login>
password=<account pwd>
applicationid=<app id>
httptimeout=20
logpath=...
loglevel=debug
# levels avail: info|warn|error|debug
note: leave out '<' and '>'

sample test query
-----------------------
 curl  "http://127.0.0.1//api/v1/entries.json?count=83&rr=1587921452717" -H  "accept: application/json"| python3 -m json.tool
 count is number of most recent data points to collect
 rr - ??



nightscout info
-------------------------
https://github.com/nightscout/cgm-remote-monitor#nightscout-api


TODO
------------------------------------
* get login working - done
* get BG call working - done
* basic logging - done
* basic read config - done
* class to manage rest dexcom calls - done
 has to handle errors - in progress
*  has to handle re-login  - done - not tested
 handle when to request new data based on system time of last received
 handle missing data requests
* cache for BG - done
* will read from logs at start (if avail) - done
* write bg to logs - done
write bg to db 
* manage rest reader and writer - done
* notifications of problems- - done
handle more than one user
encode pwd in config
* handle errors in classes under mgr class - done
* set next read absed on last read date (5 min interval) - done
* look into what happends when last log entry is the next one read (ie restart less than 5 min after last read) - done

* look into error when sending a_log as a ref
void _start( std::shared_ptr<sync_tools::monitor> a_pSync, logging::log a_log );


