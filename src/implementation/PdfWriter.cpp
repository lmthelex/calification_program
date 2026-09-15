#include "../headers/PdfWriter.hpp"

namespace
{
string to_win_ansi(const string &utf8)
{
    string result;
    for (size_t index = 0; index < utf8.size();)
    {
        const unsigned char first = static_cast<unsigned char>(utf8[index]);
        unsigned int codepoint = '?';
        size_t length = 1;

        if (first < 0x80)
        {
            codepoint = first;
        }
        else if ((first & 0xE0) == 0xC0 and index + 1 < utf8.size())
        {
            codepoint = ((first & 0x1F) << 6) |
                    (static_cast<unsigned char>(utf8[index + 1]) & 0x3F);
            length = 2;
        }
        else if ((first & 0xF0) == 0xE0 and index + 2 < utf8.size())
        {
            codepoint = ((first & 0x0F) << 12) |
                    ((static_cast<unsigned char>(utf8[index + 1]) & 0x3F) << 6) |
                    (static_cast<unsigned char>(utf8[index + 2]) & 0x3F);
            length = 3;
        }
        else if ((first & 0xF8) == 0xF0 and index + 3 < utf8.size())
        {
            codepoint = ((first & 0x07) << 18) |
                    ((static_cast<unsigned char>(utf8[index + 1]) & 0x3F) << 12) |
                    ((static_cast<unsigned char>(utf8[index + 2]) & 0x3F) << 6) |
                    (static_cast<unsigned char>(utf8[index + 3]) & 0x3F);
            length = 4;
        }

        if (codepoint <= 0x7F or (codepoint >= 0xA0 and codepoint <= 0xFF))
        {
            result += static_cast<char>(codepoint);
        }
        else
        {
            switch (codepoint)
            {
                case 0x2013: result += static_cast<char>(0x96); break;
                case 0x2014: result += static_cast<char>(0x97); break;
                case 0x2018: result += static_cast<char>(0x91); break;
                case 0x2019: result += static_cast<char>(0x92); break;
                case 0x201C: result += static_cast<char>(0x93); break;
                case 0x201D: result += static_cast<char>(0x94); break;
                case 0x2022: result += static_cast<char>(0x95); break;
                case 0x2026: result += static_cast<char>(0x85); break;
                default: result += '?'; break;
            }
        }
        index += length;
    }
    return result;
}

vector<string> wrap_lines(const string &text, size_t report_width)
{
    vector<string> result;
    istringstream input(to_win_ansi(text));
    string line;

    while (getline(input, line))
    {
        if (!line.empty() and line.back() == '\r')
        {
            line.pop_back();
        }
        while (line.size() > report_width)
        {
            size_t split_at = line.rfind(' ', report_width);
            if (split_at == string::npos or split_at < report_width / 2)
            {
                split_at = report_width;
            }
            result.push_back(line.substr(0, split_at));
            const size_t next = line.find_first_not_of(' ', split_at);
            line = next == string::npos ? "" : "    " + line.substr(next);
        }
        result.push_back(line);
    }
    if (!text.empty() and text.back() == '\n')
    {
        result.emplace_back();
    }
    return result;
}

string escape_pdf_string(const string &line)
{
    string escaped;
    for (unsigned char character: line)
    {
        if (character == '(' or character == ')' or character == '\\')
        {
            escaped += '\\';
        }
        if (character >= 32 or character >= 128)
        {
            escaped += static_cast<char>(character);
        }
    }
    return escaped;
}

string page_stream(const vector<string> &lines, size_t begin, size_t end,
                   double font_size, double line_height)
{
    ostringstream stream;
    stream << fixed << setprecision(2) << "BT\n/F1 " << font_size
           << " Tf\n" << line_height << " TL\n36 756 Td\n";
    for (size_t index = begin; index < end; ++index)
    {
        stream << "(" << escape_pdf_string(lines[index]) << ") Tj\nT*\n";
    }
    stream << "ET\n";
    return stream.str();
}
}

void write_pdf(ostream &output, const string &text, size_t report_width)
{
    vector<string> lines = wrap_lines(text, report_width);
    if (lines.empty())
    {
        lines.emplace_back();
    }
    const double width = static_cast<double>(report_width);
    const double font_size = min(10.0, 540.0 / (0.6 * width));
    const double line_height = font_size + 2.0;
    const size_t lines_per_page = max(
            static_cast<size_t>(1), static_cast<size_t>(720.0 / line_height));
    const size_t page_count =
            (lines.size() + lines_per_page - 1) / lines_per_page;
    const size_t object_count = 3 + page_count * 2;
    vector<string> objects(object_count + 1);

    objects[1] = "<< /Type /Catalog /Pages 2 0 R >>";
    ostringstream pages;
    pages << "<< /Type /Pages /Kids [";
    for (size_t page = 0; page < page_count; ++page)
    {
        pages << 4 + page * 2 << " 0 R ";
    }
    pages << "] /Count " << page_count << " >>";
    objects[2] = pages.str();
    objects[3] = "<< /Type /Font /Subtype /Type1 /BaseFont /Courier "
                 "/Encoding /WinAnsiEncoding >>";

    for (size_t page = 0; page < page_count; ++page)
    {
        const size_t page_object = 4 + page * 2;
        const size_t content_object = page_object + 1;
        const size_t begin = page * lines_per_page;
        const size_t end = min(lines.size(), begin + lines_per_page);
        const string content = page_stream(
                lines, begin, end, font_size, line_height);

        ostringstream page_definition;
        page_definition << "<< /Type /Page /Parent 2 0 R "
                        << "/MediaBox [0 0 612 792] "
                        << "/Resources << /Font << /F1 3 0 R >> >> "
                        << "/Contents " << content_object << " 0 R >>";
        objects[page_object] = page_definition.str();

        ostringstream content_definition;
        content_definition << "<< /Length " << content.size()
                           << " >>\nstream\n" << content << "endstream";
        objects[content_object] = content_definition.str();
    }

    ostringstream document;
    document << "%PDF-1.4\n%\xE2\xE3\xCF\xD3\n";
    vector<size_t> offsets(object_count + 1, 0);
    for (size_t id = 1; id <= object_count; ++id)
    {
        offsets[id] = static_cast<size_t>(document.tellp());
        document << id << " 0 obj\n" << objects[id] << "\nendobj\n";
    }

    const size_t xref_offset = static_cast<size_t>(document.tellp());
    document << "xref\n0 " << object_count + 1 << "\n";
    document << "0000000000 65535 f \n";
    for (size_t id = 1; id <= object_count; ++id)
    {
        document << setw(10) << setfill('0') << offsets[id]
                 << " 00000 n \n";
    }
    document << setfill(' ') << "trailer\n<< /Size " << object_count + 1
             << " /Root 1 0 R >>\nstartxref\n" << xref_offset
             << "\n%%EOF\n";

    output << document.str();
    if (!output)
    {
        throw runtime_error("Could not write PDF data");
    }
}
