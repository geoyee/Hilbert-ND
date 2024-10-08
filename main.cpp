//+++++++++++++++++++++++++++ PUBLIC-DOMAIN SOFTWARE ++++++++++++++++++++++++++
// Functions: TransposetoAxes AxestoTranspose
// Purpose: Transform in-place between Hilbert transpose and geometrical axes
// Example: b=5 bits for each of n=3 coordinates.
// 15-bit Hilbert integer = A B C D E F G H I J K L M N O is stored
// as its Transpose
//        X[0] = A D G J M              X[2]|
//        X[1] = B E H K N  <------->       | /X[1]
//        X[2] = C F I L O             axes |/
//               high  low                  0------ X[0]
// Axes are stored conventially as b-bit integers.
// Author: John Skilling 20 Apr 2001 to 11 Oct 2003
//-----------------------------------------------------------------------------

#include <cstdio>
#include <cstdlib>

typedef unsigned int coord_t; // char,short,int for up to 8,16,32 bits per word

void TransposetoAxes(coord_t *X, int b, int n) // Position, #bits, dimension
{
    coord_t N = 2 << (b - 1), P, Q, t;
    // Gray decode by H ^ (H/2)
    t = X[n - 1] >> 1;
    // Corrected error in Skilling's paper on the following line. The appendix had i >= 0 leading to negative array
    // index.
    for (int i = n - 1; i > 0; i--)
    {
        X[i] ^= X[i - 1];
    }
    X[0] ^= t;
    // Undo excess work
    for (Q = 2; Q != N; Q <<= 1)
    {
        P = Q - 1;
        for (int i = n - 1; i >= 0; i--)
        {
            if (X[i] & Q) // Invert
            {
                X[0] ^= P;
            }
            else
            { // Exchange
                t = (X[0] ^ X[i]) & P;
                X[0] ^= t;
                X[i] ^= t;
            }
        }
    }
}

void AxestoTranspose(coord_t *X, int b, int n) // Position, #bits, dimension
{
    coord_t M = 1 << (b - 1), P, Q, t;
    // Inverse undo
    for (Q = M; Q > 1; Q >>= 1)
    {
        P = Q - 1;
        for (int i = 0; i < n; i++)
        {
            if (X[i] & Q) // Invert
            {
                X[0] ^= P;
            }
            else
            { // Exchange
                t = (X[0] ^ X[i]) & P;
                X[0] ^= t;
                X[i] ^= t;
            }
        }
    }
    // Gray encode
    for (int i = 1; i < n; i++)
    {
        X[i] ^= X[i - 1];
    }
    t = 0;
    for (Q = M; Q > 1; Q >>= 1)
    {
        if (X[n - 1] & Q)
        {
            t ^= Q - 1;
        }
    }
    for (int i = 0; i < n; i++)
    {
        X[i] ^= t;
    }
}

int interleaveBits(coord_t *X, int b, int n) // Position, #bits, dimension
{
    unsigned int *coden = (unsigned int *)malloc(n * sizeof(unsigned int));
    for (int i = 0; i < n; ++i)
    {
        coden[i] = 0;
    }
    const int nbits2 = 2 * b;
    for (int i = 0, andbit = 1; i < nbits2; i += 2, andbit <<= 1)
    {
        for (int j = 0; j < n; ++j)
        {
            coden[j] |= (unsigned int)(X[j] & andbit) << i;
        }
    }
    int res = (coden[0] << (n - 1));
    for (int j = 1; j < n; ++j)
    {
        res |= (coden[j] << (n - j - 1));
    }
    free(coden);
    return res;
}

// From https://github.com/Forceflow/libmorton/blob/main/include/libmorton/morton3D.h
void uninterleaveBits(coord_t *X, int b, int n, unsigned int code) // Position, #bits, dimension
{
    for (int i = 0; i < n; ++i)
    {
        X[i] = 0;
    }
    for (int i = 0; i <= b; ++i)
    {
        unsigned int selector = 1;
        unsigned int shift_selector = 3 * i;
        unsigned int shiftback = 2 * i;
        for (int j = 0; j < n; ++j)
        {
            X[n - j - 1] |= (code & (selector << (shift_selector + j))) >> (shiftback + j);
        }
    }
}

int main()
{
    coord_t X[3] = {5, 10, 20}; // Any position in 32x32x32 cube
    printf("Input coords = %d,%d,%d\n", X[0], X[1], X[2]);

    AxestoTranspose(X, 5, 3); // Hilbert transpose for 5 bits and 3 dimensions
    printf("Hilbert coords = %d,%d,%d\n", X[0], X[1], X[2]);

    unsigned int code = interleaveBits(X, 5, 3);

    printf("Hilbert integer = %d = %d%d%d %d%d%d %d%d%d %d%d%d %d%d%d = 7865 check\n",
           code,
           X[0] >> 4 & 1,
           X[1] >> 4 & 1,
           X[2] >> 4 & 1,
           X[0] >> 3 & 1,
           X[1] >> 3 & 1,
           X[2] >> 3 & 1,
           X[0] >> 2 & 1,
           X[1] >> 2 & 1,
           X[2] >> 2 & 1,
           X[0] >> 1 & 1,
           X[1] >> 1 & 1,
           X[2] >> 1 & 1,
           X[0] >> 0 & 1,
           X[1] >> 0 & 1,
           X[2] >> 0 & 1);

    uninterleaveBits(X, 5, 3, code);
    printf("Reconstructed Hilbert coords = %d,%d,%d\n", X[0], X[1], X[2]);

    TransposetoAxes(X, 5, 3);
    printf("Orig coords = %d,%d,%d\n", X[0], X[1], X[2]);

    return 0;
}