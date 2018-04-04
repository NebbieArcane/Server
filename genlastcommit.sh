#!/bin/bash
touch --date "$(git log -n1 --pretty=format:'%cr')" $1
