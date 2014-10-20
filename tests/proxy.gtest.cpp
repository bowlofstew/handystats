#include <chrono>

#include <gtest/gtest.h>

#include <handystats/chrono.hpp>
#include <handystats/metrics.hpp>
#include <handystats/measuring_points.hpp>
#include <handystats/core.hpp>
#include <handystats/metrics_dump.hpp>

#include "message_queue_helper.hpp"
#include "metrics_dump_helper.hpp"


class HandyProxyTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		HANDY_CONFIG_JSON(
				"{\
					\"dump-interval\": 10,\
					\"timer\": {\
						\"idle-timeout\": 500\
					}\
				}"
			);

		HANDY_INIT();
	}

	virtual void TearDown() {
		HANDY_FINALIZE();
	}
};


TEST_F(HandyProxyTest, GaugeProxy) {
	const char* gauge_name = "gauge";
	const size_t VALUE_MIN = 10;
	const size_t VALUE_MAX = 20;

	handystats::measuring_points::gauge_proxy gauge_proxy(gauge_name);

	for (size_t value = VALUE_MIN; value <= VALUE_MAX; ++value) {
		gauge_proxy.set(value);
	}

	handystats::message_queue::wait_until_empty();
	handystats::metrics_dump::wait_until(handystats::chrono::system_clock::now());

	auto metrics_dump = HANDY_METRICS_DUMP();
	ASSERT_TRUE(metrics_dump->first.find(gauge_name) != metrics_dump->first.end());

	auto& gauge_values = metrics_dump->first.at(gauge_name);
	ASSERT_EQ(gauge_values.get<handystats::statistics::tag::count>(), VALUE_MAX - VALUE_MIN + 1);
	ASSERT_EQ(gauge_values.get<handystats::statistics::tag::min>(), VALUE_MIN);
	ASSERT_EQ(gauge_values.get<handystats::statistics::tag::max>(), VALUE_MAX);
	ASSERT_EQ(gauge_values.get<handystats::statistics::tag::avg>(), (VALUE_MIN + VALUE_MAX) / 2);
}

TEST_F(HandyProxyTest, CounterProxy) {
	const char* counter_name = "counter";
	const size_t INIT_VALUE = 10;
	const size_t DELTA = 2;
	const size_t DELTA_STEPS = 10;

	handystats::measuring_points::counter_proxy counter_proxy(counter_name, INIT_VALUE);

	for (size_t step = 0; step < DELTA_STEPS; ++step) {
		counter_proxy.increment(DELTA);
	}

	for (size_t step = 0; step < DELTA_STEPS; ++step) {
		counter_proxy.decrement(DELTA);
	}

	handystats::message_queue::wait_until_empty();
	handystats::metrics_dump::wait_until(handystats::chrono::system_clock::now());

	auto metrics_dump = HANDY_METRICS_DUMP();
	ASSERT_TRUE(metrics_dump->first.find(counter_name) != metrics_dump->first.end());

	auto& counter_values = metrics_dump->first.at(counter_name);

	ASSERT_EQ(counter_values.get<handystats::statistics::tag::value>(), INIT_VALUE);
	ASSERT_EQ(counter_values.get<handystats::statistics::tag::count>(), 1 + DELTA_STEPS * 2);
	ASSERT_EQ(counter_values.get<handystats::statistics::tag::min>(), INIT_VALUE);
	ASSERT_EQ(counter_values.get<handystats::statistics::tag::max>(), INIT_VALUE + DELTA * DELTA_STEPS);
}

TEST_F(HandyProxyTest, TimerProxySingleInstance) {
	const char* timer_name = "timer";
	const handystats::metrics::timer::instance_id_type instance_id = 111;
	const std::chrono::milliseconds sleep_interval(1);
	const size_t SLEEP_COUNT = 10;

	handystats::measuring_points::timer_proxy timer_proxy(timer_name, instance_id);

	for (size_t sleep_step = 0; sleep_step < SLEEP_COUNT; ++sleep_step) {
		timer_proxy.start(sleep_step); // passed instance_id should not override default one
		std::this_thread::sleep_for(sleep_interval);
		timer_proxy.stop(sleep_step + 1); // again, no influence
	}

	handystats::message_queue::wait_until_empty();
	handystats::metrics_dump::wait_until(handystats::chrono::system_clock::now());

	auto metrics_dump = HANDY_METRICS_DUMP();
	ASSERT_TRUE(metrics_dump->first.find(timer_name) != metrics_dump->first.end());

	auto& timer_values = metrics_dump->first.at(timer_name);

//	ASSERT_EQ(timer.instances.size(), 0);

	ASSERT_GE(
			timer_values.get<handystats::statistics::tag::value>(),
			handystats::chrono::duration::convert_to(
				handystats::metrics::timer::value_unit,
				handystats::chrono::milliseconds(sleep_interval.count())
			).count()
		);

	ASSERT_EQ(timer_values.get<handystats::statistics::tag::count>(), SLEEP_COUNT);
	ASSERT_GE(
			timer_values.get<handystats::statistics::tag::min>(),
			handystats::chrono::duration::convert_to(
				handystats::metrics::timer::value_unit,
				handystats::chrono::milliseconds(sleep_interval.count())
			).count()
		);
}

TEST_F(HandyProxyTest, TimerProxyMultiInstance) {
	const char* timer_name = "timer";
	const std::chrono::milliseconds sleep_interval(1);
	const size_t SLEEP_COUNT = 10;

	handystats::measuring_points::timer_proxy timer_proxy(timer_name);

	for (size_t sleep_step = 0; sleep_step < SLEEP_COUNT; ++sleep_step) {
		timer_proxy.start(sleep_step);
	}

	std::this_thread::sleep_for(sleep_interval);

	for (size_t sleep_step = 0; sleep_step < SLEEP_COUNT; ++sleep_step) {
		timer_proxy.stop(sleep_step);
	}

	handystats::message_queue::wait_until_empty();
	handystats::metrics_dump::wait_until(handystats::chrono::system_clock::now());

	auto metrics_dump = HANDY_METRICS_DUMP();
	ASSERT_TRUE(metrics_dump->first.find(timer_name) != metrics_dump->first.end());

	auto& timer_values = metrics_dump->first.at(timer_name);

//	ASSERT_EQ(timer.instances.size(), 0);

	ASSERT_GE(
			timer_values.get<handystats::statistics::tag::value>(),
			handystats::chrono::duration::convert_to(
				handystats::metrics::timer::value_unit,
				handystats::chrono::milliseconds(sleep_interval.count())
			).count()
		);

	ASSERT_EQ(timer_values.get<handystats::statistics::tag::count>(), SLEEP_COUNT);
	ASSERT_GE(
			timer_values.get<handystats::statistics::tag::min>(),
			handystats::chrono::duration::convert_to(
				handystats::metrics::timer::value_unit,
				handystats::chrono::milliseconds(sleep_interval.count())
			).count()
		);
}
