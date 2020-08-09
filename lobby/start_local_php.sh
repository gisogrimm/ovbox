#!/bin/bash
(
    cd www
    ovboxuser=Giso php -S localhost:8081 &
    ovboxuser=admin php -S localhost:8082 &
    ovboxuser=device php -S localhost:8083 &
    ovboxuser=room php -S localhost:8084 &
)
