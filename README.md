[![Build Status](https://api.travis-ci.org/NebbieArcane/Server.svg)](https://travis-ci.org/NebbieArcane/Server)
# Nebbie Arcane
## Server code
* Getting started
    * FIRST TIME
        * apt install vagrant
        * vagrant plugin install vagrant-timezone
        * ./getworld
        * vagrant up
        * vagrant ssh
    * DAY BY DAY
        * vagrant up
        * vagrant ssh
        * cd /vagrant
        * make
        * cd mudroot
        * ./myst
    * In emergency (or when you add a new header or code file to src):
        * ./build.sh # to rebuild the makefiles

## IMPORTANT
* ALL FILES inside src will be taken and compiled in myst. Do not put under src unrelated files.
 use utilities for this
* The /vagrant folder in the vagrant machine is mounted from the folder where the vagrantfile resides and is synced 2-way




