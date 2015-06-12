import sys
import argparse
import locale
import re
import os
from datetime import datetime


def get_last_modified_str(path: str) -> str:
    stat_time = os.stat(path).st_mtime
    utc_time = datetime.utcfromtimestamp(stat_time)
    utc_str = utc_time.strftime("%a, %d %b %Y %X GMT")
    return utc_str


def wipe_file(path: str):
    with open(path, "w") as filehandle:
        filehandle.truncate()


def convert_str_to_file_symbol(name: str) -> str:
    """Escape characters, which can't be used in cpp."""
    filesymbol = re.sub("[^a-zA-Z0-9_]", "_", name)
    return filesymbol


def generate_file(input_path: str, output_path: str, html_filepath: str):
    locale.setlocale(locale.LC_TIME, "C")

    with open(input_path, "r") as input_file, open(output_path, "a") as output_file:
        filesymbol = convert_str_to_file_symbol(html_filepath)
        filedir, filename = os.path.split(html_filepath)
        last_modified = get_last_modified_str(input_path)
        file_lines = []
        filesize = 0

        for line in input_file:
            filesize += len(line)
            file_lines.append(line)
        print(
            "Html_file htmlfile_{filesymbol} {{\n"
            "    \"{filename}\",\n"
            "    \"{filedir}\",\n"
            "    {filesize},\n"
            "    \"{last_modified}\",".format(filesymbol    = filesymbol,
                                             filename      = filename,
                                             filedir       = filedir,
                                             filesize      = filesize,
                                             last_modified = last_modified),
            file=output_file
        )
        for line in file_lines:
            escaped_line = line.replace("\\", "\\\\").replace("\"", "\\\"")
            # [:-1] to slice \n from ending of line
            print('    "{}\\n"'.format(escaped_line[:-1]), file=output_file)
        print("};\n", file=output_file)


def main():
    # args parsing
    parser = argparse.ArgumentParser(description='Generate resource file from html files from given folder. ')
    parser.add_argument("folder", help="folder, where source files are located")
    parser.add_argument("output_file", help="placement of generated resources' file")
    parser.add_argument("-f", dest="file_overwrite", action="store_true", default=False, help="overwrite file if exist")
    args = parser.parse_args()

    # args correctness checking
    if not os.path.isdir(args.folder):
        sys.exit("Error: given folder path isn't folder")
    elif os.path.isdir(args.output_file):
        sys.exit("Error: output file should be file, not a folder.")
    elif os.path.isfile(args.output_file) and not args.file_overwrite:
        sys.exit("Error: output file exists. Use '-f' to overwrite.")
    elif (not os.path.exists(args.output_file)) or (os.path.isfile(args.output_file) and args.file_overwrite):
        # main program

        wipe_file(args.output_file)
        dir_list = args.folder.split("/")
        for root, dirs, files in os.walk(args.folder):
            for file in files:
                root_list = root.split("/")

                # strip folder path from html folder path
                html_folder_list = root_list[len(dir_list):]

                # if list has elements
                if html_folder_list:
                    html_filepath = "/" + "/".join(root_list[len(dir_list):]) + "/" + file
                else:
                    html_filepath = "/" + file

                generate_file(root + "/" + file, args.output_file, html_filepath)


if __name__ == "__main__":
    main()
