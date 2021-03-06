/**********************************************************************************/
/* MIT License                                                                    */
/*                                                                                */
/* Copyright (c) 2020, 2021 JetBrains-Research                                    */
/*                                                                                */
/* Permission is hereby granted, free of charge, to any person obtaining a copy   */
/* of this software and associated documentation files (the "Software"), to deal  */
/* in the Software without restriction, including without limitation the rights   */
/* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      */
/* copies of the Software, and to permit persons to whom the Software is          */
/* furnished to do so, subject to the following conditions:                       */
/*                                                                                */
/* The above copyright notice and this permission notice shall be included in all */
/* copies or substantial portions of the Software.                                */
/*                                                                                */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     */
/* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       */
/* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    */
/* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         */
/* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  */
/* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  */
/* SOFTWARE.                                                                      */
/**********************************************************************************/

#include <cuda/instance.hpp>
#include <core/error.hpp>
#include <string>
#include <cstdlib>

namespace cubool {

    volatile Instance* Instance::gInstance = nullptr;

    Instance::Instance(bool useManagedMemory) {
        gInstance = this;
        mMemoryType = useManagedMemory? Managed: Default;

#ifdef CUBOOL_DEBUG
        sendMessage(CUBOOL_STATUS_SUCCESS, "Initialize CuBool instance");
        printDeviceCapabilities();
#endif // CUBOOL_DEBUG
    }

    void Instance::allocate(void* &ptr, size_t size) const {
        ptr = malloc(size);
        CHECK_RAISE_ERROR(ptr != nullptr, MemOpFailed, "Failed to allocate memory on the CPU");
        mHostAllocCount++;
    }

    void Instance::deallocate(void* ptr) const {
        CHECK_RAISE_ERROR(ptr != nullptr, InvalidArgument, "Passed null ptr to free");
        free(ptr);
        mHostAllocCount--;
    }

    Instance& Instance::getInstanceRef() {
        CHECK_RAISE_ERROR(gInstance != nullptr, InvalidState, "No instance in the system");
        return (Instance&) *gInstance;
    }

    Instance* Instance::getInstancePtr() {
        return (Instance* ) gInstance;
    }

    bool Instance::isInstancePresent() {
        return gInstance != nullptr;
    }

}