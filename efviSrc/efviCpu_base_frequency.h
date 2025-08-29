/* -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*- */
#ifndef CPU_BASE_FREQUENCY_H_INCLUDED
#define CPU_BASE_FREQUENCY_H_INCLUDED

// Copyright (c) 2019 Maxim Egorushkin. MIT License. See the full licence in file LICENSE.

#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace atomic_queue {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

double cpu_base_frequency();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct EfviCpuTopologyInfo {
    unsigned socket_id;
    unsigned core_id;
    unsigned hw_thread_id;
};
std::vector<EfviCpuTopologyInfo> get_cpu_topology_info();
std::vector<EfviCpuTopologyInfo> get_available_cpu_topology_info();

std::vector<EfviCpuTopologyInfo> sort_by_core_id(std::vector<EfviCpuTopologyInfo> const&);
std::vector<EfviCpuTopologyInfo> sort_by_hw_thread_id(std::vector<EfviCpuTopologyInfo> const&);

std::vector<unsigned> hw_thread_id(std::vector<EfviCpuTopologyInfo> const&);

efviVoid log_cpus(std::vector<EfviCpuTopologyInfo> const&) noexcept;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

efviVoid set_thread_affinity(unsigned hw_thread_id);
// efviVoid reset_thread_affinity();

efviVoid set_default_thread_affinity(unsigned hw_thread_id);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct EfviEnvBits64 {
    using U = unsigned long long;
    U value;

    EfviEnvBits64(char const* env_name, U default_bits=0);
    EfviEnvBits64(char const* env_name, U default_bits, U min, U max);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


} // namespace atomic_queue

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // CPU_BASE_FREQUENCY_H_INCLUDED


