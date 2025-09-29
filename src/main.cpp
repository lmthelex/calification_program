#include "headers\Rubric.hpp"

int main()
{
    string lab = "lab03/";

    //create a rubric
    ifstream rubric_file;
    Rubric rubric;

    rubric_file = open_file_read("../meta_data/" + lab + "rubric.txt");
    rubric.read_rubric(rubric_file);

    /*
     * Execute to verify the base score of the rubric
    ofstream uwu("xd",ios::out);
    uwu.basic_ios<char>::rdbuf(std::cout.rdbuf());
    rubric.print(uwu, true);
    exit(1);
    */

    //get the paths inside ../data/labX/*
    try
    {
        for (const auto &entry: directory_iterator("../data/" + lab))
        {
            if (entry.is_directory())
            {
                //open the evaluated rubric file
                ifstream evaluated_rubric_file;
                ofstream final_evaluated_rubric;
                string student_path = entry.path().filename().string(), input_path, output_path;

                input_path = "../data/" + lab + "/" + student_path + "/" + student_path + "_rubric.txt";
                output_path = "../data/" + lab + "/" + student_path + "/" + student_path + "_final_rubric.txt";
                evaluated_rubric_file = open_file_read(input_path);
                final_evaluated_rubric = open_file_write(output_path);

                //create the final rubric for the student
                Rubric evaluated_rubric(rubric);
                evaluated_rubric.read_evaluated_rubric(evaluated_rubric_file);
                evaluated_rubric.print(final_evaluated_rubric, false);

                cout << setw(2) << floor(evaluated_rubric.get_achieved_score() + 0.5) <<
                    " - " << evaluated_rubric.get_achieved_score() << " - " << evaluated_rubric.get_student_name() << endl;
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
