'''
This script will generate module IDs from the SSS_ConstantModuleIds.xslx file.

The file shall be downloaded and the path input to this file and the generated
Kernel_module_ids.h file should be checked before being committed.
'''

import sys
import os
from datetime import datetime
from pathlib import Path
import re
import openpyxl
import json

def generate_mod_ids(excel_file_path):
    '''
    Generate module IDs from the given excel file path.
    '''

    # Extract the source file name
    source_name = excel_file_path.name
    
    # Extract the version number using regex
    ver_match = re.search(r'v([\d\.]+)', source_name)
    if ver_match is None:
        raise RuntimeError('Couldn\'t find version number in file name')
    version = ver_match.group(1)

    # Open the file
    workbook = openpyxl.load_workbook(
        excel_file_path, 
        read_only=False, 
        data_only=True
    )
    
    # ids_sheet = workbook.sheet_by_name('Module IDs')
    ids_sheet = workbook['Module IDs']

    # Get the ID definitions
    id_defs = get_id_defs(ids_sheet)

    # Generate the header file
    module_defs_string_header = gen_mod_ids_file(id_defs, version, source_name)
    app_defs_string_header = gen_app_ids_file(id_defs, version, source_name)

    # Output the header file in the current working directory
    with open('Kernel_module_ids.h', 'w+') as file:
        file.write(module_defs_string_header)
    
    # Output the module IDs as JSON for easy reading by the DataPool generate
    # code. 
    with open('Kernel_module_ids.json', 'w+') as file:
        json.dump(id_defs, file, indent=4)

    # Output the header file in the current working directory
    with open('Kernel_app_ids.h', 'w+') as file:
        file.write(app_defs_string_header)

    print(f'Constant Module IDs written to {os.getcwd()}/Kernel_module_ids.h')
    print(f'JSON file written to {os.getcwd()}/Kernel_module_ids.json')

def get_id_defs(sheet):
    '''
    Extract an array of ID definitions, each element being a dict with format:
    {
        group_name: 'System',
        module_name: 'Fdir',
        module_id: 2,
        definition: 'MOD_ID_FDIR'
    }
    '''

    # Empty array to return
    modules = []

    # Group names are in merged cells, however openpyxl can't read these, so we
    # keep a copy of the last value of the group name and update it when we see
    # a non-None value
    current_group = None

    # For all rows that have a module definition
    for row in range(6, 70):
        module = {}

        # First read the definition, if it's none the index is spare so we
        # ignore it
        module['definition'] = sheet[f'G{row}'].value
        if module['definition'] is None:
            continue

        # Next read the group name, which as stated above may be modified
        module['group_name'] = sheet[f'B{row}'].value
        if module['group_name'] is None:
            module['group_name'] = current_group
        else:
            current_group = module['group_name']

        # Finally read the last two items
        module['module_name'] = sheet[f'C{row}'].value
        module['module_id'] = int(sheet[f'D{row}'].value, 0)

        modules.append(module)

    return modules

def gen_app_id_def(id_def):
    '''
    Generate a #define for an individual ID
    '''
    return f'''
/**
 * @brief App ID for the {id_def["module_name"]} application.
 */
#define {id_def["definition"].replace("MOD", "APP")} ((Kernel_AppId)(0x{id_def["module_id"]:02X}))
'''

def gen_mod_id_def(id_def):
    '''
    Generate a #define for an individual ID
    '''
    return f'''
/**
 * @brief Shifted module ID for the {id_def["module_name"]} module ({id_def["group_name"]} group).
 */
#define {id_def["definition"]} ((uint16_t)(0x{id_def["module_id"]:02X}<<KERNEL_MOD_ID_SHIFT))
'''

def gen_mod_ids_file(defs, version, source_name):
    '''
    Given the definitions array generate the header file string.
    '''
    newline = '\n'
    return f'''
/**
 * @ingroup kernel
 * @file Kernel_module_ids.h
 * @author Duncan Hamill (dh2g16@soton.ac.uk/duncanrhamill@googlemail.com)
 * @brief Module IDs for use in reporting constants.
 * 
 * The OBC software uses 16-bit constants in many places, for instance for 
 * DataPool IDs, Event IDs, error codes etc. The concept of a Constant Module 
 * ID is that the first 6 bits of these constants are reserved for a module ID 
 * that identified the module that constant is associated with. For instance 
 * the OBC kernel (main system) is module ID 0 - meaning that an event starting
 * with 0b000000 came from the kernel itself. In a 16-bit unsigned integer this
 * leaves 10 bits available for constants, a total of 1024 different 
 * possibilities. 6 bits of module ID allows for up to 64 modules.
 * 
 * This file defines those IDs for the source code. There is a corresponding
 * document {source_name} on the sharepoint which
 * defines these values officially. This file should mirror that file at all
 * times. 
 *
 * Generated by src/system/kernel/Kernel_generate_mod_ids.py
 * 
 * @version {version}
 * @date {datetime.today().strftime('%Y-%m-%d')}
 * 
 * @copyright Copyright (c) 2020
 */

#ifndef H_KERNEL_MODULE_IDS_H
#define H_KERNEL_MODULE_IDS_H

/* -------------------------------------------------------------------------   
 * INCLUDES
 * ------------------------------------------------------------------------- */

/* Standard library includes */
#include <stdint.h>

/* -------------------------------------------------------------------------   
 * DEFINES
 * ------------------------------------------------------------------------- */

/**
 * @brief The number of bits to left shift the module IDs by.
 *
 * This shift converts a raw ID number into a value which can be ORed against 
 * to generate a full value of, for instance an Event ID.
 * 
 * The module ID for FDIR is 0x02, but it's shifted value is 0x800.
 */
#define KERNEL_MOD_ID_SHIFT (10)

/* -------------------------------------------------------------------------   
 * MODULE IDS
 * ------------------------------------------------------------------------- */

{newline.join([gen_mod_id_def(id_def) for id_def in defs])}

#endif /* H_KERNEL_MODULE_IDS_H */
'''

def gen_app_ids_file(defs, version, source_name):
    '''
    Given the definitions array generate the AppId header file string.
    '''
    newline = '\n'
    return f'''
/**
 * @ingroup kernel
 * @file Kernel_module_ids.h
 * @author Duncan Hamill (dh2g16@soton.ac.uk/duncanrhamill@googlemail.com)
 * @brief App IDs.
 *
 * Generated by src/system/kernel/Kernel_generate_mod_ids.py
 * 
 * @version {version}
 * @date {datetime.today().strftime('%Y-%m-%d')}
 * 
 * @copyright Copyright (c) 2020
 */

#ifndef H_KERNEL_APP_IDS_H
#define H_KERNEL_APP_IDS_H

/* -------------------------------------------------------------------------   
 * INCLUDES
 * ------------------------------------------------------------------------- */

/* Internal includes */
#include "system/kernel/Kernel_public.h"

/* -------------------------------------------------------------------------   
 * APP IDS
 * ------------------------------------------------------------------------- */

{newline.join([gen_app_id_def(id_def) for id_def in defs if id_def["group_name"] == "Applications"])}

#endif /* H_KERNEL_APP_IDS_H */
'''

if __name__ == '__main__':
    if len(sys.argv) == 2:
        generate_mod_ids(Path(sys.argv[1]))
    else:
        raise ValueError('Expected a path to the SSS_ConstantModuleIds.xlsx file')