#!/usr/bin/env python3

import re
import sys
import glob
import os
from typing import List
from collections import defaultdict
from dataclasses import dataclass


def strip_code_blocks(document: str) -> str:
    return re.sub(r'[\t ]*```[\w]*[\t ]*\n(.*?)\n\s*```', '', document, flags=re.DOTALL)


def process_section_names(matches: List[str]) -> List[str]:
    processed_matches = []
    for match in matches:
        match = match.lower()
        match = re.sub(r'\s+', '-', match)
        match = re.sub(r'[^a-zA-Z0-9_\-]+', '', match)
        processed_matches.append(match)

    return processed_matches


def process_duplicates(links: List[str]) -> List[str]:
    processed_links = []
    name_counts = defaultdict(int)

    for link in links:
        if name_counts[link] == 0:
            processed_links.append(link)
        else:
            processed_links.append(f"{link}-{name_counts[link]}")
        name_counts[link] += 1

    return processed_links


def parse_sections(document: str) -> List[str]:
    matches = re.findall(r'^\s*#+\s*(.*)\s*$', document, flags=re.MULTILINE)
    matches = process_section_names(matches)
    matches = process_duplicates(matches)
    return matches


def parse_a_tags(document: str) -> List[str]:
    return re.findall(r'<\s*a\s+id=["\']([^"\']*)["\']\s*>', document)


def parse_anchors(document: str) -> List[str]:
    # Strip Markdown code blocks, as they may contain line comments "# ..." which would match Markdown sections
    document = strip_code_blocks(document)
    return parse_sections(document) + parse_a_tags(document)


@dataclass
class Link:
    linenum: int
    filename: str
    anchor: str


def find_links(document: str) -> List[Link]:
    documents_lines = document.splitlines()
    links = []

    for linenum, line in enumerate(documents_lines):
        for filename, anchor in re.findall(r'\[[^]]*]\(([a-zA-Z0-9_\-.]*)#([a-zA-Z0-9_\-]+)\)', line):
            links.append(Link(linenum, filename, anchor))

    for linenum, line in enumerate(documents_lines):
        for filename in re.findall(r'\[[^]]*]\(([a-zA-Z0-9_\-.]*)\)', line):
            links.append(Link(linenum, filename, ""))

    return links


@dataclass(init=False)
class MarkdownFile:
    filename: str
    anchors: List[str]
    links: List[Link]

    def __init__(self, filename: str, document: str):
        self.filename = filename
        self.anchors = parse_anchors(document)
        self.links = find_links(document)


def parse_markdown_files(filenames: List[str]) -> dict[str, MarkdownFile]:
    markdown_files = {}
    for filename in filenames:
        with open(filename, "r") as file:
            markdown_files[filename] = MarkdownFile(filename, file.read())
    return markdown_files


def find_broken_links(to_verify: MarkdownFile, markdown_files: dict[str, MarkdownFile]) -> List[Link]:
    missing_links = []
    for link in to_verify.links:
        if link.filename == "":
            linked_file = to_verify
        else:
            if link.filename not in markdown_files:
                missing_links.append(link)
                continue

            linked_file = markdown_files[link.filename]

        if link.anchor != "":
            if link.anchor not in linked_file.anchors:
                missing_links.append(link)

    return missing_links


def are_links_ok(directory: str, markdown_files: dict[str, MarkdownFile]):
    anything_wrong = False
    for filename, markdown_file in markdown_files.items():
        print(f"{directory}/{filename}: ", end="")
        missing_links = find_broken_links(markdown_file, markdown_files)
        if len(missing_links) == 0:
            print("OK")
        else:
            anything_wrong = True
            print("some links are dead:")
            for missing_link in missing_links:
                print(f"  line {missing_link.linenum}: {missing_link.filename}#{missing_link.anchor}")

    return anything_wrong


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} [directory]")
        exit(1)

    directory = sys.argv[1]
    os.chdir(directory)
    markdown_filenames = glob.glob(f"*.md")
    if len(markdown_filenames) == 0:
        print(f"No Markdown files found in '{directory}/'")
        exit(1)

    markdown_files = parse_markdown_files(markdown_filenames)
    anything_wrong = are_links_ok(directory, markdown_files)
    if anything_wrong:
        exit(1)


if __name__ == "__main__":
    main()
