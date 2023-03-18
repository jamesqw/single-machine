#ifndef CONFLUO_AGGREGATE_AGGREGATE_MANAGER_H_
#define CONFLUO_AGGREGATE_AGGREGATE_MANAGER_H_

#include "aggregate_ops.h"

namespace confluo {

/**
 * Manager of the aggregates
 */
class aggregate_manager {
 public:
  /**
   * Registers an aggregate to the manager
   *
   * @param agg The aggregate to add to the manager
   *
   * @return The size of the manager
   */
  static size_t register_aggregate(const aggregator &agg);

  /**
   * Gets an aggregate from the name
   *
   * @param name The name of the aggregate to get
   *
   * @return The type of the aggregate
   */
  static aggregate_type get_aggregator_id(const std::string &name);

  /**
   * Gets the aggregator based on name
   *
   * @param name The name of the aggregator
   *
   * @return The aggregator matching the name
   */
  static aggregator get_aggregator(const std::string &name);

  /**
   * Gets aggregator based on id
   *
   * @param id The identifier of the aggregator
   *
   * @return The matching aggregator
   */
  static aggregator get_aggregator(size_t id);

  /**
   * Gets whether the id is a valid aggregator
   *
   * @param id The identifier to test
   *
   * @return True if id is a valid aggregator, false otherwise
   */
  static bool is_valid_id(size_t id);
};

}

#endif /* CONFLUO_AGGREGATE_AGGREGATE_MANAGER_H_ */
