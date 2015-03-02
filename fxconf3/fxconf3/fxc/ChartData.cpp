#pragma once

namespace fxc {

	struct ChartData {
		double*	closes;
		double*	highs;
		double* lows;
		int		bars;
	};

}