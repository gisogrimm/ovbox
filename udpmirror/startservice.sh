#!/bin/bash
killall roomservice
sleep 2
killall roomservice
sleep 2
killall roomservice
sleep 1
for p in 4367 {4933..4936} {5933..5940}; do
    ./roomservice -l http://box.orlandoviols.com  -p $p -n Frankfurt &
    sleep 1
done
for p in {5950..5959}; do
    ./roomservice -l http://box.orlandoviols.com  -p $p -n HfKBremen${p} &
    sleep 1
done
