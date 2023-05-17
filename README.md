# Arjun-GIS

A minimal group independent support (GIS) calculator. Using the original idea of MIS and B+E, with quick fused queries, using inverted and mass queries for fast heuristic search. This tool builds on [Arjun](https://github.com/meelgroup/arjun), and is written for the paper

> *Solving the Identifying Code Set Problem with Grouped Independent Support*, Anna L.D. Latour, Arunabha Sen, Kuldeep S. Meel, IJCAI 2023.

Note that the simplification part of Arjun contains code from SharpSAT-td by Tuukka Korhonen and Matti Jarvisalo, see [this PDF](https://raw.githubusercontent.com/Laakeri/sharpsat-td/main/description.pdf) and [this code](https://github.com/Laakeri/sharpsat-td) for details. Note that treewidth-decomposition is _not_ part of Arjun.

## How to Build
To build on Linux, you will need the following:
```
sudo apt-get install build-essential cmake
sudo apt-get install zlib1g-dev libboost-program-options-dev libboost-serialization-dev
```

Then, build CryptoMiniSat, Louvain-Community, and Arjun:
```
git clone https://github.com/msoos/cryptominisat
cd cryptominisat
mkdir build && cd build
cmake ..
make
sudo make install
sudo ldconfig

cd ../..
git clone https://github.com/meelgroup/arjun
cd arjun
mkdir build && cd build
cmake ..
make
sudo make install
sudo ldconfig
```

## How to Use

(TODO)
