# Nebbie Arcane
##Server code
* Getting started
    * FIRST TIME
        * apt install vagrant
        * ./getworld
        * vagrant up
        * vagrant ssh
        * cd /vagrant
        * autoreconf
        * automake --add-missing
        * ./configure
        * make
        * cd bin
        * ./myst
        * vagrant up
    * DAY BY DAY
        * vagrant up
        * vagrant ssh
        * make
        * cd mudroot
        * ./myst
    * In emergency:
        * ./build.sh
            
##IMPORTANT
* ALL FILES inside src will be taken and compiled in myst. Do not put under src unrelatec cod
use utilities for this        
* The /vagrant folder in the vagrant machine is mounted from the folder where the vagrantfile resides and is synced 2-way





