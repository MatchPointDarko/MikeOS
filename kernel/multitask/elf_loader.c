/*
 * MikeOS: ELF Files loader.
 */

#if 0
#include "logger.h"

static bool_t check_elf_header(Elf32_Ehdr* elf_header);
static bool_t check_elf_supported(Elf32_Ehdr* elf_header);
static inline Elf32_Shdr* get_elf_first_section_header(Elf32_Ehder* elf_header);
static inline Elf32_Shdr* get_section_header(Elf32_Ehder* elf_header, unsigned int section_index);
static void load_sections(Elf32_Ehder* elf_header);

bool_t load_elf(void* memory_mapped_file)
{
    Elf32_Ehdr* elf_header = (Elf32_Ehder*)memory_mapped_file;

    if(check_elf_header(elf_header) == false)
    {
        return false;
    }

    switch(elf_header->e_type)
    {
        case ET_EXEC:
        {
            load_sections(memory_mapped_file);
        }
        break;

        case ET_REL:
        {
            //TODO
        }
        break;
    }

    return true;
}

static bool_t check_elf_supported(Elf32_Ehdr* elf_header)
{
    if(check_elf_header(memory_mapped_file) == false)
    {
        log_print(LOG_log_print(LOG_ERROR, "Invalid elf file");
        return false;
    }

    if(elf_header->e_ident[EI_CLASS] != ELFCLASS32)
    {
        log_print(LOG_ERROR, "Unsupported ELF File Class");
        return false;
    }

    if(elf_header->e_ident[EI_DATA] != ELFDATA2LSB)
    {
        log_print(LOG_ERROR, "Unsupported ELF File byte order");
        return false;
    }

    if(elf_header->e_machine != EM_386)
    {
        log_print(LOG_ERROR, "Unsupported ELF File target");
        return false;
    }

    if(elf_header->e_ident[EI_VERSION] != EV_CURRENT)
    {
        log_print(LOG_ERROR, "Unsupported ELF File version");
        return false;
    }

    if(elf_header->e_type != ET_REL && elf_header->e_type != ET_EXEC)
    {
        log_print(LOG_ERROR, "Unsupported ELF File type");
        return false;
    }

    return true;
}

static bool_t check_elf_header(Elf32_Ehdr* elf_header)
{
    if(elf_header == NULL)
    {
        return false;
    }

    if(elf_header->e_ident[EI_MAG0] != ELFMAG0)
    {
        return false;
    }
        if(elf_header->e_ident[EI_MAG1] != ELFMAG1)
    {
        return false;
    }

    if(elf_header->e_ident[EI_MAG2] != ELFMAG2)
    {
        return false;
    }

    if(elf_header->e_ident[EI_MAG3] != ELFMAG3)
    {
        return false;
    }

    return true;
}

static inline Elf32_Shdr* get_elf_first_section_header(Elf32_Ehder* elf_header)
{
    return (Elf32_Shdr*)((unsigned int)elf_header + elf_header->e_shoff);
}

static inline Elf32_Shdr* get_section_header(Elf32_Ehder* elf_header, unsigned int section_index)
{
    return &get_elf_section_header(elf_header)[section_index];
}

static void load_sections(Elf32_Ehder* elf_header)
{
    unsigned int number_of_sections = elf_header->e_shnum;

    for(int i = 0; i < number_of_sections; i++)
    {
        Elf32_Shdr* section = get_section_header(elf_header, i);
        if(section->sh_type == SHT_NOBITS)
        {
            if(section->sh_size == 0) continue;

            if(section->sh_flags & SHF_ALLOC)
            {
                log_print("Loading a section starting at address: %x", section->sh_addr);
                //Zero this.
                memset(0, (void *)section->sh_addr, section->sh_size);
            }
        }

        else if(section->sh_type == SHT_PROGBITS)
        {
            log_print("Loading a section starting at address: %x", section->sh_addr);
            // Load this shit.
            void* section_in_elf = (unsigned int)elf_header + section->sh_offset;
            memcpy((void *)section->sh_addr, section_in_elf, section->h_size);
        }
    }
}

#endif
