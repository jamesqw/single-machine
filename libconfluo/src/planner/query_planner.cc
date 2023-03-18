#include "planner/query_planner.h"

namespace confluo {
namespace planner {

query_planner::query_planner(const data_log *dlog, const index_log *idx_list, const schema_t *schema)
    : dlog_(dlog),
      idx_list_(idx_list),
      schema_(schema) {
}

query_plan query_planner::plan(const parser::compiled_expression &expr) const {
  query_plan qp(dlog_, schema_, expr);
  for (const parser::compiled_minterm &m : expr) {
    std::shared_ptr<query_op> op = optimize_minterm(m);
    switch (op->op_type()) {
      case query_op_type::D_NO_OP: {
        break;
      }
      case query_op_type::D_NO_VALID_INDEX_OP: {
        qp.clear();
        qp.push_back(std::make_shared<full_scan_op>());
        return qp;
      }
      case query_op_type::D_INDEX_OP: {
        qp.push_back(op);
        break;
      }
      default: {
        throw illegal_state_exception("Minterm generated invalid query_op");
      }
    }
  }
  return qp;
}

query_planner::key_range query_planner::merge_range(const query_planner::key_range &r1,
                                                    const query_planner::key_range &r2) const {
  return std::make_pair(std::max(r1.first, r2.first), std::min(r1.second, r2.second));
}

bool query_planner::add_range(query_planner::key_range_map &ranges,
                              uint32_t id,
                              const query_planner::key_range &r) const {
  key_range_map::iterator it;
  key_range r_m;
  if ((it = ranges.find(id)) != ranges.end()) {  // Multiple key ranges
    r_m = merge_range(it->second, r);
  } else {  // Single key-range
    r_m = r;
  }

  if (r_m.first <= r_m.second) {  // Valid key range
    ranges[id] = r_m;
    return true;
  }
  return false;  // Invalid key-range
}

std::shared_ptr<query_op> query_planner::optimize_minterm(const parser::compiled_minterm &m) const {
  // Get valid, condensed key-ranges for indexed attributes
  key_range_map m_key_ranges;
  for (const auto &p : m) {
    uint32_t idx = p.field_idx();
    const auto &col = (*schema_)[idx];
    if (col.is_indexed() && p.op() != reational_op_id::NEQ) {
      double bucket_size = col.index_bucket_size();
      key_range r;
      switch (p.op()) {
        case reational_op_id::EQ: {
          r = std::make_pair(p.value().to_key(bucket_size), p.value().to_key(bucket_size));
          break;
        }
        case reational_op_id::GE: {
          r = std::make_pair(p.value().to_key(bucket_size), col.max().to_key(bucket_size));
          break;
        }
        case reational_op_id::LE: {
          r = std::make_pair(col.min().to_key(bucket_size), p.value().to_key(bucket_size));
          break;
        }
        case reational_op_id::GT: {
          r = std::make_pair(++p.value().to_key(bucket_size), col.max().to_key(bucket_size));
          break;
        }
        case reational_op_id::LT: {
          r = std::make_pair(col.min().to_key(bucket_size), --p.value().to_key(bucket_size));
          break;
        }
        default: {
          throw invalid_operation_exception("Invalid operator in predicate");
        }
      }

      if (!add_range(m_key_ranges, col.index_id(), r)) {
        return std::make_shared<no_op>();
      }
    }
  }

  if (m_key_ranges.empty()) {  // None of the fields are indexed
    return std::make_shared<no_valid_index_op>();
  }

  // If we've reached here, we only have non-zero valid, indexed key-ranges.
  // Now we only need to return the minimum cost index lookup
  uint32_t min_id;
  size_t min_cost = UINT64_MAX;
  for (const auto &m_entry : m_key_ranges) {
    size_t cost;
    // TODO: Make the cost function pluggable
    if ((cost = idx_list_->at(m_entry.first)->approx_count(
        m_entry.second.first, m_entry.second.second)) < min_cost) {
      min_cost = cost;
      min_id = m_entry.first;
    }
  }

  return std::make_shared<index_op>(idx_list_->at(min_id), m_key_ranges[min_id]);
}

}
}