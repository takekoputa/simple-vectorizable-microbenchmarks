#include<stdint.h>
#include<vector>
#include<cassert>

typedef uint64_t TElement;
typedef uint64_t TIndex;

void restricted_gather_load(TElement* __restrict__ dst, TElement* __restrict__ src, TIndex* __restrict__ indices, const TIndex& n_indices)
{
    for (TIndex i = 0; i < n_indices; i++)
        dst[i] = src[indices[i]];
}

// Tell the compiler not to inline this function for having a deterministic behavior.
// Originally, without the __restrict__ keyword, the compiler vectorizes the loop if this function is inlined, and does
// not vectorize it if the function is not inlined (since src != dst is not proven by the compiler)
__attribute__((noinline))
void gather_load(std::vector<TElement>& dst, std::vector<TElement>& src, std::vector<TIndex>& indices)
{
    restricted_gather_load(dst.data(), src.data(), indices.data(), indices.size());
}

class IndexGenerator
{
    private:
        TIndex seed;
        TIndex mod;
        TIndex next_index;
    public:
        IndexGenerator()
        {
        }
        IndexGenerator(TIndex seed, TIndex mod)
        {
            // To have a sparse distribution of the indices, `mod` should be a prime number
            this->seed = seed;
            this->mod = mod;
            this->next_index = seed;
        }
        void reset()
        {
            this->next_index = this->seed;
        }
        TIndex next()
        {
            TIndex curr_index = this->next_index;
            this->next_index *= seed;
            this->next_index %= mod;
            return curr_index;
        }
};

int main()
{
    const int SIZE = 100003;
    const int N_INDEX = 100002;

    std::vector<TElement> src(SIZE, 0);
    std::vector<TElement> dst(SIZE, 0);
    std::vector<TIndex> indices(N_INDEX, 0);

    // Initialize the arrays
    for (TIndex i = 0; i < SIZE; i++)
        src[i] = i;

    // Creating index
    const TElement seed = 3;
    const TElement mod = 100003;
    // This index generator will essentially perform a permutation of the src to the dst
    IndexGenerator rng(seed, mod);
    for (TIndex i = 0; i < N_INDEX; i++)
        indices[i] = rng.next();

    // Performing indexed-loads
    gather_load(dst, src, indices);

    // Checking result
    rng.reset();
    for (TIndex i = 0; i < N_INDEX; i++)
        assert(dst[i] == rng.next());

    return 0;
}
