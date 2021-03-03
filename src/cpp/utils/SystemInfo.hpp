// Copyright 2020 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef UTILS_SYSTEMINFO_HPP_
#define UTILS_SYSTEMINFO_HPP_

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif // if defined(_WIN32)

#include <cstdint>
#include <limits>
#include <random>

#include <utils/Host.hpp>

namespace eprosima {

/**
 * This singleton serves as a centralized point from where to obtain platform dependent system information.
 */
class SystemInfo
{
public:

    /**
     * Get the identifier of the current process.
     *
     * @return the identifier of the current process.
     */
    inline int process_id() const
    {
#if defined(__cplusplus_winrt)
        return (int)GetCurrentProcessId();
#elif defined(_WIN32)
        return (int)_getpid();
#else
        return (int)getpid();
#endif // platform selection
    }

    inline uint32_t unique_process_id() const
    {
        return unique_process_id_;
    }

    /**
     * Get the identifier of the current host.
     *
     * @return the identifier of the current host.
     */
    inline uint16_t host_id() const
    {
        return Host::instance().id();
    }

    /**
     * Get a reference to the singleton instance.
     *
     * @return reference to the singleton instance.
     */
    static const SystemInfo& instance()
    {
        static SystemInfo singleton;
        return singleton;
    }

private:

    SystemInfo()
    {
        create_unique_process_id();
    }

    void create_unique_process_id()
    {
        // Generate a 4 bytes unique identifier that would be the same across all participants on the same process.
        // This will be used on the GuidPrefix of the participants, as well as on the SHM transport unicast locators.

        // Even though using the process id here might seem a nice idea, there are cases where it might not serve as
        // unique identifier of the process:
        // - One of them is when using a Kubernetes pod on which several containers with their own PID namespace are
        //   created.
        // - Another one is when a system in which a Fast DDS application is started during boot time. If the system
        //   crashes and is then re-started, it may happen that the participant may be considered an old one if the
        //   announcement lease duration did not expire.
        // In order to behave correctly in those situations, we will use the 16 least-significant bits of the PID,
        // along with a random 16 bits value. This should not be a problem, as the PID is known to be 16 bits long on
        // several systems. On those where it is longer, using the 16 least-significant ones along with a random value
        // should still give enough uniqueness for our use cases.
        int pid = process_id();

        std::random_device generator;
        std::uniform_int_distribution<uint16_t> distribution(0, (std::numeric_limits<uint16_t>::max)());
        uint16_t rand_value = distribution(generator);

        unique_process_id_ = (static_cast<uint32_t>(rand_value) << 16) | static_cast<uint32_t>(pid & 0xFFFF);
    }

    uint32_t unique_process_id_;

};

} // namespace eprosima

#endif // UTILS_SYSTEMINFO_HPP_
