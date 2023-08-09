#include <stdio.h>
#include <stdint.h>
#include <cstring>

#include <sys/mman.h>

#include <inttypes.h>
extern "C" {
#include <Zydis/Zydis.h>
}

void test_fn() {
    printf("I am a test fn\n");
}

uint8_t* buffer_start;
uint8_t* buffer;

int really_exit_asap() {
    printf("really_exit_asap\n");
    exit(3);
}

template<typename T>
void simulate(T *fn) {
    auto diefthinsi_eisodou = reinterpret_cast<uint8_t*>(fn);

    printf("%p = %d\n", diefthinsi_eisodou, *(uint8_t*)diefthinsi_eisodou);

    auto runtime_address = reinterpret_cast<ZyanU64>(diefthinsi_eisodou);

    // Loop over the instructions in our buffer.
    ZyanUSize offset = 0;
    ZydisDisassembledInstruction instruction;
    while (ZYAN_SUCCESS(ZydisDisassembleIntel(
        /* machine_mode:    */ ZYDIS_MACHINE_MODE_LONG_64,
        /* runtime_address: */ runtime_address,
        /* buffer:          */ diefthinsi_eisodou + offset,
        /* length:          */ 16,
        /* instruction:     */ &instruction
    ))) {
        printf("%016" PRIX64 "  %s\n", runtime_address + offset, instruction.text);
        memcpy(buffer, (void*)runtime_address, instruction.info.length);
        buffer += instruction.info.length; 

        offset += instruction.info.length;
        runtime_address += instruction.info.length;

        if (instruction.info.meta.branch_type != ZYDIS_BRANCH_TYPE_NONE) {
            printf("Branch0rz\n");
            break;
        }
    }
}

int main(int, const char **) {
    buffer_start = buffer = (uint8_t*)mmap(NULL, 4096 * 16, PROT_READ | PROT_EXEC | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    printf("Allocated buffer: %p\n", buffer_start);
    test_fn();

    simulate(&test_fn);
}