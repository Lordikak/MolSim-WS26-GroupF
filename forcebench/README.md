# forcebench

Benchmarks for the gravity force calculation used for the worksheet 1 presentation

## Run

From repo root:

```bash
forcebench/run.sh
```

Builds and runs every variant with g++ and clang++ each using `-O3` and `-O3 -march=native` and writes results to 
`forcebench/results.csv`

## Description

- Uses N=4000 particles at random positions, velocities and masses
- 1 Warmup run then 6 measured runs
- Measures median time for one force calculation divided by N^2

## Reference Results

`results_ref_jakobpc.csv` are the results used in the presentation measured on:

|         |                                             |
|---------|---------------------------------------------|
| CPU     | 12th Gen Intel(R) Core(TM) i7-12700K        |
| g++     | g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0 |
| clang++ | Ubuntu clang version 18.1.3 (1ubuntu1)      |
| OS      | 6.6.87.1-microsoft-standard-WSL2            |

`results_ref_CMK.csv` are the results used in the presentation measured on:

|         |                                             |
|---------|---------------------------------------------|
| CPU     | AMD Ryzen AI 7 350 w/ Radeon 860M           |
| g++     | g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0 |
| clang++ | Ubuntu clang version 18.1.3 (1ubuntu1)      |
| OS      | 6.17.9-76061709-generic                     |


`results_ref_sashaPC.csv` are the results used in the presentation measured on:

|         |                                             |
|---------|---------------------------------------------|
| CPU     | AMD Ryzen 7 6800H with Radeon Graphics      |
| g++     | g++ (GCC) 15.2.1 20260123 (Red Hat 15.2.1-7)|
| clang++ | clang version 20.1.8 (Fedora 20.1.8-4.fc42) |
| OS      | 6.19.14-108.fc42.x86_64                     |

`results_ref_jakoblap.csv` are the results used in the presentation measured on:

|         |                                             |
|---------|---------------------------------------------|
| CPU     | Intel(R) Core(TM) 5 120U                    |
| g++     | g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0 |
| clang++ | Ubuntu clang version 18.1.3 (1ubuntu1)      |
| OS      | 7.0.0-34-generic                            |
