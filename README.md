# gismo

A minimal group independent support (GIS) calculator. This tool builds on [Arjun](https://github.com/meelgroup/arjun), and is developed for the paper

> *Solving the Identifying Code Set Problem with Grouped Independent Support*, Anna L.D. Latour, Arunabha Sen, Kuldeep S. Meel, IJCAI 2023, paper #4051. [[paper pdf](https://www.ijcai.org/proceedings/2023/0219.pdf), [extended version](https://raw.githubusercontent.com/latower/latower.github.io/master/files/misc/LatEtAl23-extended_2023-08-23.pdf)]

## Related Resources

Please find a repository with our experimentation scripts, benchmarks, and results at [github.com/latower/identifying-codes](https://github.com/latower/identifying-codes/).

## How to Build
To build on Linux, you will need the following:
```
sudo apt-get install build-essential cmake
sudo apt-get install zlib1g-dev libboost-program-options-dev libboost-serialization-dev
```

Then, build CryptoMiniSat and gismo:
```
git clone https://github.com/msoos/cryptominisat
cd cryptominisat
mkdir build && cd build
cmake ..
make
sudo make install
sudo ldconfig

cd ../..
git clone https://github.com/meelgroup/gismo
cd gismo
mkdir build && cd build
cmake ..
make
sudo make install
sudo ldconfig
```

Developers, please pick your favourite build script (e.g, `build_static.sh`) and build as follows:
```
git clone https://github.com/meelgroup/gismo
cd gismo
mkdir build && cd build
ln -a ../scripts/* .
./build_static.sh
```

## How to Use

### Input format

The input for gismo is a CNF in DIMACS format, in which the variables of interest are indicated, and the way in which they are grouped. Considering the example in `example/example.gcnf`, the first part of the grouped DIMACS encoding looks like this:

```
p cnf 11 31
c ind 1 2 3 4 5 6 7 8 9 10 0
c grp 1 6 0
c grp 2 7 0
c grp 3 8 0
c grp 4 9 0
c grp 5 10 0
-5 -4 0
-5 -11 0
...
```
Here, we see that we have 11 variables, the first 10 of which are the variables of interest (and variable `11` is an auxiliary variable from the encoding of the cardinality constraint). The variables of interest are partitioned as follows: `groups := { {1,6}, {2,7}, {3,8}, {4,9}, {5,10} }`.

### Running gismo

```bash
user@machine: gismo/build$ ./gismo ../example/example.gcnf
```
The output should contain a line that reads `c ind 3 8 1 6 0`, indicating that variables `3`, `8`, `1`, and `6` are in the support of the grouped independent support. 


## Contributors
- Anna L.D. Latour ([github.com/latower](https://github.com/latower))
- Mate Soos ([github.com/msoos](https://github.com/msoos))
