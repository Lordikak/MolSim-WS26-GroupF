# Trajectory format

## File ending MSB (MolSim binary)

Two files one ending .msb one ending .msb.idx as an index file

## Format

Everything is little endian and everything is append only

### Header:

| Offset | Type    | Value                       | Descr          |
|--------|---------|-----------------------------|----------------|
| 0      | byte[4] | "msbf"                      | Magic          |
| 4      | u32     | 100                         | Format version |
| 8      | u64     | [Bitmask](#Bitmask)         | Bitmask        |
| 16     | f64     |                             | Start time     |
| 24     | f64     |                             | delta time     |
| 32     | f64[6]  | [Domain size](#Domain-size) | size of domain |

#### Domain size

3 f64 for minx miny minz then 3 for maxx maxy maxz

### Bitmask

`TODO`

For now position and velocity is in there using `field type` = f64

### Data

The header is followed by an Array of [Frame](#Frame)

### Frame

| Offset | Type              | Descr              |
|--------|-------------------|--------------------|
| 0      | u32               | Step               |
| 4      | u32               | particle count `N` |
| 8      | [Blocks](#Blocks) | Blocks             |

### Blocks

for every field one block, every block is `N` values with each particles entry contiguous: (`xyz xyz xyz ...`)

If the data is not in there they are skipped

| Block    | Type                 | Layout          |
|----------|----------------------|-----------------|
| position | N × 3 × `field type` | xyz xyz xyz ... |
| velocity | N × 3 × `field type` | xyz xyz xyz ... |
...

## IDX

### Header:

Flat Array of [Index entry](#Index-entry)

### Index Entry

| Offset | Type | Descr                |
|--------|------|----------------------|
| 0      | u64  | Offset in data file  |
| 8      | u32  | Simulation iteration |
| 12     | u32  | particle count `N`   |

Steps are strictly increasing to allow bin search
