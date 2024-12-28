import sys
import os

# Create a class(c++ .h,.cpp)
directory = sys.argv[1]
className = sys.argv[2]

# Set default values for optional arguments
parentClass = os.getenv('PARENT_CLASS', '')
parentType = os.getenv('PARENT_TYPE', '') if parentClass else ''

# Create directory if it does not exist
if not os.path.exists(directory):
    os.makedirs(directory)

# Create header file
header_file_content = f"""#pragma once
{f'#include "{parentClass}.h"' if parentClass else ""}

class {className}{" : " + parentType + " " + parentClass if parentClass else ""} {{
public:
    {className}();
    ~{className}();
}};
"""

header_file_path = os.path.join(directory, f"{className}.h")
with open(header_file_path, 'w') as header_file:
    header_file.write(header_file_content)

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
with open(source_file_path, 'w') as source_file:
    source_file.write(source_file_content)
