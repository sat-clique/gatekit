/**
 * \file
 *
 * \brief Public functions for the gate structure scanner
 */

#pragma once

#include <gatekit/detail/scanner_structure.h>
#include <gatekit/gate.h>

namespace gatekit {

/**
 * Scans the given clauses for gate constraints.
 *
 * Usage example: while starting a SAT solver, scan the problem instance for
 *   gates before running the simplifier, then use the gate structure's
 *   definitions to simplify the problem instance.
 *
 * \tparam ClauseHandle   The clause handle type, for instance
 *                        `std::vector<int> const*` for simple applications.
 *                        See traits.h for more information.
 *
 * \tparam ClauseHandleIter Iterator over ClauseHandle objects.
 */
template <typename ClauseHandle, typename ClauseHandleIter>
auto scan_gates(ClauseHandleIter begin, ClauseHandleIter end) -> gate_structure<ClauseHandle>
{
  return detail::scan_gates_impl<ClauseHandle, ClauseHandleIter>(begin, end);
}

}
