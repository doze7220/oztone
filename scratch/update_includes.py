import os
import re
import glob

# 1. Fix ConfigManager.h includes
config_h_path = 'src/Config/ConfigManager.h'
with open(config_h_path, 'r', encoding='utf-8') as f:
    content = f.read()

content = re.sub(r'#include "Config/Config_([^"]+)\.h"', r'#include "Config_\1.h"', content)
# Ensure ConfigManager_DefaultIni.h is just included if needed (it is in .cpp)
with open(config_h_path, 'w', encoding='utf-8') as f:
    f.write(content)

# 2. Fix src/Config/Config_*.cpp includes
config_cpp_files = glob.glob('src/Config/Config_*.cpp')
for file in config_cpp_files:
    with open(file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    content = content.replace('#include "../ConfigManager.h"', '#include "ConfigManager.h"')
    content = content.replace('#include "../ConfigManager_DefaultIni.h"', '#include "ConfigManager_DefaultIni.h"')
    
    with open(file, 'w', encoding='utf-8') as f:
        f.write(content)

# 3. Fix src/*.cpp and src/*.h includes
src_files = glob.glob('src/*.cpp') + glob.glob('src/*.h')
for file in src_files:
    if os.path.basename(file) in ['ConfigManager.h', 'ConfigManager.cpp', 'ConfigManager_DefaultIni.h']:
        continue
    
    with open(file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # We want to replace exactly #include "ConfigManager.h"
    content = content.replace('#include "ConfigManager.h"', '#include "Config/ConfigManager.h"')
    
    with open(file, 'w', encoding='utf-8') as f:
        f.write(content)

print("Includes updated successfully.")
