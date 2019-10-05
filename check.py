#!/usr/bin/env python3

from collections import namedtuple
import json
import os

TimeVal = namedtuple("TimeVal", ["sec", "usec"])

def check():

    os.chdir("results")

    data = {}
    for f in filter(os.path.isfile, os.listdir()):
        with open(f, "r") as fson:
            data[int(f.split('.')[0].split('_')[-1])] = json.load(fson)

    # de-JSON-ify
    for proc, val in data.items():
        for barrier in val["barriers"]:
            barrier["arr"] = TimeVal(**barrier["arr"])
            barrier["dep"] = TimeVal(**barrier["dep"])
        data[proc] = val["barriers"]

    proc_barrier = []
    for proc, _ in enumerate(data):
        proc_barrier.append(data[proc])

    # Invert list so we barrier is first index
    arr_times = []
    dep_times = []
    for _ in enumerate(proc_barrier[0]):
        arr_times.append([])
        dep_times.append([])

    for proc in proc_barrier:
        for b, barrier in enumerate(proc):
            arr_times[b].append(barrier["arr"])
            dep_times[b].append(barrier["dep"])

    # Check if arrival times at barrier B are always less than any of the
    # departure times for that barrier

    for b, _ in enumerate(arr_times):
        latest_arrival = max(arr_times[b])
        earliest_departure = min(dep_times[b])
        try:
            assert latest_arrival <= earliest_departure
        except AssertionError:
            print("Failed on barrier {}:".format(b))
            print("  latest arrival is after earliest departure")
            print("  arr: {}".format(latest_arrival))
            print("  dep: {}".format(earliest_departure))
            return False

    return True
