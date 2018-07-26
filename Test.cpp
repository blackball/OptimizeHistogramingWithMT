#include "Timer.h"
#include "ParaFor.h"
#include <string.h> // memset 
#include <vector>
#include <stdio.h>
#include <stdlib.h> // rand 
#include <time.h>
#include <stdint.h>
#include <algorithm>

using T = int32_t; 
#define LUT_SIZE 80000

typedef std::vector<T> V;
typedef std::vector<std::vector<T> > VV;
typedef std::vector<T> LUT;

static void GenerateData(V &indices, size_t n)
{
    indices.resize(n);
    for (size_t i = 0; i < n; ++i)
    {
        indices[i] = rand() % LUT_SIZE;
    }
}

static void GenerateVV(VV &vv, int n = 500)
{
    srand((unsigned int)time(NULL));    
    vv.resize(n);
    for (int i = 0; i < n; ++i)
    {
        GenerateData(vv[i], rand() % 1000000);
    }    
}

static void Naive(const VV &vv, LUT &lut)
{
    for (size_t i = 0; i < vv.size(); ++i)
    {
        for (size_t j = 0; j < vv[i].size(); ++j)
        {
            ++lut[vv[i][j]];
        }
    }
}

template<int NumThread>
struct MTOP
{
    T luts[NumThread][LUT_SIZE];    
    const VV &vv;
    V &v; 
    MTOP(const VV &ovv, V &ov) : vv(ovv), v(ov)
    {
        memset(luts, 0, sizeof(luts));
    }

    int GetNumItems() const
    {
        return vv.size();
    }
    
    void operator()(int index, int tid)
    {
        const V &cv = vv[index];
        const int size = cv.size();
        for (int i = 0; i < size; ++i) {
            ++luts[tid][cv[i]];
        }
    }

    void Post()
    {
        v.resize(LUT_SIZE);
        for (int i = 0; i < LUT_SIZE; ++i)
        {
            for (int j = 0; j < NumThread; ++j)
            {
                v[i] += luts[j][i];
            }
        }
    }
};

static bool IsSame(const LUT &a, const LUT &b)
{
    if (a.size() != b.size()) return false;
    const int size = a.size();
    for (int i = 0; i < size; ++i)
    {
        if (a[i] != b[i]) return false; 
    }
    return true; 
}

int main(int argc, char *argv[])
{
    VV vv;
    GenerateVV(vv);
    LUT lut1(LUT_SIZE, 0); 
    LUT lut2(LUT_SIZE, 0); 

    Timer timer;

    timer.Start();
    Naive(vv, lut1);
    timer.Stop();
    
    printf("Duration in ms: %lf\n", timer.Duration());
    
    timer.Start();    
    ParaFor<MTOP<4>, 4> pf(vv, lut2);
    pf.Join();    
    timer.Stop();
    
    printf("Duration in ms: %lf\n", timer.Duration());

    if (IsSame(lut1, lut2))
    {
        puts("Results are identical!");
    }
    else
    {
        puts("Results are different!");
    }
    
    return 0;
}

