#include <mlpack/prereqs.hpp>
#include <mlpack/core.hpp>
#include <mlpack/core/data/split_data.hpp>
#include <mlpack/core/data/save.hpp>
#include <mlpack/methods/softmax_regression/softmax_regression.hpp>

#include <iostream>

#include "learn.h"

using namespace mlpack;
using namespace mlpack::regression;

/* This parameter is passed into regression model. Smaller values lead to too
 * certain of an algorithm, thus leading to overly predictable playcalling.
 *
 * Lower values seem to lead to more passing. With two teams playing with this
 * value of lambda, teams average about 18.5 points, 230 yards passing, and 80
 * yards rushing. Not terrible.
 */
constexpr int LAMBDA = 100;

/**
 * Trains a play calling model from play by play data from 2019 season.
 *
 * Uses a logistic regression model, with three classes (run, short pass,
 * long pass).
 *
 * After running, the model will be saved to the filename MODEL_FILENAME
 * for use by the main engine.
 */
int main(int argc, char *argv[]) {
	arma::mat dataset;
	data::Load(argv[1], dataset, true);

    arma::mat train, valid;
    data::Split(dataset, train, valid, 0.3);

	arma::mat trainX = train.submat(1, 0, train.n_rows - 1, train.n_cols - 1);
	arma::mat validX = valid.submat(1, 0, valid.n_rows - 1, valid.n_cols - 1);

	arma::Row<size_t> trainY = arma::conv_to<arma::Row<size_t>>::from(
								train.row(train.n_rows - 1));

	arma::Row<size_t> validY = arma::conv_to<arma::Row<size_t>>::from(
								valid.row(valid.n_rows - 1));

	// Number of classes in the dataset.			
	const size_t numClasses = arma::max(arma::max(trainY)) + 1;

	SoftmaxRegression model = SoftmaxRegression(trainX, trainY, numClasses, LAMBDA);

	arma::Row<size_t> predictions;
	std::cout << "Training accuracy: " << model.ComputeAccuracy(validX, validY)
			  << std::endl;

	data::Save(MODEL_FILENAME, MODEL_NAME, model);

	return 0;
}
