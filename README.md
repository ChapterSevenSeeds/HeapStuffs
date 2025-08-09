# Heap Stuffs

### A study on different kinds of heaps.

## Implemented Allocation Strategies

- First fit with mandatory chunk split if there is enough room for a chunk header.
- Best fit with the same split as the above.
- First fit with the same split as the above plus 25% of the requested bytes.
- Best fit with the same split as the above.
- Power of two fit with mandatory chunk split if there is enough room for a chunk header - blocks are only allowed to be
  sized in powers of two (8 bytes for 1-8, 16 bytes for 9-16, 32 bytes for 17-32, etc).

## Implemented Free Strategies

- Simple free (the chunk is marked as free).
- Coalescing free (chunk is merged with adjacent free chunks, if any).