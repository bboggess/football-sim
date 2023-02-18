#include <mlpack/core/data/load.hpp>
#include <mlpack/methods/softmax_regression/softmax_regression.hpp>
#include <cstdlib>

#include "model.h"
#include "learn.h"
#include "../engine/game.h"
#include "../engine/clock.h"

using namespace mlpack;
using namespace mlpack::regression;

static SoftmaxRegression model;

void initModel() {
	data::Load<SoftmaxRegression>(MODEL_FILENAME, MODEL_NAME, model);
}

/**
 * Turns situation into a vector the model understands.
 */
static void loadDataVector(Situation *sit, arma::mat &data, size_t numFeatures) {
	data = arma::mat(numFeatures, 1);

	data.at(0,0) = sit->clock->getQuarter();
	data.at(1,0) = sit->clock->getMinutes();
	data.at(2,0) = sit->clock->getSeconds();
	data.at(3,0) = static_cast<int>(sit->down);
	data.at(4,0) = sit->distance;
	data.at(5,0) = sit->fieldPos;
}

PlayCall getPlayCall(Situation *sit) {
	arma::mat probabilities, data;
	loadDataVector(sit, data, model.FeatureSize());
	model.Classify(data, probabilities);

	double runThresh = probabilities.at(0,0) * 100;
	double shortPassThresh = probabilities.at(1,0) * 100 + runThresh;
	unsigned int roll = rand() % 100;

	if (roll <= runThresh)
		return RUN;
	else if (roll <= shortPassThresh)
		return SHORT_PASS;
	else
		return LONG_PASS;
}
