#include "headers/CalificationProgram.hpp"

namespace
{
constexpr size_t DEFAULT_REPORT_WIDTH = 85;

void print_usage()
{
    cerr << "Usage: calification_program --eval <course/lab> "
         << "[---report_widith <30-300>]\n";
}

bool is_report_width_flag(const string &flag)
{
    return flag == "---report_widith" or flag == "--report_widith" or
           flag == "---report_width" or flag == "--report-width";
}

size_t parse_report_width(const string &input)
{
    size_t read = 0;
    unsigned long width = 0;
    try
    {
        width = stoul(input, &read);
    }
    catch (const exception &)
    {
        throw invalid_argument("Report width must be an integer from 30 to 300");
    }
    if (read != input.size() or width < 30 or width > 300)
    {
        throw invalid_argument("Report width must be an integer from 30 to 300");
    }
    return static_cast<size_t>(width);
}
}

int main(int argc, char *argv[])
{
    if ((argc != 3 and argc != 5) or string(argv[1]) != "--eval" or
        (argc == 5 and !is_report_width_flag(argv[3])))
    {
        print_usage();
        return 1;
    }

    try
    {
        const string evaluation_name = argv[2];
        const size_t report_width = argc == 5
                ? parse_report_width(argv[4]) : DEFAULT_REPORT_WIDTH;
        CalificationProgram program(
                evaluation_name, resolve_lab_folder(evaluation_name),
                report_width);
        program.run();
    }
    catch (const exception &error)
    {
        cerr << "Error: " << error.what() << "\n";
        return 1;
    }
    
    return 0;
}
