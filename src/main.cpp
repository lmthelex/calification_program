#include "headers\Rubric.hpp"

int main()
{
    //open rubric file
    ifstream rubric_file;

    rubric_file = open_file_read("../meta_data/rubric.txt");

    //open the base rubric
    Rubric rubric;
    rubric.read_rubric(rubric_file);

    //get the paths inside ../data/*
    try
    {
        for (const auto &entry: directory_iterator("../data"))
        {
            if (entry.is_directory())
            {
                //open the evaluated rubric file
                ifstream evaluated_rubric_file;
                ofstream final_evaluated_rubric;
                string student_path = entry.path().filename().string(), input_path, output_path;

                input_path = "../data/" + student_path + "/" + student_path + "_rubric.txt";
                output_path = "../data/" + student_path + "/" + student_path + "_final_rubric.txt";
                evaluated_rubric_file = open_file_read(input_path);
                final_evaluated_rubric = open_file_write(output_path);

                //create the final rubric for the student
                Rubric evaluated_rubric(rubric);
                evaluated_rubric.read_evaluated_rubric(evaluated_rubric_file);
                evaluated_rubric.print(final_evaluated_rubric);
            }
        }
    }
    catch (const filesystem_error &e)
    {
        cerr << "Filesystem error: " << e.what() << endl;
    }
    catch (const exception &e)
    {
        cerr << "General error: " << e.what() << endl;
    }

    return 0;
}
