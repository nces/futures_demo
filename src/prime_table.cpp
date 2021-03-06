#include "prime_table.h"
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>
#include <fstream>
#include <exception>
#include <iostream>

using std::vector;
using std::ceil; using std::sqrt;
using std::ostream; using std::cout;
using std::to_string; using std::string;
using std::binary_search;
using std::make_shared; using std::shared_ptr; using std::unique_ptr; using std::make_unique;
using std::ofstream; using std::ifstream;
using std::runtime_error;
using std::cout;
using std::move;

shared_ptr<PrimeTable> PrimeTable::mp_instance = nullptr;

void PrimeTable::init(Element_t max_factor) {
    assert(mp_instance == nullptr && "Can only init PrimeTable once");
    mp_instance = make_shared<PrimeTable>(private_key_dummy{}, max_factor);
}

void PrimeTable::init(string filename) {
    assert(mp_instance == nullptr && "Can only init PrimeTable once");
    mp_instance = make_shared<PrimeTable>(private_key_dummy{}, filename);
}

PrimeTable* PrimeTable::instance() {
    assert(mp_instance && "Init the PrimeTable first");
    return mp_instance.get();
}

PrimeTable::PrimeTable(const private_key_dummy &pkd, string filename) {
    ifstream fs;
    size_t num_elements{ 0 };
    size_t elem_size{ 0 };

    try {
        fs.open(filename, std::ios_base::in | std::ios_base::binary);
        if (!fs.is_open()) {
            throw runtime_error(filename + " was not found.");
        }

        fs.read(reinterpret_cast<char*>(&num_elements), sizeof(size_t));
        fs.read(reinterpret_cast<char*>(&elem_size), sizeof(size_t));

        // ensure container has enough room
        m_primes.resize(num_elements);

        // write new data to container's data
        fs.read(reinterpret_cast<char*>(m_primes.data()), num_elements * elem_size);
    }
    catch (runtime_error &e) {
        cout << e.what() << '\n';
    }
    catch (...) {
        // TODO do something here if needed
    }

    fs.close();
}

PrimeTable::PrimeTable(const private_key_dummy &pkd, Element_t max_factor, CtorAlgorithm alg)
    : m_primes{ 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31 }
{
    assert(max_factor > 0);

    const Element_t limit = max_factor;

    switch (alg) {
    case CtorAlgorithm::NAIVE:
        for (Element_t i = 37; i <= limit; ++i) {
            const Element_t i_limit = static_cast<Element_t>(ceil(sqrt(i)));
            bool isPrime = true;

            for (Element_t j = 0; j < m_primes.size(); ++j) {
                // Don't check beyond sqrt
                if (m_primes[j] > i_limit) {
                    break;
                }

                // Is number evenly divisable by something
                if (i % m_primes[j] == 0) {
                    isPrime = false;
                    break;
                }
            }

            if (isPrime) {
                m_primes.push_back(i);
            }
        }
        break;
    case CtorAlgorithm::RESTRICTED_MEMORY_ERATOSTHENES:
        break;
    case CtorAlgorithm::NAIVE_ERATOSTHENES:
    {
        // set up an array of potential numbers that need to be checked for primliness.
        const size_t num_array_size = limit - 31;
        auto num_ptr = make_unique<Element_t[]>(num_array_size);
        Element_t *num_array = num_ptr.get();

        for (Element_t n = 32, i = 0; i < num_array_size;) {
            num_array[i++] = n++;
        }

        Element_t tmp_mask = 0x80;
        for (unsigned char i = 0; i < sizeof(Element_t) - 1; ++i) {
            tmp_mask = tmp_mask * 256; // bit-shift 8L
        }
        const Element_t mark_mask = tmp_mask;

        assert(mark_mask > limit && ~mark_mask & (limit ^ mark_mask) == limit); // ensure marked bit isn't used for any element <= limit

        break;
    }
    default:
        throw runtime_error("Something horked up with PrimeTable Ctor, default case hit");
    }
}

bool PrimeTable::contains(Element_t val) const {
    return binary_search(m_primes.begin(), m_primes.end(), val,
        [](Element_t lhs, Element_t rhs) { return lhs < rhs; });
}

ostream& operator<<(ostream& os, const PrimeTable& pt) {
    uint32_t i = 5;
    for (auto val : pt.m_primes) {
        os << to_string(val) + "   ";

        if (i-- == 0) {
            os << '\n';
            i = 5;
        }
    }

    return os;
}

/* Loads a prime number table from a binary file. The format of the binary file is as follows:
    size_t number_of_elements
    size_t size_of_each_element
    element_t[] elements
*/
bool PrimeTable::saveToFile(std::string filename) const {
    bool success = false;
    ofstream fs;
    const size_t pt_size = m_primes.size();
    constexpr size_t element_size = sizeof(PrimeTable::Element_t);

    try {
        fs.open(filename, std::ios_base::out | std::ios_base::binary);
        if (!fs.is_open()) {
            throw runtime_error(filename + " was not found.");
        }

        fs.write(reinterpret_cast<const char*>(&pt_size), sizeof(size_t));
        fs.write(reinterpret_cast<const char*>(&element_size), sizeof(size_t));
        fs.write(reinterpret_cast<const char*>(m_primes.data()), pt_size * element_size);
        success = true;
    }
    catch (runtime_error &e) {
        cout << e.what() << '\n';
    }
    catch (...) {
        // TODO do something here if needed
    }

    fs.close();
    return success;
}

