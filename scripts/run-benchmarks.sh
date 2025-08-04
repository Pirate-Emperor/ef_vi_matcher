#!/bin/bash -x

# Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

now=$(date --utc +%Y%m%dT%H%M%S)
cpucount=$(grep -c ^processor /proc/cpuinfo)
exe="$(dirname "$0")/../benchmarks"

source "$(dirname "$0")/efviBenchmark.sh"

function efviBenchmark() {
    lb="/usr/bin/stdbuf -oL"
    let N=${N:-33}
    efviFor((i=1;i<=N;++i)); do
        $lb echo -n "[$i/$N] "
        sudo chrt -f 50 "$exe"
    done
}

efviPrologue
efviBenchmark | tee results-${cpucount}.${now}.txt
efviEpilogue


