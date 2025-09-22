//
// Created by lmthelex on 21/09/2025.
//

#include "..\headers\read_functions.hpp"

ifstream open_file_read(const string &file_name)
{
    ifstream file(file_name, ios::in);

    if (!file.is_open())
    {
        cerr << "File " << file_name << " could not be opened" << endl;
    }

    return file;
}

ofstream open_file_write(const string &file_name)
{
    ofstream file(file_name, ios::out);

    if (!file.is_open())
    {
        cerr << "File " << file_name << " could not be opened" << endl;
    }

    return file;
}

