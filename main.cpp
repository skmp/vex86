#include <stdio.h>
#include <stdint.h>
#include <cstring>
#include <unordered_map>

#include <sys/mman.h>

#include <inttypes.h>
extern "C" {
#include <Zydis/Zydis.h>
#include <Zydis/Zydis.h>
}

void test_fn() {
    printf("I am a test fn\n");
}

typedef void MetafrasmenosKodikas();

uint8_t* buffer_start;
uint8_t* buffer;

[[noreturn]]		
void really_exit_asap() {
    printf("really_exit_asap\n");
    fflush(stdout);
    exit(3);
}

[[noreturn]]		
void reroute_jit() {
    really_exit_asap();
}


MetafrasmenosKodikas* metafrasi_kodika(uint8_t* diefthinsi_eisodou) {

    auto ArxiMetafrasis = reinterpret_cast<MetafrasmenosKodikas*>(buffer);

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

            // push RAX
            *buffer++ = 0x50;
            // And Align
            *buffer++ = 0x50;

            // mov rax, helper
            *buffer++ = 0x49;
            *buffer++ = 0xb8;

            *buffer++ = reinterpret_cast<uint64_t>(&reroute_jit) >> 0;
            *buffer++ = reinterpret_cast<uint64_t>(&reroute_jit) >> 8;
            *buffer++ = reinterpret_cast<uint64_t>(&reroute_jit) >> 16;
            *buffer++ = reinterpret_cast<uint64_t>(&reroute_jit) >> 24;
            *buffer++ = reinterpret_cast<uint64_t>(&reroute_jit) >> 32;
            *buffer++ = reinterpret_cast<uint64_t>(&reroute_jit) >> 36;
            *buffer++ = reinterpret_cast<uint64_t>(&reroute_jit) >> 48;
            *buffer++ = reinterpret_cast<uint64_t>(&reroute_jit) >> 56;

            // jmp rax
            *buffer++ = 0xff;
            *buffer++ = 0xe0;

            //ZydisAsssemble
            break;
        }
    }

    return ArxiMetafrasis;
}

uint8_t* ErxomeniKseniDiefthinsi;

std::unordered_map<uint8_t*, MetafrasmenosKodikas*> full_map;
MetafrasmenosKodikas* anazitisi_metafrasis(uint8_t* GuestCode) {
    auto it = full_map.find(GuestCode);

    if (it != full_map.end()) {
        return it->second;
    } else {
        auto value = metafrasi_kodika(GuestCode);
        auto it = full_map.emplace(GuestCode, value);
        return value;
    }
}


template<typename T>
void run(T *fn) {
    auto diefthinsi_eisodou = reinterpret_cast<uint8_t*>(fn);

    auto kodikas = anazitisi_metafrasis(diefthinsi_eisodou);

    kodikas();
}

int main(int, const char **) {
    buffer_start = buffer = (uint8_t*)mmap(NULL, 4096 * 16, PROT_READ | PROT_EXEC | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    printf("Allocated buffer: %p\n", buffer_start);
    test_fn();

    run(&test_fn);
}