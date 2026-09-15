#include "headers/CalificationProgram.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3 or string(argv[1]) != "--eval")
    {
        cerr << "Usage: calification_program --eval <course/lab>\n";
        return 1;
    }

    try
    {
        const string evaluation_name = argv[2];
        CalificationProgram program(
                evaluation_name, resolve_lab_folder(evaluation_name));
        program.run();
    }
    catch (const exception &error)
    {
        cerr << "Error: " << error.what() << "\n";
        return 1;
    }
    
    return 0;
}
