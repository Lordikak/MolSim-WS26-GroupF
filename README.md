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
| -DENABLE_VTK_OUTPUT=ON | generate VTK files in `output/VTK` |
| -DENABLE_DOXYGEN=OFF   | dont create the doc_doxygen target |

run instead with additional option `-DENABLE_VTK_OUTPUT=ON` to generate VTK files in `output/VTK`
```bash
cmake -S . -B build -DENABLE_VTK_OUTPUT=ON
cmake --build build -j
```

## Documentation

Generate the Doxygen documentation into `build/html` (open `build/html/index.html`)
```bash
cmake -S . -B build
cmake --build build --target doc_doxygen
```

## Run with standard arguments of deltaT and end time
```bash
./build/MolSim input/eingabe-sonne.txt
```
## Run with custom arguments of deltaT and end time
```bash
./build/MolSim input/eingabe-sonne.txt --dt <deltaT> --end <endTime>
```

## Results
results are contained in the folder `output`
