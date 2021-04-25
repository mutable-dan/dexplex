# dexplex
Sits between dexcom share and xdrip clients.  

- dexplex gets BG data from dexcom
- save historic records
- provides interface for xdrip --> http://host/api/v1/entries.json?count=1
- can reduce the load on dexcom by having many clients connect

Dexcom produces a CGM (continuous glucose monitor) and has a 'share' interface to allow caregivers to monitor a user.
Opensource projects such as xDrip+ https://jamorham.github.io/#xdrip-plus provide excellent alternative monitoring solutions for android.  xDrip can connect directly to dexcom share or you can setup your own server with https://github.com/nightscout


### dependencies
 ----------------
 libcurl
  ubuntu 18.04: apt install libcurl4-gnutls-dev 

#### cpr
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
     
#### rest lib server
  -------------------
    https://github.com/Corvusoft/restbed/
    apt install librestbed-dev



#### json lib
  -------------------------------------
*  https://github.com/nlohmann/json
*  https://nlohmann.github.io/json/


#### logging
  -------------------------------------
  spdlog - compile (don't use header only to reduce compile time)
    https://github.com/gabime/spdlog

#### hostsetup
  -------------
  set locat time -> ex ln -s /usr/share/zoneinfo/America/New_York /etc/localtime

### sample config
-----------------------

```

account=<your account login>
password=<account pwd>
applicationid=<app id>
httptimeout=20
logpath=...
loglevel=debug
# levels avail: info|warn|error|debug
httptimeout=20
# timeout for requests to dexcom in seconds
port=8080
# listen port for xdrip requests

```


### sample test query
-----------------------
 curl  "http://127.0.0.1//api/v1/entries.json?count=83&rr=1587921452717" -H  "accept: application/json"| python3 -m json.tool
 count is number of most recent data points to collect
 rr - System.currentTimeMillis from Models/JoH.java line 177
 ```
 55         Call<List<Entry>> getEntries(
                    @Header("api-secret") String secret,
                    @Query("count") int count,
                    @Query("rr") String rr);

107    getService().getEntries(
           session.url.getHashedSecret(),
           count,
           JoH.tsl() + "").enqueue(session.entriesCallback);

```



### nightscout and other info
-------------------------
* https://github.com/nightscout/cgm-remote-monitor#nightscout-api
* https://github.com/nightscout/cgm-remote-monitor/wiki/API-v1.0.0-beta-Security
* http://www.nightscout.info/wiki/labs/interpreting-raw-dexcom-data
* https://github.com/nightscout/share2nightscout-bridge/blob/master/index.js
* https://www.hanselman.com/blog/BridgingDexcomShareCGMReceiversAndNightscout.aspx
* https://github.com/NightscoutFoundation/xDrip/issues/830        api secret
* https://github.com/NightscoutFoundation/xDrip/blob/f96119cf4a7911e4fef62bc510a45481ba7467d2/app/src/main/java/com/eveningoutpost/dexdrip/cgm/nsfollow/utils/NightscoutUrl.java#L83  code for url builder
* https://github.com/nightscout/share2nightscout-bridge/blob/master/index.js code for secret


### setting up reverse proxy
----------------------------------
#### lighthttpd
    for ubuntu: add 'apt install gamin' if fails to start
 /etc/lighttpd/lighttpd.conf sample
     server.modules += ( "mod_proxy" )
     proxy.debug = 65535
     proxy.server = ( "" => ( "" => ( "host" => "127.0.0.1", "port" => 8080 ) ) )

```

     server.document-root        = "/var/www/html"
     server.upload-dirs          = ( "/var/cache/lighttpd/uploads" )

     server.errorlog             = "/var/log/lighttpd/error.log"
     server.pid-file             = "/var/run/lighttpd.pid"
     server.username             = "www-data"
     server.groupname            = "www-data"
     server.port                 = 80

     compress.cache-dir          = "/var/cache/lighttpd/compress/"
     compress.filetype           = ( "application/json", "application/javascript", "text/css", "text/html", "text/plain" )
```

```
    passes all through
    to pass only rntries through
       $HTTP["url"] =~ "^/api/v1/entries.json" {
       proxy.header = ("map-urlpath" => ( "/api/v1/entries.json" => "/" ))
       proxy.server = ( "" => ( ( "host" =>  "127.0.0.1", "port" => 8080 ) ) )
       }
```

#### caddy
--------------
    domain.tld:443
    reverse_proxy 1.27.0.0.1:8080




TODO
------------------------------------
- encode pwd in config
- handle missing data requests
- write bg to db
- handle more than one user
- handle when to request new data based on system time of last received - ie
- * get login working - done
- * get BG call working - done
- * basic logging - done
- * basic read config - done
- * class to manage rest dexcom calls - done
- * has to handle errors - in progress
- *  has to handle re-login  - done - not tested
- * cache for BG - done
- * will read from logs at start (if avail) - done
- * write bg to logs - done
- * manage rest reader and writer - done
- * notifications of problems- - done
- * handle errors in classes under mgr class - done
- * set next read absed on last read date (5 min interval) - done
- * look into what happends when last log entry is the next one read (ie restart less than 5 min after last read) - done
- * look into error when sending a_log as a ref
