# MolSim GroupF

## Members
  - Sasha Marola
  - Campbell Krulewski
  - Jakob Schulz

## Build without VTK
```bash
mkdir build && cd build
cmake ..
make
```
## Options 
run instead with additional option `-DENABLE_VTK_OUTPUT=ON` to generate VTK files in `output/vtk`
```bash
cmake -S . -B build -DENABLE_VTK_OUTPUT=ON
```
or with option `-DENABLE_DOXYGEN=ON` to generate documentation
```bash
cmake -S . -B build -DENABLE_DOXYGEN=ON
```

## Run with standard arguments of deltaT and end time
```bash
./MolSim ../input/eingabe-sonne.txt
```
## Run with custom arguments of deltaT and end time
```bash
./MolSim ../input/eingabe-sonne.txt deltaT endTime
```

## Results
results are contained in the folder `output`
