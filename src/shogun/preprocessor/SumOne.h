/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Soeren Sonnenburg, Sergey Lisitsyn, Evgeniy Andreev, Yuyu Zhang, 
 *          Saurabh Goyal
 */

#ifndef _CSUMONE__H__
#define _CSUMONE__H__

#include <shogun/lib/config.h>

#include <shogun/preprocessor/DensePreprocessor.h>
#include <shogun/features/Features.h>
#include <shogun/lib/common.h>


namespace shogun
{
/** @brief Preprocessor SumOne, normalizes vectors to have sum 1.
 *
 * Formally, it computes
 *
 * \f[
 * {\bf x} \leftarrow \frac{{\bf x}}{{\sum_i x_i}}
 * \f]
 *
 */
class SumOne : public DensePreprocessor<float64_t>
{
	public:
		/** default constructor */
		SumOne();

		/** destructor */
		~SumOne() override;

		/// initialize preprocessor from file
		virtual bool load(FILE* f);
		/// save preprocessor init-data to file
		virtual bool save(FILE* f);

		/// apply preproc on single feature vector
		/// result in feature matrix
		SGVector<float64_t> apply_to_feature_vector(SGVector<float64_t> vector) override;

		/** @return object name */
		const char* get_name() const override { return "SumOne"; }

		/// return a type of preprocessor
		EPreprocessorType get_type() const override { return P_SUMONE; }

	protected:
		SGMatrix<float64_t> apply_to_matrix(SGMatrix<float64_t> matrix) override;
};
}
#endif
