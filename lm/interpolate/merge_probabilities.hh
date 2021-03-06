#ifndef KENLM_INTERPOLATE_MERGE_PROBABILITIES_H
#define KENLM_INTERPOLATE_MERGE_PROBABILITIES_H

#include "util/fixed_array.hh"
#include "util/stream/multi_stream.hh"
#include "lm/interpolate/interpolate_info.hh"
#include "lm/common/ngram.hh"

namespace lm {
namespace interpolate {

/**
 * The first pass for the offline log-linear interpolation algorithm. This
 * reads K **suffix-ordered** streams for each model, for each order, of
 * ngram records (ngram-id, prob, backoff). It further assumes that the
 * ngram-ids have been unified over all of the stream inputs.
 *
 * Its output is records of (ngram-id, prob-prod, backoff-level,
 * backoff-level, ...) where the backoff-levels (of which there are K) are
 * the context length (0 for unigrams) that the corresponding model had to
 * back off to in order to obtain a probability for that ngram-id. Each of
 * these streams is terminated with a record whose ngram-id is all
 * maximum-integers for simplicity in implementation here.
 *
 * @param models An array of length N (max_i N_i) containing at
 *  the ChainPositions for the streams for order (i + 1).
 * @param output_chains The output chains for each order (of length K)
 */
void MergeProbabilities(
    const InterpolateInfo &info,
    util::FixedArray<util::stream::ChainPositions> &models_by_order,
    util::stream::Chains &output_chains);
}

/**
 * This class represents the output payload for this pass, which consists
 * of an ngram-id, a probability, and then a vector of orders from which
 * each of the component models backed off to for this ngram, encoded
 * using the BoundedSequenceEncoding class.
 */
class PartialProbGamma : public lm::NGramHeader {
public:
  PartialProbGamma(std::size_t order, std::size_t backoff_bytes)
      : lm::NGramHeader(NULL, order), backoff_bytes_(backoff_bytes) {
    // nothing
  }

  std::size_t TotalSize() const {
    return sizeof(WordIndex) * Order() + sizeof(float) + backoff_bytes_;
  }

  float &Prob() {
    return *reinterpret_cast<float *>(end());
  }

  float Prob() const {
    return *reinterpret_cast<const float *>(end());
  }

  const uint8_t *FromBegin() const {
    return reinterpret_cast<const uint8_t *>(end()) + sizeof(float);
  }

  uint8_t *FromBegin() {
    return reinterpret_cast<uint8_t *>(end()) + sizeof(float);
  }

private:
  std::size_t backoff_bytes_;
};
}
#endif
