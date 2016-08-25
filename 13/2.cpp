#include <cassert>
#include <cmath>
#include <iostream>
#include <experimental/optional>
#include <random>
#include <type_traits>

using namespace std;
using namespace std::experimental;

template<
		typename F,
		typename Pdf,
		typename CdfInverse,
		typename VarianceOut = nullptr_t,

		typename = enable_if<
			is_same< double *, VarianceOut >::value
				|| is_same< nullptr_t, VarianceOut >::value
		>
>
double monty(
		F f, Pdf pdf, CdfInverse cdf_inverse,
		double range_min, double range_max, size_t N,
		VarianceOut variance_out = nullptr
) {
	constexpr bool COMPUTE_VARIANCE = is_same< double *, VarianceOut >::value;

	auto range = range_max - range_min;
	assert( range >= 0 );

	default_random_engine engine( random_device{}() );
	uniform_real_distribution<double> distrib( 0.0, 1.0 );
	auto rand = [ &engine, &distrib ] () {
		return distrib( engine );
	};

	double sum = 0.0;
	double variance_sum = 0.0;
	double expected_so_far = 0.0;
	for( size_t i = 0; i < N; ++i ) {
		auto x = cdf_inverse( rand() );
		auto f_x = f(x);
		auto pdf_x = pdf(x);

		sum += f_x / pdf(x);
		if constexpr ( COMPUTE_VARIANCE ) {
			expected_so_far = sum / (i + 1);
			variance_sum += expected_so_far * expected_so_far;
		}
	}
	double expected = sum / N;

	if constexpr ( COMPUTE_VARIANCE ) {
		*variance_out = variance_sum / N - expected * expected;
	}

	return sum / N;
}


// template< typename F, typename Pdf, typename CdfInverse, typename Variance = nullopt_t >
// double monty(
// 		F f, Pdf pdf, CdfInverse cdf_inverse,
// 		double range_min, double range_max, size_t N,
// 		std::optional<double &> variance_out = nullopt
// ) {
//
// 	auto range = range_max - range_min;
// 	assert( range >= 0 );
//
// 	default_random_engine engine( random_device{}() );
// 	uniform_real_distribution<double> distrib( 0.0, 1.0 );
// 	auto rand = [ &engine, &distrib ] () {
// 		return distrib( engine );
// 	};
//
// 	double sum = 0;
// 	for( size_t i = 0; i < N; ++i ) {
// 		auto x = cdf_inverse( rand() );
// 		sum += f(x) / pdf(x);
// 	}
//
// 	return sum / N;
// }


int main( int argc, char **argv ) {
	
	for ( size_t i = 2; i <= 128 * 2048; i *= 2 ) {
		double variance;
		double estimate = monty(
				[] ( double x ) { return x * x; },
				[] ( double x ) { return x / 2.0; },
				[] ( double y ) { return sqrt( 4.0 * y ); },
				0.0,
				2.0,
				i,
				&variance
		);

		cout << "N=" << i << "; E=" << estimate << "; V=" << variance << "\n";
	}
}
