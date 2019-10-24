# Harvard CS s165 - Big Data Systems #
---
*This repository contains the code of a workload generator for a key-value database.
It follows the DSL specified for A1 of CS s165.*

## Workload and Data Generator ##
---

### Dependencies ###
You need the GNU scientific library in order to use the generator (https://www.gnu.org/software/gsl/).

* Ubuntu Linux: ```sudo apt-get install libgsl-dev```
* Fedora Linux: ```dnf install gsl-devel```
* Mac OS X: ```brew install gsl```
* Cygwin: install the *gsl, gsl-devel* packages

### Building ###
```
cd generator;
make clean; make;
```

or simply...
```
cc generator.c -o generator -lgsl -lgslcblas
```

### Running ###
You can now run the following to see all available options:
```
./generator --help
```


### Examples ###

**Workload 1:** Insert 100000 keys, perform 1000 gets and 10 range queries.

```
./generator --puts 100000 --gets 1000  > workload.txt
```

**Workload 2:** Same as above but store the data in external (.dat) binary files.
```
./generator --puts 100000 --gets 1000  > workload.txt
```

**Workload 3:** Perform 100000 puts and issue 100 range queries (drawn from a uniform distribution).
```
./generator --puts 100000 --ranges 100 --uniform-ranges > workload.txt
```

**Note: You can always set the random number generator seed using --seed XXXX**


## Evaluating a Workload ##
---
You can execute a workload and see some basic statistics about it, using the ```evaluate.py``` python script.
This script runs the workload on an in-memory python dict-backed implementation of a key-value database that follows the DSL.

### Dependencies ###
You need to install the [blist](https://pypi.python.org/pypi/blist/?) library.

Most platforms: ```pip install blist```

*Note: In Fedora Linux, you might need to install it using: ```dnf install python-blist```.*

### Running ###

Run as follows:
```
python evaluate.py workload.txt
```

**Note: For extra options etc, please look inside the script.**