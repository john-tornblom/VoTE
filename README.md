# VoTE (Verifier of Tree Ensembles) [![Build Status][buildbadge]][buildstats]
VoTE (Verifier of Tree Ensembles) is a toolsuite for analyzing input/output
mappings of decision trees and tree ensembles. VoTE consists of two distinct
type of components, VoTE Core and VoTE Property Checker. VoTE Core takes as
input a tree ensemble with corresponding input domain, and emits a sequence of
equivalence classes, i.e. sets of points in the input domain that yield the
same output. These equivalence classes are then processed by a VoTE Property
Checker that checks that all input/output mappings captured by an equivalence
class complies with some requirement.

For more information, see [related publications](#related-publications).

## Building
On Ubuntu-flavored operating systems, you can invoke the following commands to
install dependencies, generate makefiles, and compile the source code.
```console
john@localhost:VoTE$ sudo apt-get install autoconf libtool build-essential
john@localhost:VoTE$ sudo apt-get install python-cffi # optional python bindings
john@localhost:VoTE$ ./bootstrap.sh
john@localhost:VoTE$ ./configure
john@localhost:VoTE$ make
```

## Running Case Studies
VoTE includes example case studies that use scikit-learn and catboost to train
tree ensembles, and VoTE to verify requirements. To install scikit-learn and
catboost on Ubuntu-flavored operating systems, invoke the following command:
```console
john@localhost:VoTE$ sudo apt-get install python-sklearn
john@localhost:VoTE$ python -m pip install catboost --user
```

To run a collision detection verification case study, invoke the following
command:
```console
john@localhost:VoTE$ ./support/collision_detection.sh
```

VoTE also includes Python bindings for easy prototyping and testing of
domain-specific property checkers. See [example.py][example] for a simple
example. To invoke a test suite for the python bindings, run the following
command:
```console
john@localhost:VoTE$ python bindings/python/setup.py test
```

## Reporting Bugs
If you encounter problems with VoTE, please [file a github issue][issues]. If
you plan on sending pull requests which affect more than a few lines of code,
please file an issue before you start to work on you changes. This will allow us
to discuss the solution properly before you commit time and effort.

## Related Publications
- J. Törnblom and S. Nadjm-Tehrani, **An Abstraction-Refinement Approach to
  Formal Verification of Tree Ensembles**, *In proceedings of 2nd International
  workshop on Artificial Intelligence Safety Engineering, held in conjunction
  with SAFECOMP*, Springer, 2019. Available as [preprint][paper:absref].

- J. Törnblom and S. Nadjm-Tehrani, **Formal Verification of Input-Output
  Mappings of Tree Ensembles**. *Currently in submission*. Available as
  [preprint][paper:vote].

- J. Törnblom and S. Nadjm-Tehrani, **Formal Verification of Random Forests in
  Safety-Critical Applications**, *in International Workshop on Formal
  Techniques for Safety-Critical Systems (FTSCS)*, Springer, 2019. DOI:
  [10.1007/978-3-030-12988-0_4](https://doi.org/10.1007/978-3-030-12988-0_4).
  Available as [preprint][paper:vorf].

## License
[VoTE Core](lib) is licensed under the LGPLv3+, and [programs](src) linking to
VoTE Core are licensed under the GPLv3+, see COPYING and COPYING.LESSER for more
information. Files in the [ext folder](ext) are copyrighted by external
entities. In particular, VoTE Core use [parson][parsonurl] to parse
JSON-persisted models. Parson is licened under the [MIT license][mitlic].


[buildbadge]: https://travis-ci.org/john-tornblom/VoTE.svg?branch=master
[buildstats]: https://travis-ci.org/john-tornblom/VoTE
[example]: bindings/python/example.py
[issues]: https://github.com/john-tornblom/vote/issues/new
[paper:absref]: https://www.ida.liu.se/labs/rtslab/publications/2019/John_WAISE.pdf
[paper:vote]: https://arxiv.org/pdf/1905.04194
[paper:vorf]: https://www.ida.liu.se/labs/rtslab/publications/2018/John_FTSCS.pdf
[parsonurl]: http://kgabis.github.io/parson
[mitlic]: https://opensource.org/licenses/mit-license.php
