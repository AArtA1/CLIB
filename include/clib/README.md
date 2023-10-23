# FLEX-FLOAT

```c
struct flexfloat {
    uint64_t E; // EXPONENT WIDTH
    uint64_t M; // MANTISSA WIDTH
    int64_t B;  // BIAS
    
    uint8_t s;  // [0, 1]
    uint64_t e; // [0, 2^E-1]
    uint64_t m; // [0, 2^M-1]
};
```
