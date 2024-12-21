import os

def create_class_files(directory, className):
    # Create header file
    header_file_content = f"""#pragma once

class {className} {{
public:
    {className}();
    ~{className}();
}};
"""
    header_file_path = os.path.join(directory, f"{className}.h")
    print(f"Creating header file at: {header_file_path}")
    with open(header_file_path, 'w') as header_file:
        header_file.write(header_file_content)
    print(f"Header file content:\n{header_file_content}")

    # Create source file
    source_file_content = f"""#include "{className}.h"

{className}::{className}() {{
    // Constructor implementation
}}

{className}::~{className}() {{
    // Destructor implementation
}}

"""
    source_file_path = os.path.join(directory, f"{className}.cpp")
    print(f"Creating source file at: {source_file_path}")
    with open(source_file_path, 'w') as source_file:
        source_file.write(source_file_content)
    print(f"Source file content:\n{source_file_content}")

if __name__ == "__main__":
    directory = "path/to/your/directory"  # ここにディレクトリパスを指定
    className = "YourClassName"  # ここにクラス名を指定
    create_class_files(directory, className)
