#!/bin/bash

#gnome-terminal -e "bash -c \"cd ${PWD##*/}; . ~/.profile; exec bash\""

#gnome-terminal -e "bash -c \"cd ${PWD##*/}; PATH=~/code/tools/gcc-arm-none-eabi/bin:$PATH; exec bash\""

gnome-terminal -e "bash -c \"cd ${PWD##*/}; exec bash\""
