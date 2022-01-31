/**
 * \file
 *
 * \brief Public functions for the gate structure scanner
 */

#pragma once

#include <gatekit/detail/scanner_structure.h>
#include <gatekit/gate.h>

namespace gatekit {

template <typename ClauseHandle, typename ClauseHandleIter>
auto scan_gates(ClauseHandleIter begin, ClauseHandleIter end) -> gate_structure<ClauseHandle>
{
  return detail::scan_gates_impl<ClauseHandle, ClauseHandleIter>(begin, end);
}

}
