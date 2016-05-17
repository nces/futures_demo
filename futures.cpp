#include <iostream>
#include <future>
#include <vector>
#include <string>
#include <cassert>
#include <limits>
#include <cmath>
#include <utility>
#include <chrono>

#include "prime_factorization.h"
#include "prime_table.h"

using std::vector;
using std::future; using std::async; using std::launch;
using std::cout;
using std::ceil; using std::sqrt;
using std::move;
using namespace std::chrono;

constexpr uint32_t kAMOUNT = 1000000;

static void factorize(PrimeFactorization* out, const vector<uint32_t>& data,
               uint32_t start_ele, uint32_t num_eles) 
{
    for (uint32_t i = 0; i < num_eles; i++) {
        out[i] = PrimeFactorization(data[i + start_ele]);
    }
}

/* TODO Can be improved by using stride and offset, this is because nums may be
   sorted and larger numbers will take longer to factorize causing the jobs with
   larger numbers taking more time... good exercise I suppose, the main goal was
   to use futures correctly and show how they improve performance. That goal is
   accomlished */
void parallel_factorize(vector<PrimeFactorization> &pf,
                        const vector<uint32_t>& nums,
                        const uint32_t jobs = 4)
{
    assert(jobs < pf.size() && jobs != 0); // Would want different solution for this case

    // set the amount of work for each job and account for division rounding
    const uint32_t nums_per_job = nums.size() / jobs;
    const uint32_t nums_last_job = nums.size() - nums_per_job * (jobs - 1);

    vector<future<void>> futures(jobs);

    // launch jobs asynchronously
    for (uint32_t i = 0; i < jobs - 1; i++) {
        const uint32_t start_idx = i * nums_per_job;
        futures[i] = async(launch::async, factorize,
            pf.data() + start_idx, nums, start_idx, nums_per_job);
    }

    // special case for last job to account for division rounding
    const uint32_t start_idx = nums.size() - nums_last_job;
    futures[jobs - 1] = async(launch::async, factorize,
        pf.data() + start_idx, nums, start_idx, nums_last_job);

    // wait for all jobs to finish
    for (auto& f : futures) {
        f.get();
    }
}

#ifdef WIN32
int main(uint32_t argc, char* argv[]) {
#else
int main(int argc, char* argv[]) {
#endif
    PrimeTable::init(static_cast<uint32_t>(ceil(sqrt(UINT32_MAX))));
    vector<PrimeFactorization> pf_vec0(kAMOUNT);
    vector<PrimeFactorization> pf_vec1(kAMOUNT);

    vector<uint32_t> nums;
    nums.reserve(kAMOUNT);

    for (uint32_t i = 0; i < kAMOUNT; i++) {
        nums.push_back(i);
    }

    auto t0 = high_resolution_clock::now();
    parallel_factorize(pf_vec0, nums, 8);
    auto t1 = high_resolution_clock::now();
    auto span0 = duration_cast<duration<double>>(t1 - t0);

    auto t2 = high_resolution_clock::now();
    parallel_factorize(pf_vec1, nums, 1);
    auto t3 = high_resolution_clock::now();
    auto span1 = duration_cast<duration<double>>(t3 - t2);

    //cout << *PrimeTable::instance() << '\n';
    for (uint32_t i = (kAMOUNT - 10 < kAMOUNT) ? kAMOUNT - 10 : 0; i < kAMOUNT; i++) {
        cout << pf_vec0[i] << pf_vec1[i];
    }

    cout << "8 jobs time: " << span0.count() << '\n';
    cout << "1 job time: " << span1.count() << '\n';
    return 0;
}
