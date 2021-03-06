#!/bin/sh

handystats_config="{
	\"enabled\": true,
	\"defaults\": {
		\"moving-interval\": 1000,
		\"histogram-bins\": 20,
		\"tags\": []
	},
	\"load_test.counter.*\": {
		\"tags\": [\"value\", \"moving-avg\"]
	},
	\"load_test.gauge.*\": {
		\"tags\": [\"value\", \"moving-avg\"]
	},
	\"load_test.timer.*\": {
		\"tags\": [\"quantile\", \"moving-avg\"]
	}
}"

# Number of working threads (excluding handystats' thread and stats printing thread)
threads=2

# Number of metrics with specific type (sum must be > 0)
gauges=5
counters=5
timers=5

# Stats printing interval (ms)
output_interval=1000

# Array of pairs (rate, time_limit), time_limit in seconds
steps=(
1000 10
5000 10
10000 10
20000 10
50000 10
100000 10
)

exe_path="`pwd`/load"

if [ $# -ge 1 ];
then
	exe_path=$1;
fi

$exe_path \
--handystats-config "$handystats_config" \
--threads $threads \
--gauges $gauges \
--counters $counters \
--timers $timers \
--step ${steps[*]} \
--output-interval $output_interval \

