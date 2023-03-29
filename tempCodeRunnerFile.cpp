        ostringstream ss;

        // Double the width of the first row
        ss << array[0][0] << "--------" << array[0][1] << "--------" << array[0][2] << endl;
        ss << "|       |        |" << endl;
        ss << "| " << array[1][0] << "----" << array[1][1] << "----" << array[1][2] << " |" << endl;
        ss << "| |           |  |" << endl;
        ss << "| |  " << array[2][0] << "--" << array[2][1] << "--" << array[2][2] << "  | |" << endl;
        ss << "| |  |      |  | |" << endl;
        ss << array[2][7] << "-" << array[1][7] << "-" << array[0][7] << "      " << array[0][3] << "-" << array[1][3] << "-" << array[2][3] << endl;
        ss << "| |  |      |  | |" << endl;
        ss << "| |  " << array[2][6] << "--" << array[2][5] << "--" << array[2][4] << "  | |" << endl;
        ss << "| |           |  |" << endl;
        ss << "| " << array[1][6] << "----" << array[1][5] << "----" << array[1][4] << " |" << endl;
        ss << "|       |        |" << endl;
        // Double the width of the last row
        ss << array[2][7] << "--------" << array[2][1] << "--------" << array[2][2] << endl;

        return ss.str();
    }