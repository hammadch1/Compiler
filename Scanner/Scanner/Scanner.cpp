#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>
using namespace std;

// DEFINES BEING USING IN THIS SCANNER--------------------------------------------------------------------------------
#define ERROR_STATE 49
#define max_no_of_transitions 24
#define t_states 51
//--------------------------------------------------------------------------------------------------------------------

// OUTPUT FILES-------------------------------------------------------------------------------------------------------
ofstream tokenf_out("output/tokens.txt");
ofstream errorf_out("output/errors.txt");
//--------------------------------------------------------------------------------------------------------------------

// GLOBAL MEMORY FOR SCANNER------------------------------------------------------------------------------------------
int TT[t_states][max_no_of_transitions] = { 0 };
size_t buffer_size = 1 << 20;// Buffer size 1 Megabyte (or any number you like)
char* buffer = new char[buffer_size];
struct tokentype {
    string tokentype;
    int tokentypeid;
};
tokentype tokentTypeArray[31] = {
    {"identifier",1},
    {"number",2},
    {"<", 3},
    {"<>", 4},
    {"<<", 5},
    {">>", 6},
    {">", 7},
    {"!=", 8},
    {"::", 9},
    {":=", 10},
    {":", 11},
    {"=+", 12},
    {"=>", 13},
    {"=<", 14},
    {"==", 15},
    {"=", 16},
    {"*", 17},
    {"+", 18},
    {"++", 19},
    {"/", 20},
    {"--", 21},
    {"-", 22},
    {"&&", 23},
    {"||", 24},
    {"%", 25},
    {"[", 26},
    {"{", 27},
    {"(", 28},
    {")", 29},
    {"}", 30},
    {"]", 31}
};
int FinalStatesorNot[51] = { 0,0,0,-1,0,0,0,0,0,0,-2,0,0,0,0,8,0,4,10,-11,0,15,-16,17,0,20,0,6,5,-3,19,-18,12,0,23,
    0,24,13,14,25,9,-22,26,27,28,29,30,31,0,-7,21 };
/*  fulfilled to assign negative numbers for the pointer-- case
    0 or non 0 tells if it's a final state or not. 0 means not final
    0th index is for state 1 and so on...*/
char Map[256];
int current_state = 0, last_state = 0; // state 1 in my case
vector<pair<long long, string>> hashes;
// -------------------------------------------------------------------------------------------------------------------


// FUNCTIONS---------------------------------------------------------------------------------------------------------- 
void ReadTransitionTable(int T[t_states][max_no_of_transitions])
{
    int i = 0, j = 0, k = 0, len, last = 0;
    string num = "", line;
    ifstream infile("input/Transition-table.csv");
    if (infile.is_open()){
        while (getline(infile, line)){
            k = 0, last = 0, j = 0;
            len = line.length();
            while (k != len) {
                if (line[k] == ',' || k == len - 1) {
                    T[i][j] = atoi(num.append(line, last, k).c_str());
                    num = "";
                    j++;
                    if (k != len - 1)
                        last = k + 1;
                }
                k++;
            }
            i++;
        }
        infile.close();
    }
    else cout << "Unable to open file";
}
void DisplayTransitionTable(int T[][max_no_of_transitions])
{
    for (int r = 0; r < t_states; r++)
    {
        for (int c = 0; c < max_no_of_transitions; c++)
        {
            cout << T[r][c] << " ";
        }
        cout << endl;
    }
}
int IsAcceptingState(int current_state)
{
    return FinalStatesorNot[current_state];
}
long long ComputeHash(string const& s) {
    const int p = 31;
    const int m = 1e9 + 9;
    long long hash_value = 0;
    long long p_pow = 1;
    for (char c : s) {
        hash_value = (hash_value + ((c - 'a') + 1) * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }
    return hash_value;
}
bool IsKeyword(string value)
{
    long long hashvalue = ComputeHash(value);
    for (int i = 0; i < hashes.size(); i++)
    {
        if (hashes[i].first == hashvalue)
            return true;
    }
    return false;
}
void ReadKeywordsList()
{
    ifstream kw_fin("input/keywords.txt");
    string kw = "";
    char s = '\0';
    while (kw_fin >> s)
    {
        if ((s == ',' || s == ' ') && kw.size()>0)
        {
            hashes.push_back({ ComputeHash(kw), kw});
            kw = "";
        }
        else kw += s;
    }
}
bool IsExponent(char buffer, int last_state)
{
    return buffer == 'E' && (last_state == 4 || last_state == 6);
}
void WriteTokens(int bytes)
{
    string tokentype = "", tokenvalue = "";
    int tokentypeid = 0;
    bool isTokenCreated = false, bufferAdded, endlcheck=false;
    int transition_col, error_line_no=1;
    for (int i = 0; i < bytes; i++)
    {
        bufferAdded = false;
        int ch = buffer[i];
        if (IsExponent(buffer[i], current_state))
        {
            transition_col = 22;//Exponent transition col
        }
        else transition_col = Map[ch];
        last_state = current_state;
        current_state = TT[current_state][transition_col] - 1;
        if (current_state == (ERROR_STATE - 1))//error state case
        {
            if (last_state == 1)// state 2 -> error state transition case i.e. keyword case
            {
                if (IsKeyword(tokenvalue))
                {
                    tokenf_out << "<"<<tokenvalue<<", " << tokenvalue << ">\n";
                    isTokenCreated = true;
                    tokenvalue = "";
                    current_state = last_state = 0;
                }
                else
                {
                    if (buffer[i] == '\n')
                    {
                        errorf_out << "<error, " << tokenvalue << "> at Line " << error_line_no << endl;
                        error_line_no++;
                        endlcheck = true;
                    }
                    else
                    {
                        if (tokenvalue == "")
                            tokenvalue += buffer[i];
                        errorf_out << "<error, " << tokenvalue << "> at Line " << error_line_no << endl;
                        i--;
                    }
                    tokenvalue = "";
                    current_state = last_state = 0;
                }
            }
            else
            {
                if (last_state == 7)
                    i--;
                if (buffer[i] == '\n')
                {
                    if(tokenvalue.size())
                        errorf_out << "<error, " << tokenvalue << "> at Line " << error_line_no << endl;
                    error_line_no++;

                }
                else
                {
                    if (tokenvalue == "")
                        tokenvalue += buffer[i];
                    errorf_out << "<error, " << tokenvalue << "> at Line " << error_line_no << endl;
                }
                tokenvalue = "";
                current_state = last_state = 0;
                continue;
            }
        }
        else if (tokentypeid = IsAcceptingState(current_state))
        {
            tokentype = tokentTypeArray[abs(tokentypeid) - 1].tokentype;
            if (bufferAdded == false && last_state != 2 && last_state != 4 && last_state != 9 && last_state != 6 && ((last_state == 12 && current_state != 29) || (last_state == 13 && current_state != 49) || (last_state == 14 && current_state != 48) || (last_state == 16 && current_state != 19) || (last_state == 20 && current_state != 22) || (last_state == 0 && current_state == 23) || (last_state == 24 && current_state != 31) || (last_state == 0 && current_state == 25) || (last_state == 26 && current_state != 41) || (last_state == 33 && current_state != 48) || (last_state == 35 && current_state != 48) || (last_state == 0 && current_state == 39) || (current_state >= 42 && current_state <= 47)))
                tokenvalue += buffer[i];
            tokenf_out << "<" << tokentype << ", " << tokenvalue << ">\n";
            tokenvalue = "";
            current_state = last_state = 0;
            if (tokentypeid < 0)
            {
                i--; continue;
            }
            isTokenCreated = true;
        }
        if (buffer[i] == '\n' && endlcheck==false)
        {
            error_line_no++; tokenvalue = "";
            current_state = last_state = 0;
        }
        if (Map[ch] != 23 && isTokenCreated == false)
        {
            tokenvalue += buffer[i];
            bufferAdded = true;
        }
        endlcheck=isTokenCreated = false;
    }
}
void InitializeMapArray()
{
    // initialization of Map array
    for (int i = 0; i < 256; i++)
    {
        if (i >= 65 && i <= 90 || i >= 97 && i <= 122)
            Map[i] = 0;
        else if (i == 95)
            Map[i] = 1;
        else if (i >= 48 && i <= 57)
            Map[i] = 2;
        else if (i == 60)
            Map[i] = 3;
        else if (i == 62)
            Map[i] = 4;
        else if (i == 33)
            Map[i] = 5;
        else if (i == 58)
            Map[i] = 6;
        else if (i == 61)
            Map[i] = 7;
        else if (i == 42)
            Map[i] = 8;
        else if (i == 43)
            Map[i] = 9;
        else if (i == 47)
            Map[i] = 10;
        else if (i == 45)
            Map[i] = 11;
        else if (i == 38)
            Map[i] = 12;
        else if (i == 124)
            Map[i] = 13;
        else if (i == 37)
            Map[i] = 14;
        else if (i == 91)
            Map[i] = 15;
        else if (i == 123)
            Map[i] = 16;
        else if (i == 40)
            Map[i] = 17;
        else if (i == 41)
            Map[i] = 18;
        else if (i == 125)
            Map[i] = 19;
        else if (i == 93)
            Map[i] = 20;
        else if (i == 46)
            Map[i] = 21;
        else
            Map[i] = 23;
    }
}
// -------------------------------------------------------------------------------------------------------------------

// SCANNER'S MAIN-----------------------------------------------------------------------------------------------------
int main()
{   
    char file_decision = '\0';
    cout << "Hello, Tester of Lexical Analyzer!!!\nDo you want to give me a source c++ code test file and if no then you"
        " will be using the attached source c++ code test file ? (Y/n)\n";
    cin >> file_decision;
    string inputFileName;
    //INPUT FILES-----------------------------------------------------------------------------------------------------
    ifstream fin;
    if (file_decision == 'Y')
    {
        cout << "File Name: ";
        cin >> inputFileName;
        fin.open("input/"+ inputFileName);
    }
    else if (file_decision == 'n')
        fin.open("input/test-source.txt");
    ReadTransitionTable(TT);
    ReadKeywordsList();
    //----------------------------------------------------------------------------------------------------------------
    InitializeMapArray();
    while (fin)
    {
        fin.read(buffer, buffer_size);// Try to read next chunk of data
        streamsize count = fin.gcount();// Get the number of bytes actually read
        if (!count)
            break;// If nothing has been read, break
        // Do whatever you need with first count bytes in the buffer
        WriteTokens(count);
        // ...
    }
    fin.close();
    tokenf_out.close();
    cout << "tokens.txt has been generated in output folder\n";
    errorf_out.close();
    cout << "errors.txt has been generated in output folder\n";
    delete[] buffer;
}
// -------------------------------------------------------------------------------------------------------------------