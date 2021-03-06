#pragma once

#include <gatekit/gate.h>

#include <memory>
#include <vector>

namespace gatekit {
using Clause = std::vector<int>;
using ClauseHandle = std::shared_ptr<Clause>;
using ClauseList = std::vector<Clause>;

enum class encoding { full, opt };

auto and_gate(std::vector<int> const& inputs, int output) -> gate<ClauseHandle>;
auto or_gate(std::vector<int> const& inputs, int output) -> gate<ClauseHandle>;
auto xor_gate(int lhs, int rhs, int output) -> gate<ClauseHandle>;

auto monotonic(gate<ClauseHandle>&& gate, encoding encoding = encoding::opt)
    -> ::gatekit::gate<ClauseHandle>;
}
