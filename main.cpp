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

thread_local uint64_t state[16];

#define STORE(on, n) movq %on, %fs:.Lstate$local@TPOFF+n

static thread_local uint64_t tls_temp;

uint64_t get_tls_tmp_ofs() {
    uint64_t out;
    asm
    (
      "leaq _ZL8tls_temp@TPOFF, %0 \n"
      :"=r"(out)
    );
    return out;
}

[[noreturn]]		
void swapout_jit() {
/*
    STORE(rax, 0)
    STORE(rbx, 0)
    STORE(rcx, 0)
    STORE(rdx, 0)

    STORE(rsi, 0)
    STORE(rdi, 0)
    STORE(rsp, 0)
    STORE(rbp, 0)
*/

    
    // store to $rax
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

        if (instruction.info.attributes & ZYDIS_ATTRIB_IS_RELATIVE) {
            

            for (int i = 0; i < ZYDIS_MAX_OPERAND_COUNT; i++) {
                if (instruction.operands[i].encoding == ZYDIS_OPERAND_ENCODING_MODRM_RM) {

                    if (instruction.operands[i].mem.base == ZYDIS_REGISTER_RIP) {
                        auto real_addy = instruction.operands[i].mem.disp.value + runtime_address + instruction.info.length;
                        printf("operand: %d, encoding: %d\n", i, instruction.operands[i].encoding);
                        printf("details: base: %d disp: %lX val: %lX type: %d \n",
                        (int)instruction.operands[i].mem.base,
                        instruction.operands[i].mem.disp.value,
                        real_addy,
                        instruction.operands[i].mem.type);

                        assert(i==1);

                        //XCHG    %rax,%fs:~
                        *buffer++ = 0x64;
                        *buffer++ = 0x48;
                        *buffer++ = 0x87;
                        *buffer++ = 0x04;
                        *buffer++ = 0x25;

                        auto tls_offs = get_tls_tmp_ofs();
                        *buffer++ = reinterpret_cast<uint64_t>(&tls_offs) >> 0;
                        *buffer++ = reinterpret_cast<uint64_t>(&tls_offs) >> 8;
                        *buffer++ = reinterpret_cast<uint64_t>(&tls_offs) >> 16;
                        *buffer++ = reinterpret_cast<uint64_t>(&tls_offs) >> 24;

                        // mov rax, helper
                        *buffer++ = 0x48;
                        *buffer++ = 0xb8;

                        *buffer++ = reinterpret_cast<uint64_t>(&real_addy) >> 0;
                        *buffer++ = reinterpret_cast<uint64_t>(&real_addy) >> 8;
                        *buffer++ = reinterpret_cast<uint64_t>(&real_addy) >> 16;
                        *buffer++ = reinterpret_cast<uint64_t>(&real_addy) >> 24;
                        *buffer++ = reinterpret_cast<uint64_t>(&real_addy) >> 32;
                        *buffer++ = reinterpret_cast<uint64_t>(&real_addy) >> 36;
                        *buffer++ = reinterpret_cast<uint64_t>(&real_addy) >> 48;
                        *buffer++ = reinterpret_cast<uint64_t>(&real_addy) >> 56;

                        // now replace rip with rax
                        ZydisEncoderRequest req;
                        memset(&req, 0, sizeof(req));

                        req.mnemonic = instruction.info.mnemonic;
                        req.machine_mode = instruction.info.machine_mode;
                        req.operand_count = instruction.info.operand_count;

                        assert(instruction.operands[0].type == ZYDIS_OPERAND_TYPE_REGISTER);

                        req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
                        req.operands[0].reg.value = instruction.operands[0].reg.value;
                        
                        req.operands[1].type = ZYDIS_OPERAND_TYPE_MEMORY;
                        req.operands[1].mem.base = ZYDIS_REGISTER_RAX;
                        req.operands[1].mem.displacement = 0;
                        req.operands[1].mem.index = ZYDIS_REGISTER_NONE;
                        req.operands[1].mem.scale = 0;
                        req.operands[1].mem.size = instruction.info.operand_width / 8;

                        ZyanUSize encoded_length = 15;

                        if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, buffer, &encoded_length)))
                        {
                            puts("Failed to encode instruction");
                            exit(1);
                        }

                        buffer += encoded_length;

                        // now all done, swap back tmp
                        //XCHG    %rax,%fs:~
                        *buffer++ = 0x64;
                        *buffer++ = 0x48;
                        *buffer++ = 0x87;
                        *buffer++ = 0x04;
                        *buffer++ = 0x25;

                        *buffer++ = reinterpret_cast<uint64_t>(&tls_offs) >> 0;
                        *buffer++ = reinterpret_cast<uint64_t>(&tls_offs) >> 8;
                        *buffer++ = reinterpret_cast<uint64_t>(&tls_offs) >> 16;
                        *buffer++ = reinterpret_cast<uint64_t>(&tls_offs) >> 24;

                        // Processed this one!
                        offset += instruction.info.length;
                        runtime_address += instruction.info.length;
                        continue;
                    }
                }
            }
            
        }
        if (instruction.info.meta.branch_type != ZYDIS_BRANCH_TYPE_NONE) {
            printf("Branch0rz\n");

            // push RAX
            *buffer++ = 0x50;
            // And Align
            *buffer++ = 0x50;

            // mov rax, helper
            *buffer++ = 0x48;
            *buffer++ = 0xb8;

            *buffer++ = reinterpret_cast<uint64_t>(&swapout_jit) >> 0;
            *buffer++ = reinterpret_cast<uint64_t>(&swapout_jit) >> 8;
            *buffer++ = reinterpret_cast<uint64_t>(&swapout_jit) >> 16;
            *buffer++ = reinterpret_cast<uint64_t>(&swapout_jit) >> 24;
            *buffer++ = reinterpret_cast<uint64_t>(&swapout_jit) >> 32;
            *buffer++ = reinterpret_cast<uint64_t>(&swapout_jit) >> 36;
            *buffer++ = reinterpret_cast<uint64_t>(&swapout_jit) >> 48;
            *buffer++ = reinterpret_cast<uint64_t>(&swapout_jit) >> 56;

            // jmp rax
            *buffer++ = 0xff;
            *buffer++ = 0xe0;

            break;
        } else {
            memcpy(buffer, (void*)runtime_address, instruction.info.length);
            buffer += instruction.info.length; 
        }

        offset += instruction.info.length;
        runtime_address += instruction.info.length;
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