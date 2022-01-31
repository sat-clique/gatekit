# gatekit

`gatekit` is a collection of data structures and algorithms related to gate
constraints in propositional logic, in particular to CNF-encoded Boolean
Satisfiability problems. `gatekit` is designed to interface nicely with SAT
solver data structures, but also aims to be a library for rapidly experimenting
with gate structures recovered from SAT problems.

The basic gate recognition algorithm is based on the one implemented in
[cnftools](https://github.com/sat-clique/cnftools), but has been optimized
and extended.

