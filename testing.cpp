#include <cstdint>

[[noreturn]]
int main(){
    uint64_t rax;
    uint64_t rsi;
    uint64_t rbx;
    // mov rax, gs:[0x60]
    asm volatile("movq %%gs:[0x60], %[Var]" : [Var] "=r" (rax));

    // mov rax, [rax + 0x18]
    rax = *(reinterpret_cast<uint64_t*>(reinterpret_cast<void*>(rax) + 0x18));
    // mov rsi, [rax + 0x20]
    rsi = *(reinterpret_cast<uint64_t*>(reinterpret_cast<void*>(rax) + 0x20));

    // lodsq
    rax = *(reinterpret_cast<uint64_t*>(rsi));
    rsi += 8;

    // xchg rax, rsi
    uint64_t temp = rax;
    rax = rsi;
    rsi = temp;

    // lodsq
    rax = *(reinterpret_cast<uint64_t*>(rsi));
    rsi += 8;

    // mov rbx, [rax + 0x20]
    rbx = *(reinterpret_cast<uint64_t*>(reinterpret_cast<void*>(rax) + 0x20));

    void* kernel32Base = reinterpret_cast<void*>(rbx);

    reinterpret_cast<void(*)(uint32_t)>(rbx + 268448)(60);
}
