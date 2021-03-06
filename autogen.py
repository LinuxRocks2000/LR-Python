## Multi-purpose C++ autogenerator.
## Primarily used to convert C++ normie functions to C++ python functions
print("::AUTOGEN STARTED::")
inputFile = open("graphics.hpp")
outputFile = open("autogenerated.hpp", "w+")

stage = 0
rotaryLock = 0
fname = ""
argTypes = {} ## Arguments for a currently autogenning function
functions = {}
curArgType = ""
curArgName = ""
bracketLevel = 0

ctps = {
    "uint8_t": "b",
    "int": "h",
    "uint16_t": "H",
    "char*": "s"
}

for x in inputFile.read():
    if stage == 0:
        if x == 'v':
            rotaryLock = 1
            stage = 1
    elif stage == 1:
        if rotaryLock == 1 and x == "o":
            rotaryLock = 2
        elif rotaryLock == 2 and x == "i":
            rotaryLock = 3
        elif rotaryLock == 3 and x == "d":
            stage = 2
        else:
            rotaryLock = 0 ## The word isn't void.
            stage = 0
    elif stage == 2:
        if x != "(":
            if x != " ":
                fname += x
        else:
            print(fname)
            stage = 3
    elif stage == 3:
        if x == " " and len(curArgType) > 0: ## Yep, you can only have one word for types. Use typedef or #define.
            stage = 4
        elif x != " ":
            curArgType += x
    elif stage == 4:
        if x in [")", ","]:
            stage = 3
            if (curArgType in ctps):
                argTypes[curArgName] = curArgType
            curArgType = ""
            curArgName = ""
        else:
            curArgName += x
        if x == ")":
            stage = 5;
    elif stage == 5:
        if x == "{":
            bracketLevel += 1
        elif x == "}":
            bracketLevel -= 1
        elif bracketLevel == 0 and not x == " ": ## Won't happen until it has passed all of them, because the first character this starts on should always be space or opening bracket.
            stage = 0
            functions[fname] = argTypes
            fname = ""
            argTypes = {}

result = ""

for x in functions:
    result += "static PyObject* " + x + "(PyObject *self, PyObject *args){\n"
    for y in functions[x]:
        result += "\t" + functions[x][y] + " " + y + ";\n"

    result += "\tif (PyArg_ParseTuple(args, \"" + "".join([ctps[functions[x][i]] for i in functions[x]]) + '"'
    result += "".join([", &" + i for i in functions[x]])
    result += ")){\n"

    result += "\t\t" + x + "(";
    result += "".join([i + ", " for i in list(functions[x].keys())[:-1]])
    result += list(functions[x].keys())[-1];
    result += ");\n"

    result += "\t}\n\telse{\n"
    result += "\t\tprintf(\"Please check your arguments.\\n\");\n\t}\n"

    result += "\treturn PyLong_FromLong(0);\n}\n\n" ## Double \n makes it more clean. Will eventually return the id number of that object, for move operations.

result += "static PyMethodDef lrpython_functions[] = {\n"

for x in functions:
    result += '\t{"' + x + '", ' + x + ', METH_VARARGS, "Autogenerated function."},\n'

result += "\t{NULL, NULL, 0, NULL}\n};\n\n"

result += 'static PyModuleDef lrpython_module = {PyModuleDef_HEAD_INIT, "lrpython", NULL, -1, lrpython_functions, NULL, NULL, NULL, NULL};\n\n'
result += 'static PyObject* begin_lrpython_module(void){\n\treturn PyModule_Create(&lrpython_module);\n}\n'

print("::AUTOGEN FINISHED::")
outputFile.write(result);
