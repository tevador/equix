# Equi-X

Equi-X is a CPU-friendly client puzzle with fast verification. It is based on Equihash(60,3) with two major changes:

1. Blake2b hash function is replaced with [HashX](https://github.com/tevador/hashx).
2. XOR is replaced with modular addition.

An Equi-X solution for nonce `X` is a set of eight 16-bit indices <code>i<sub>0</sub>, ..., i<sub>7</sub></code> such that:

<code>H<sub>X</sub>(i<sub>0</sub>) + H<sub>X</sub>(i<sub>1</sub>) + H<sub>X</sub>(i<sub>2</sub>) + H<sub>X</sub>(i<sub>3</sub>) + H<sub>X</sub>(i<sub>4</sub>) + H<sub>X</sub>(i<sub>5</sub>) + H<sub>X</sub>(i<sub>6</sub>) + H<sub>X</sub>(i<sub>7</sub>) = 0 (mod 2<sup>60</sup>)</code>

where <code>H<sub>X</sub></code> is a HashX function generated for nonce `X`. Equi-X is therefore a variant of the [subset sum problem](https://en.wikipedia.org/wiki/Subset_sum_problem). Each nonce value provides 2 solutions on average.

Equi-X also has additional requirements that prove that the solution was found using the Wagner's algorithm. See the [Equihash paper](https://eprint.iacr.org/2015/946.pdf) for details.

### Example solution

```
H(0x6592) = 0x348a96fd685dcba \
                              (+) = 0x357120e8ffb8000 \
H(0x76b7) = 0x00e689eb975a346 /                        |
                                                      (+) = 0x102552500000000 \
H(0x74a6) = 0xacccc4ad2d06bcd \                        |                       |
                              (+) = 0xdab431670048000 /                        |
H(0xe259) = 0x2de76cb9d341433 /                                                |
                                                                              (+) = 0
H(0x6c31) = 0xcfa5375c0a7f5d7 \                                                |
                              (+) = 0xa73d9777f110000 \                        |
H(0x8803) = 0xd798601be690a29 /                        |                       |
                                                      (+) = 0xefdaadb00000000 /
H(0x80c2) = 0xcabd8974bbee8d5 \                        |
                              (+) = 0x489d16380ef0000 /
H(0xa1db) = 0x7ddf8cc3530172b /
```

## Performance

The following table compares Equi-X with the most common variants of Equihash:

|Algorithm |n  |k  |memory|solution size|verification<sup>1</sup>|CPU perf.<sup>2</sup>|GPU perf.<sup>3</sup>|
|----------|---|---|------|-------------|------------|----------|---------|
|**Equi-X**|60 |3  |1.9 MB|16 bytes     |50 μs       |2000 Sol/s|    -    |
|ZCash     |200|9  |144 MB|1344 bytes   |150 μs      |30 Sol/s  |450 Sol/s|
|BTG       |144|5  |2.5 GB|100 bytes    |10 μs       |1 Sol/s   |65 Sol/s |

1. Using a single core of AMD Ryzen 1700 (approximate).
2. Using 8 cores of AMD Ryzen 1700 (approximate).
3. Using NVIDIA GTX 1080 (approximate).

## Build

```
git clone --recursive https://github.com/tevador/equix.git
cd equix
mkdir build
cd build
cmake ..
make
```
```
./equix-bench --nonces 1000
```