# dexplex
client multiplexer for dexcom 


 dependancies
 ----------------
  # not used --- cpprestsdk - https://github.com/Microsoft/cpprestsdk
  # not used --  apt install libcpprest-dev

  rest lib
     https://github.com/whoshuu/cpr
     https://github.com/whoshuu/cpr.git
     cd cpr && git submodule update --init --recursive

     apt install libssl-dev

     mkdir build && cd build
     cmake ..
     make
     

export LD_LIBRARY_PATH=../cpr/lib/


