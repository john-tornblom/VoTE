# VoTE (Verifier of Tree Ensembles) [![Build Status](https://travis-ci.org/john-tornblom/VoTE.svg?branch=master)](https://travis-ci.org/john-tornblom/VoTE) 
VoTE (Verifier of Tree Ensembles) is a toolsuite for analyzing input/output
mappings of decision trees and tree ensembles. VoTE consists of two distinct
type of components, VoTE Core and VoTE Property Checker. VoTE Core takes as
input a tree ensemble with corresponding input domain, and emits a sequence of
equivalence classes, i.e. sets of points in the input domain that yield the
same output. These equivalence classes are then processed by a VoTE Property
Checker that checks that all input/output mappings captured by an equivalence
class complies with some requirement.

For more information, see
[our paper](http://arxiv.org/abs/1905.04194).

## Building
Autotools is used to generate makefiles, and VoTE Core depends on libjson-c to
parse decision trees and tree ensembles. On Ubuntu-flavored operating systems,
you can invoke the following commands to install dependencies, generate
makefiles, and compile the source code.
```console
john@localhost:VoTE$ sudo apt-get install autoconf libtool pkg-config build-essential libjson-c-dev python-cffi python-dev
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
domain-specific property checkers. See [example.py](bindings/python/example.py)
for a simple example. To invoke a test suite for the python bindings, run the
following command:
```console
john@localhost:VoTE$ python bindings/python/setup.py test
```

## Reporting Bugs
If you encounter problems with VoTE, please 
[file a github issue](https://github.com/john-tornblom/vote/issues/new). 
If you plan on sending pull requests which affect more than a few lines of code, 
please file an issue before you start to work on you changes. This will allow us
to discuss the solution properly before you commit time and effort.

## License
[VoTE Core](lib) is licensed under the LGPLv3+, and [programs](src) linking to
VoTE Core are licensed under the GPLv3+. see COPYING and COPYING.LESSER for more
information. Files in the [ext folder](ext) are copyrighted by external entities.
In particular, VoTE Core use [parson](http://kgabis.github.io/parson) to parse
JSON-persisted models. Parson is licened under the
[MIT license](https://opensource.org/licenses/mit-license.php).
