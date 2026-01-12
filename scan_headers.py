import os
import re

def scan_files(root_dir):
    extensions = ('.h', '.hpp', '.cpp')
    results = []

    # Regex for functions
    func_regex = re.compile(r"^\s*(?!(?:if|for|while|switch|return|else|delete|do|using|namespace|public|private|protected)\b)\s*(?:[\w<>:|*&]+\s+)*([\w:]+)\s*\([^)]*\)\s*(?:const\s*)?(?:override\s*)?(?:final\s*)?[{;]")
    class_regex = re.compile(r"^\s*(?:class|struct)\s+([a-zA-Z_][a-zA-Z0-9_]*)\b")

    for root, dirs, files in os.walk(root_dir):
        if "externals" in root or "generated" in root or ".git" in root:
            continue

        for file in files:
            if file.endswith(extensions):
                path = os.path.join(root, file)
                rel_path = os.path.relpath(path, os.getcwd())

                with open(path, "r", encoding="utf-8-sig", errors="ignore") as f:
                    try:
                        lines = f.readlines()
                    except:
                        continue

                for i, line in enumerate(lines):
                    stripped = line.strip()
                    if not stripped: continue
                    
                    # Search for functions
                    func_match = func_regex.search(line)
                    if func_match:
                        func_name = func_match.group(1)
                        if any(k in func_name for k in ["if", "for", "while", "main"]): continue
                        
                        documented = False
                        # Look back for comments
                        prev_idx = i - 1
                        while prev_idx >= 0:
                            prev_line = lines[prev_idx].strip()
                            if "/// <summary>" in prev_line or "/**" in prev_line:
                                documented = True
                                break
                            if not prev_line or prev_line.startswith("//") or prev_line.startswith("/*") or prev_line.startswith("*") or prev_line.startswith("["):
                                # Continue looking back through comments or empty lines
                                prev_idx -= 1
                                continue
                            break # Found non-comment code
                        
                        if not documented:
                            results.append(f"{rel_path}:{i+1}: {func_name}")
                            continue

                    # Search for classes/structs (only if not already documented by class_regex)
                    class_match = class_regex.search(line)
                    if class_match:
                        if stripped.endswith(";"): continue
                        class_name = class_match.group(1)
                        
                        documented = False
                        prev_idx = i - 1
                        while prev_idx >= 0:
                            prev_line = lines[prev_idx].strip()
                            if "/// <summary>" in prev_line or "/**" in prev_line:
                                documented = True
                                break
                            if not prev_line or prev_line.startswith("//") or prev_line.startswith("/*") or prev_line.startswith("["):
                                prev_idx -= 1
                                continue
                            break
                        
                        if not documented:
                            results.append(f"{rel_path}:{i+1}: {class_name}")

    return results

if __name__ == "__main__":
    import sys
    target = sys.argv[1] if len(sys.argv) > 1 else "."
    findings = scan_files(target)
    for f in findings:
        print(f)
