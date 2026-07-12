import os

path = "D:/ozlab/oztone/src/Window.cpp"
with open(path, "r", encoding="utf-8") as f:
    lines = f.readlines()

new_lines = lines[:328] + lines[604:722] + lines[1040:]

with open(path, "w", encoding="utf-8") as f:
    f.writelines(new_lines)
