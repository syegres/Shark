//===========================================================================
/*!
 * 
 *
 * \brief       Random Forest Trainer
 * 
 * 
 *
 * \author      K. N. Hansen, J. Kremer
 * \date        2011-2012
 *
 *
 * \par Copyright 1995-2017 Shark Development Team
 * 
 * <BR><HR>
 * This file is part of Shark.
 * <http://shark-ml.org/>
 * 
 * Shark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Shark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Shark.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
//===========================================================================


#ifndef SHARK_ALGORITHMS_TRAINERS_RFTRAINER_H
#define SHARK_ALGORITHMS_TRAINERS_RFTRAINER_H

#include <shark/Core/DLLSupport.h>
#include <shark/Algorithms/Trainers/AbstractTrainer.h>
#include <shark/Data/DataView.h>
#include <shark/Models/Trees/RFClassifier.h>
#include <shark/Algorithms/Trainers/CARTcommon.h>

#include <set>

namespace shark {
/*!
 * \brief Random Forest
 *
 * Random Forest is an ensemble learner, that builds multiple binary decision trees.
 * The trees are built using a variant of the CART methodology
 *
 * The algorithm used to generate each tree based on the SPRINT algorithm, as
 * shown by J. Shafer et al.
 *
 * Typically 100+ trees are built, and classification/regression is done by combining
 * the results generated by each tree. Typically the a majority vote is used in the
 * classification case, and the mean is used in the regression case
 *
 * Each tree is built based on a random subset of the total dataset. Furthermore
 * at each split, only a random subset of the attributes are investigated for
 * the best split
 *
 * The node impurity is measured by the Gini criteria in the classification
 * case, and the total sum of squared errors in the regression case
 *
 * After growing a maximum sized tree, the tree is added to the ensemble
 * without pruning.
 *
 * For detailed information about Random Forest, see Random Forest
 * by L. Breiman et al. 2001.
 *
 * For detailed information about the SPRINT algorithm, see
 * SPRINT: A Scalable Parallel Classifier for Data Mining
 * by J. Shafer et al.
 */
class RFTrainer 
: public AbstractTrainer<RFClassifier, unsigned int>
, public AbstractTrainer<RFClassifier>,
  public IParameterizable
{

public:
	using ModelType = RFClassifier;
	using LabelType = RealVector;
	using SubmodelType = CARTClassifier<LabelType>;
	using CARTType = SubmodelType;
	using TreeType = CARTType::TreeType;
	using NodeInfo = CARTType::NodeInfo;
	/// Construct and compute feature importances when training or not
	SHARK_EXPORT_SYMBOL RFTrainer(bool computeFeatureImportances = false, bool computeOOBerror = false);

	/// \brief From INameable: return the class name.
	std::string name() const
	{ return "RFTrainer"; }

	/// Train a random forest for classification.
	SHARK_EXPORT_SYMBOL void train(RFClassifier& model, ClassificationDataset const& dataset);

	/// Train a random forest for regression.
	SHARK_EXPORT_SYMBOL void train(RFClassifier& model, RegressionDataset const& dataset);

	/// Set the number of random attributes to investigate at each node.
	SHARK_EXPORT_SYMBOL void setMTry(std::size_t mtry) { m_try = mtry; }


	/// Set the number of trees to grow.
	void setNTrees(long nTrees) {
		SHARK_RUNTIME_CHECK(nTrees >= 1, "nTrees must be a positive number");
		m_B = nTrees;
	}


	/// Controls when a node is considered pure. If set to 1, a node is pure
	/// when it only consists of a single node.
	SHARK_EXPORT_SYMBOL void setNodeSize(std::size_t nodeSize) { m_nodeSize = nodeSize; }

	/// Set the fraction of the original training dataset to use as the
	/// out of bag sample. The default value is 0.66.
	void setOOBratio(double ratio)
	{
		SHARK_RUNTIME_CHECK(m_OOBratio > 0 && m_OOBratio <= 1, "OOBratio must be in the interval (0,1]");
		m_OOBratio = ratio;
	}


	/// Return the parameter vector.
	RealVector parameterVector() const
	{
		RealVector ret(1); // number of trees
		init(ret) << (double)m_B;
		return ret;
	}

	/// Set the parameter vector.
	void setParameterVector(RealVector const& newParameters)
	{
		SHARK_ASSERT(newParameters.size() == numberOfParameters());
		setNTrees(static_cast<long>(newParameters[0]));
	}

	// set true if the feature importances should be computed
	bool m_computeFeatureImportances;

	// set true if OOB error should be computed
	bool m_computeOOBerror;

	// set true if trainer should bootstrap with replacement
	bool m_bootstrapWithReplacement;

	using ImpurityMeasure = detail::cart::ImpurityMeasure;
	// set to gini, misclassification or crossEntropy as desired
	ImpurityMeasure m_impurityMeasure;

protected:
	/// ClassVector
	using ClassVector = UIntVector;
	using LabelVector = std::vector<LabelType>;
	using Split = detail::cart::Split;

	/// Build a decision tree for classification
	SHARK_EXPORT_SYMBOL TreeType buildTree(detail::cart::SortedIndex&& tables, DataView<ClassificationDataset const> const& elements, ClassVector& cFull, std::size_t nodeId, Rng::rng_type& rng);

	/// Builds a decision tree for regression
	SHARK_EXPORT_SYMBOL TreeType buildTree(detail::cart::SortedIndex&& tables, DataView<RegressionDataset const> const& elements, LabelType const& sumFull, std::size_t nodeId, Rng::rng_type& rng);


	SHARK_EXPORT_SYMBOL RFTrainer::Split findSplit(detail::cart::SortedIndex const& tables, DataView<RegressionDataset const> const& elements, RealVector const& sumFull, std::set<size_t> const& tableIndices) const;
	SHARK_EXPORT_SYMBOL RFTrainer::Split findSplit(detail::cart::SortedIndex const& tables, DataView<ClassificationDataset const> const& elements, ClassVector const& cFull, std::set<size_t> const& tableIndices) const;

	/// Generate random table indices.
	SHARK_EXPORT_SYMBOL std::set<std::size_t> generateRandomTableIndices(Rng::rng_type &rng) const;

	/// Reset the training to its default parameters.
	void setDefaults();

	/// Number of attributes in the dataset
	std::size_t m_inputDimension;

	/// Dimension of a label. Used in Regression
	std::size_t m_labelDimension;
	/// Holds the number of distinct labels. Used in Classification
	std::size_t m_labelCardinality;

	/// number of attributes to randomly test at each inner node
	std::size_t m_try;

	/// number of trees in the forest
	long m_B;

	/// number of samples in the terminal nodes
	std::size_t m_nodeSize;

	/// fraction of the data set used for growing trees
	/// 0 < m_OOBratio < 1
	double m_OOBratio;

	/// true if the trainer is used for regression, false otherwise.
	bool m_regressionLearner;

	// set true if the CART OOB error should be computed for each tree
	bool m_computeCARTOOBerror;


	using ImpurityMeasureFn = detail::cart::ImpurityMeasureFn;

	ImpurityMeasureFn m_impurityFn;
};
}
#endif
