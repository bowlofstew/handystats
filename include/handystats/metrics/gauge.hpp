#ifndef HANDYSTATS_METRICS_GAUGE_H_
#define HANDYSTATS_METRICS_GAUGE_H_

#include <utility>

#include <handystats/chrono.hpp>

#include <boost/accumulators/framework/accumulator_set.hpp>
#include <boost/accumulators/framework/features.hpp>

#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/accumulators/statistics/sum.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/mean.hpp>

#include <handystats/accumulators/moving_average.hpp>
#include <handystats/accumulators/interval_count.hpp>
#include <handystats/accumulators/interval_sum.hpp>
#include <handystats/accumulators/interval_mean.hpp>


namespace handystats { namespace metrics {

struct gauge
{
	typedef double value_type;
	typedef chrono::default_duration time_duration;
	typedef chrono::default_clock clock;
	typedef chrono::default_time_point time_point;

	struct internal_stats {
		typedef boost::accumulators::features <
				boost::accumulators::tag::min,
				boost::accumulators::tag::max,
				boost::accumulators::tag::sum,
				boost::accumulators::tag::count,
				boost::accumulators::tag::mean,
				boost::accumulators::tag::moving_average,
				boost::accumulators::tag::interval_count,
				boost::accumulators::tag::interval_sum,
				boost::accumulators::tag::interval_mean
			> value_features;

		boost::accumulators::accumulator_set<value_type, value_features> values;

		internal_stats()
			: values(
					boost::accumulators::tag::moving_average::alpha = 2.0 / 16,
					boost::accumulators::time_interval = std::chrono::seconds(1)
					)
		{
		}

		void update_value(value_type value, time_point timestamp);

		value_type min() const;
		value_type max() const;
		value_type sum() const;
		value_type count() const;
		value_type mean() const;
		value_type moving_average() const;
		value_type interval_count() const;
		value_type interval_sum() const;
		value_type interval_mean() const;
	};

	gauge();
	gauge(value_type value, time_point timestamp);

	void set(value_type value, time_point timestamp);

	value_type value;
	time_point timestamp;

	internal_stats stats;

}; // struct gauge

}} // namespace handystats::metrics


#endif // HANDYSTATS_METRICS_GAUGE_H_
