#!/usr/bin/env python3

import subprocess
import re
from typing import Union, List
from dataclasses import dataclass


@dataclass
class HelpEntry:
    short: Union[str, None]
    long: Union[str, None]
    description: str
    has_arg: bool = False
    implicit_arg: Union[str, None] = None

    def to_markdown(self):
        output = ''

        if self.short is None:
            output += f'* ***{self.long}***'
        else:
            output += f'* ***{self.short}***, ***{self.long}***'

        if self.has_arg:
            if self.implicit_arg is None:
                output += ' *arg*'
            else:
                output += f' *arg (= {self.implicit_arg})*'

        output += '\n\n'
        output += f'  {self.description}'

        return output


def find_first_entry_line_idx(lines: List[str]):
    for idx, line in enumerate(lines):
        if re.match(r'^ {2}-.+', line):
            return idx
    else:
        return len(lines)


def read_help_lines(rampack_exec: str, command: str):
    result = subprocess.run([rampack_exec, command, '--help'], stdout=subprocess.PIPE)
    output = result.stdout.decode('utf-8')
    return output.splitlines()


def parse_help(rampack_exec: str, command: str):
    lines = read_help_lines(rampack_exec, command)
    first_entry = find_first_entry_line_idx(lines)
    lines = lines[first_entry:]

    help_entries = []
    for line in lines:
        match = re.match(r'^ {2}(..). (--[-0-9a-zA-Z]+) \[=arg\(=([^)\]]+)\)] +(.*)', line)
        if match:
            help_entries.append(HelpEntry(short=match.group(1), long=match.group(2), description=match.group(4),
                                          has_arg=True, implicit_arg=match.group(3)))
            continue

        match = re.match(r'^ {2}(..). (--[-0-9a-zA-Z]+) arg +(.*)', line)
        if match:
            help_entries.append(HelpEntry(short=match.group(1), long=match.group(2), description=match.group(3),
                                          has_arg=True))
            continue

        match = re.match(r'^ {2}(..). (--[-0-9a-zA-Z]+) +(.*)', line)
        if match:
            help_entries.append(HelpEntry(short=match.group(1), long=match.group(2), description=match.group(3)))
            continue

        match = re.match(r'^ +(.*)', line)
        if match:
            help_entries[-1].description += match.group(1)
            continue

    for i, entry in enumerate(help_entries):
        if entry.short == "  ":
            entry.short = None

    return help_entries


def implode_entries(help_entries: List[HelpEntry]):
    return '\n\n'.join([entry.to_markdown() for entry in help_entries])


def export_entries(help_entries: List[List[HelpEntry]], doc_path: str, modes: List[str]):
    with open(doc_path, 'r') as doc:
        doc_str = doc.read()

    for entries, mode in zip(help_entries, modes):
        compiled_entries = '[//]: # (This is automatically generated block, do not edit!!!)\n\n'
        compiled_entries += implode_entries(entries)

        start_str = '[//]: # (start {})'.format(mode)
        end_str = '[//]: # (end {})'.format(mode)

        start_index = doc_str.find(start_str)
        end_index = doc_str.find(end_str)

        if start_index != -1 and end_index != -1 and start_index < end_index:
            doc_str = doc_str[:start_index+len(start_str)] + "\n" + compiled_entries + "\n\n" + doc_str[end_index:]
        else:
            raise RuntimeError("Automatic options block for mode '{}' is missing in {}".format(mode, doc_path))

    with open(doc_path, 'w') as doc:
        doc.write(doc_str)


def main():
    doc_path = 'docs/operation-modes.md'
    rampack_exec = 'rampack'
    modes = ['casino', 'preview', 'shape-preview', 'trajectory']
    help_entries = []

    for mode in modes:
        help_entries.append(parse_help(rampack_exec, mode))

    export_entries(help_entries, doc_path, modes)

    print("Succesfully updated '{}'".format(doc_path))


if __name__ == '__main__':
    main()
