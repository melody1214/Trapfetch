#!/bin/bash

sudo -u melody echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null
sync

