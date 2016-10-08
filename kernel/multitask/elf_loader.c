/*
 * MikeOS: ELF Files loader.
 */

#include <logging/logger.h>
#include <multitask/elf.h>
#include <fs/file.h>
#include <libc/memory.h>
#include <error_codes.h>

#define ELF_HEADER_SIZE (sizeof(Elf32_Ehdr))

static error_code_t check_elf_header(Elf32_Ehdr* elf_header);
static error_code_t check_elf_supported(Elf32_Ehdr* elf_header);
static inline Elf32_Shdr* get_section_header(Elf32_Ehdr* elf_header, unsigned int section_index);
static error_code_t load_segments(task_t* task, Elf32_Ehdr* elf_header, file_t* elf_file);

error_code_t load_elf(task_t* task)
{
    if(task == NULL)
    {
        return INVALID_ARGUMENT;
    }

    file_t* elf_file = file_open(task->name, "r");

    if(elf_file == NULL)
    {
        return NO_SUCH_FILE;
    }

    uint8_t elf_header_buf[ELF_HEADER_SIZE] = {0};
    Elf32_Ehdr* elf_header = (Elf32_Ehdr*)elf_header_buf;

    if(file_read(elf_file, elf_header_buf, ELF_HEADER_SIZE) == 0)
    {
        //This can't be, this file is empty..
        return FAILURE;
    }

    if(check_elf_header(elf_header) != SUCCESS)
    {
        //This is not an elf file.
        return FAILURE;
    }

    if(check_elf_supported(elf_header) != SUCCESS)
    {
        return FAILURE;
    }

    switch(elf_header->e_type)
    {
        case ET_EXEC:
        {
            load_segments(task, elf_header, elf_file);
        }
        break;

        case ET_REL:
        {
            //TODO
        }
        break;
    }

    return SUCCESS;
}

static error_code_t check_elf_supported(Elf32_Ehdr* elf_header)
{
    if(elf_header->e_ident[EI_CLASS] != ELFCLASS32)
    {
        log_print(LOG_ERROR, "Unsupported ELF File Class");
        return INVALID_ARGUMENT;
    }

    if(elf_header->e_ident[EI_DATA] != ELFDATA2LSB)
    {
        log_print(LOG_ERROR, "Unsupported ELF File byte order");
        return INVALID_ARGUMENT;
    }

    if(elf_header->e_machine != EM_386)
    {
        log_print(LOG_ERROR, "Unsupported ELF File target");
        return INVALID_ARGUMENT;
    }

    if(elf_header->e_ident[EI_VERSION] != EV_CURRENT)
    {
        log_print(LOG_ERROR, "Unsupported ELF File version");
        return INVALID_ARGUMENT;
    }

    if(elf_header->e_type != ET_REL && elf_header->e_type != ET_EXEC)
    {
        log_print(LOG_ERROR, "Unsupported ELF File type");
        return INVALID_ARGUMENT;
    }

    return SUCCESS;
}

static error_code_t check_elf_header(Elf32_Ehdr* elf_header)
{
    if(elf_header == NULL)
    {
        return INVALID_ARGUMENT;
    }

    if(elf_header->e_ident[EI_MAG0] != ELFMAG0)
    {
        return INVALID_ARGUMENT;
    }

    if(elf_header->e_ident[EI_MAG1] != ELFMAG1)
    {
        return INVALID_ARGUMENT;
    }

    if(elf_header->e_ident[EI_MAG2] != ELFMAG2)
    {
        return INVALID_ARGUMENT;
    }

    if(elf_header->e_ident[EI_MAG3] != ELFMAG3)
    {
        return INVALID_ARGUMENT;
    }

    //This is an elf header, probably an elf file
    return SUCCESS;
}

static inline uint32_t load_program_header_table(file_t* elf_file,
                                                 Elf32_Ehdr* elf_header,
                                                 Elf32_Phdr* segments_headers,
                                                 uint32_t number_of_segments)
{
    //Assumes the offset is after the header.
    file_seek(elf_file, elf_header->e_phoff, SEEK_SET);
    return file_read(elf_file, (uint8_t *)segments_headers, number_of_segments * sizeof(Elf32_Phdr));
}

static error_code_t load_segments(task_t* task, Elf32_Ehdr* elf_header, file_t* elf_file)
{
    uint32_t number_of_segments = elf_header->e_phnum;

    Elf32_Phdr segments[number_of_segments];

    if(!load_program_header_table(elf_file, elf_header, segments, number_of_segments))
    {
        //Couldn't read any bytes..
        return FAILURE;
    }

    for(int i = 0; i < number_of_segments; i++)
    {
        if(segments[i].p_type == PT_LOAD)
        {
            uint32_t segment_memory_size = segments[i].p_memsz;
            //Align to page size
            uint32_t aligned_mem_size = PAGE_BOUNDARY(segment_memory_size);

            if(aligned_mem_size < segment_memory_size)
            {
                aligned_mem_size += PAGE_SIZE;
            }

            uint32_t segment_file_size = segments[i].p_filesz;
            uint32_t segment_virtual_address = segments[i].p_vaddr;

            if(mmap(segment_virtual_address, (segment_memory_size / PAGE_SIZE) + 1) != SUCCESS)
            {
                return FAILURE;
            }

            file_seek(elf_file, segments[i].p_offset, SEEK_SET);
            if(!file_read(elf_file, segment_virtual_address, segment_file_size))
            {
                return FAILURE;
            }

            //Zero the leftovers.
            if(segment_memory_size > segment_file_size)
            {
                memset(0, segment_virtual_address + segment_file_size, aligned_mem_size - segment_file_size);
            }

            //Update the task struct
            if(segments[i].p_flags == PF_RX)
            {
                task->text_begin = segment_virtual_address;
            }

            if(segments[i].p_flags == PF_RW)
            {
                task->data_begin = segment_virtual_address;
            }
        }
    }

    return SUCCESS;
}

