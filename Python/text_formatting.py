import argparse
import sys


def format_text(input_file, output_file, line_length, paragraph_spaces):
    lines = input_file.readlines()
    lines = list(map(lambda word: word[:len(word)-(word[len(word) - 1] == '\n')], lines))
    print(lines)
    while lines[0] == '':
        lines.pop(0)
    paragraphs = [lines[0]]
    for i, line in enumerate(lines[1:]):
        if line != '':
            paragraphs[len(paragraphs) - 1] += ' ' + line
        elif lines[i - 1] != '':
            paragraphs.append('')
    for paragraph in paragraphs:
        words = paragraph.split()
        if len(words) == 0:
            continue
        new_line = ' ' * paragraph_spaces
        while len(words) != 0:
            if line_length < len(words[0]):
                print("Text contains too long word!")
                break
            elif line_length - len(new_line) > len(words[0]):
                new_line += words[0] + ' '
                words = words[1:]
            else:
                output_file.write(new_line + '\n')
                new_line = words[0] + ' '
                words = words[1:]
        output_file.write(new_line + '\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-i', '--input',
        type=argparse.FileType('r'),
        default=sys.stdin
    )
    parser.add_argument(
        '-o', '--output',
        type=argparse.FileType('w'),
        default=sys.stdout
    )
    parser.add_argument(
        '-l', '--line-length',
        type=int,
        default=60
    )
    parser.add_argument(
        '-p', '--paragraph-spaces',
        type=int,
        default=4
    )
    args = parser.parse_args()
    format_text(args.input, args.output, args.line_length, args.paragraph_spaces)


if __name__ == '__main__':
    main()