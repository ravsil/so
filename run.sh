#!/bin/bash

trace-cmd record -e sched_switch -e sched_wakeup ./benchmark
trace-cmd report trace.dat > processos.txt

