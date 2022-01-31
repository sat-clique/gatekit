# gatekit

`gatekit` is a collection of data structures and algorithms related to gate
constraints in propositional logic, in particular to CNF-encoded Boolean
Satisfiability problems. `gatekit` is designed to interface nicely with SAT
solver data structures, but also aims to be a library for rapidly experimenting
with gate structures recovered from SAT problems.

The basic gate recognition algorithm is based on the one implemented in
[cnftools](https://github.com/sat-clique/cnftools), but has been optimized
and extended.

## Relevant Literature

* Iser M., Manthey N., Sinz C. (2015): Recognition of Nested Gates in CNF Formulas.
  In: Heule M., Weaver S. (eds) Theory and Applications of Satisfiability Testing -- SAT 2015. SAT 2015. Lecture Notes in Computer Science, vol 9340. Springer, Cham.
  https://doi.org/10.1007/978-3-319-24318-4_19

## Relevant Software

* [cnfkit](https://github.com/sat-clique/cnfkit)
* [cnftools](https://github.com/sat-clique/cnftools)
* [gbd](https://github.com/udopia/gbd)

