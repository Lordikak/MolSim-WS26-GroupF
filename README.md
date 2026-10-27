# MolSim GroupF

## Members
  - Sasha Marola
  - Campbell Krulewski
  - Jakob Schulz

## Build without VTK
```bash
cmake -S . -B build
cmake --build build -j
```
## Options 

| Option                 | Effect                             |
|------------------------|------------------------------------|
| -DENABLE_VTK_OUTPUT=ON | generate VTK files in `output/vtk` |
| -DENABLE_DOXYGEN=ON    | generate documentation             |

run instead with additional option `-DENABLE_VTK_OUTPUT=ON` to generate VTK files in `output/vtk`
```bash
cmake -S . -B build -DENABLE_VTK_OUTPUT=ON
cmake --build build -j
```
or with option `-DENABLE_DOXYGEN=ON` to generate documentation
```bash
cmake -S . -B build -DENABLE_DOXYGEN=ON
cmake --build build -j
```

## Run with standard arguments of deltaT and end time
```bash
./build/MolSim input/eingabe-sonne.txt
```
## Run with custom arguments of deltaT and end time
```bash
./build/MolSim input/eingabe-sonne.txt deltaT endTime
```

## Results
results are contained in the folder `build/output`
