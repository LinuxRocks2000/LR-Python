#!/usr/bin/python
import os
import sys

#try:
#    os.mkdir("build")
#except:
#    print("Build directory exists! Cleaning build space.")
#    os.system("rm -rf build")
#    os.mkdir("build")
#for x, y, z in os.walk("libretro-common"):
#    for i in z:
#        l = i.split(".")
#        if len(l) > 1 and l[1] in ("c", "cpp", "hpp"): ## If it's a useful file
#            if not os.system("gcc -c -o build/" + i + ".o " + x + "/" + i + " -Ilibretro-common/include"):
#                print("Build Successful")
#            else:
#                print("Build Failed")
os.system("python3 autogen.py") ## Run the autogen script
if not os.system("g++ -c -o core.o core.cpp -fPIC -Ilibretro-common/include -I/usr/include/python3.9 -lpython3.9"):
    print("Built object file, building shared library")
else:
    print("Build failed!")
    sys.exit(1)
if not os.system("g++ -shared -o python_libretro.so core.o -lpython3.9"):
    print("Built core.so shared object.")
else:
    sys.exit(1)
