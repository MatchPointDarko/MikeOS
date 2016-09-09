#!/usr/bin/python
"""
Create a very simple initrd.
[inode1][file_data1][inode2][file_data2] ...

Usage:
    initrd_gen.py <directory_name>
"""
import os
import struct
import ctypes
import itertools
from docopt import docopt

INITRD_DIR = '/home/sourcer/mike_os/grub/boot/'
INITRD_NAME = 'initrd.img'
INITRD_PATH = os.path.join(INITRD_DIR, INITRD_NAME)
NAME_SIZE = 20

def inode_to_raw(name, offset, size):

    name = ctypes.create_string_buffer(name, NAME_SIZE).raw
    size = struct.pack('I', size)
    offset = struct.pack('I', offset)
    return ''.join([offset, size, name])

def dir_iter(directory_name):
    for current_dir, sub_dirs, files_in_dir \
                                        in os.walk(directory_name):
        for file_path in files_in_dir:
            yield os.path.abspath(os.path.join(current_dir, file_path))

def accumulate(iterator):
    total = 0
    for item in iterator:
        total += item
        yield total 

def create_initrd(directory_name):
    files = []
    offset = 0
    
    with open(INITRD_PATH, 'w+') as initrd:
        files_paths = list(dir_iter(directory_name))

        files_sizes = [os.stat(file_name)[6]
                       for file_name in files_paths]
        files_offsets = list(files_sizes)
        files_offsets.insert(0, 0)
        files_offsets.pop(len(files_offsets) - 1)
        files_offsets = accumulate(files_offsets)
        files_names = [os.path.basename(file_path) 
                       for file_path in files_paths]
        
        inodes_table = \
                [inode_to_raw(name,
                              offset, 
                              size)
                 for name, offset, size in itertools.izip(files_names,
                                                          files_offsets,
                                                          files_sizes)]
        initrd.write(struct.pack('I', len(files_paths)))
        initrd.write(''.join(inodes_table))
        
        for file_path in files_paths:
            with open(file_path, 'rb') as file_obj:
                initrd.write(file_obj.read())

def main(directory_name):
    if not os.path.isdir(directory_name):
        raise TypeError("Expected a directory")

    create_initrd(os.path.abspath(directory_name))


if __name__ == "__main__":
    arguments = docopt(__doc__)
    main(arguments['<directory_name>'])
