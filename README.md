# MolSim GroupF

## Members
  - Sasha Marola
  - Campbell Krulewski
  - Jakob Schulz

## Build
```bash
mkdir build && cd build
cmake ..
make
```

## Run
```bash
./MolSim ../input/eingabe-sonne.txt
```

## Results
results are contained in the folder `output`

## Options 
run with additional option `-DENABLE_VTK_OUTPUT=ON` to generate VTK files in `output/vtk`
```bash
cmake -S . -B build -DENABLE_VTK_OUTPUT=ON
```