#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <vector>
using namespace std;

// DEFINES BEING USING IN THIS PARSER---------------------------------------------------------------------------------
#define TOTAL_CFG_VARIABLES 31
#define TOTAL_TERMINALS 24
#define TOTAL_CFG_RULES 60
#define MAX_LEN_CFG_RULE 10
//--------------------------------------------------------------------------------------------------------------------

// GLOBAL MEMORY FOR PARSER-------------------------------------------------------------------------------------------
int P[TOTAL_CFG_VARIABLES][TOTAL_TERMINALS] = { 0 };
int CFG_RULES_TABLE[TOTAL_CFG_RULES][MAX_LEN_CFG_RULE] = { 0 };
int error_count = 0;
struct tokentype {
    string tokentype;
    int tokentypeid;
};
tokentype tokenTypeArray[25] = {
    {"int",	1},
    {"float", 2},
    {",", 3},
    {"identifier", 4},
    {"for", 5},
    {"while", 6},
    {"(", 7},
    {"number", 8},
    {"if", 9},
    {"[", 10},
    {";",	11},
    {"else",	12},
    {":",	13},
    {"=", 14},
    {"<", 15},
    {">", 16},
    {"!", 17},
    {"+", 18},
    {"-", 19},
    {"*", 20},
    {"/", 21},
    {")",	22},
    {"]",	23},
    {"$",	24},
    {"Null", 25}
};
struct cfg_variable {
    string variable; int variableID; bool startVariable=false;
};
cfg_variable Variable[TOTAL_CFG_VARIABLES] = {
    {"Function", -1, true},
    {"ArgList", -2},
    {"ArgList’", -3},
    {"Arg", -4},
    {"Declaration", -5},
    {"Type", -6},
    {"IdentList", -7},
    {"IdentList’", -8},
    {"Stmt", -9},
    {"ForStmt", -10},
    {"OptExpr", -11},
    {"WhileStmt", -12},
    {"IfStmt", -13},
    {"ElsePart", -14},
    {"CompoundStmt", -15},
    {"CompoundStmt’", -16},
    {"StmtList’", -17},
    {"Expr", -18},
    {"Expr'",	-19},
    {"Rvalue", -20},
    {"Rvalue’", -21},
    {"Compare", -22},
    {"Compare’", -23},
    {"Compare’’", -24},
    {"Mag’", -25},
    {"Mag", -26},
    {"Mag’’", -27},
    {"Term’", -28},
    {"Term", -29},
    {"Term’’", -30},
    {"Factor", -31}
};
vector<pair<string, int>> input;
//--------------------------------------------------------------------------------------------------------------------

// FUNCTIONS---------------------------------------------------------------------------------------------------------- 
void ReadParsingTable(int P[TOTAL_CFG_VARIABLES][TOTAL_TERMINALS])
{
    int i = 0, j = 0, k = 0, len, last = 0;
    string num = "", line;
    ifstream infile("input/Parsing-table.csv");
    if (infile.is_open()) {
        while (getline(infile, line)) {
            k = 0, last = 0, j = 0;
            len = line.length();
            while (k != len) {
                if (line[k] == ',' || k == len - 1) {
                    P[i][j] = atoi(num.append(line, last, k).c_str());
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
void ReadCfgRulesTable(int CFG_RULES_TABLE[TOTAL_CFG_RULES][MAX_LEN_CFG_RULE])
{
    int i = 0, j = 0, k = 0, len, last = 0;
    string num = "", line;
    ifstream infile("input/CFG-Rules.csv");
    if (infile.is_open()) {
        while (getline(infile, line)) {
            k = 0, last = 0, j = 0;
            len = line.length();
            while (k != len) {
                if (line[k] == ',' || k == len - 1) {
                    CFG_RULES_TABLE[i][j] = atoi(num.append(line, last, k).c_str());
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
void DisplayParsingTable(int T[][TOTAL_TERMINALS])
{
    for (int ri = 0; ri < TOTAL_CFG_VARIABLES; ri++)
    {
        for (int ci = 0; ci < TOTAL_TERMINALS; ci++)
        {
            cout << T[ri][ci] << ' ';
        }
        cout << endl;
    }
}
void DisplayCfgRulesTable(int T[][MAX_LEN_CFG_RULE])
{
    for (int ri = 0; ri < TOTAL_CFG_RULES; ri++)
    {
        for (int ci = 0; ci < MAX_LEN_CFG_RULE; ci++)
        {
            cout << T[ri][ci] << ' ';
        }
        cout << endl;
    }
}
string convertToString(char* a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}
void ReadInput()
{
    ifstream fin("input/tokens.txt");
    char array[20];
    while (!fin.eof())
    {
        fin.ignore(1);
        fin.getline(array, 20, ',');
        fin.ignore(100, '\n');
        string tok = convertToString(array, strlen(array));
        int id;
        for (int i = 0; i < TOTAL_TERMINALS + 1; i++)
        {
            if (tokenTypeArray[i].tokentype == tok)
            {
                id = tokenTypeArray[i].tokentypeid; break;
            }
        }
        input.push_back({ tok,id });
    }
    for (int i = 0; i < TOTAL_TERMINALS + 1; i++)
    {
        if (tokenTypeArray[i].tokentype == "$")
        {
            input.push_back({tokenTypeArray[i].tokentype, tokenTypeArray[i].tokentypeid}); break;
        }
    }
}
bool IsStringParsable()
{
    stack<int> S;
    for (int i = 0; i < TOTAL_TERMINALS + 1; i++)
    {
        if (tokenTypeArray[i].tokentype == "$")
        {
            S.push(tokenTypeArray[i].tokentypeid); break;
        }
    }
    for (int i = 0; i < TOTAL_CFG_VARIABLES; i++)
    {
        if (Variable[i].startVariable == true)
        {
            S.push(Variable[i].variableID); break;
        }
    }
    int v_iter = 0;
    bool canParseTheTokens = false;
    while (!S.empty())
    {
        if (S.top() == input[v_iter].second)
        {
            S.pop();
            if (v_iter + 1 < input.size())
                ++v_iter;
        }
        else
        {
            int cfgRulesTableRow = P[abs(S.top()) - 1][input[v_iter].second - 1];
            if (cfgRulesTableRow == 777)
            {
                if (!S.empty())
                    S.pop();
                error_count++;
            }
            for (int c = MAX_LEN_CFG_RULE - 1; c >= 0; c--)
            {
                if (CFG_RULES_TABLE[cfgRulesTableRow][c] != 999 && CFG_RULES_TABLE[cfgRulesTableRow][c] != 0)
                {
                    if (CFG_RULES_TABLE[cfgRulesTableRow][c] == 25 && c == 0)// if null then pop from stack
                    {
                        if (!S.empty())
                            S.pop();
                    }
                    else
                    {
                        S.pop();
                        for (int c_inner = c; c_inner >= 0; c_inner--)
                            S.push(CFG_RULES_TABLE[cfgRulesTableRow][c_inner]);
                        break;
                    }
                }
            }
        }
    }
    return (S.empty() && v_iter == input.size() - 1); // STRING PARSING POSSIBILITY CHECK
}
// -------------------------------------------------------------------------------------------------------------------

// PARSER'S MAIN------------------------------------------------------------------------------------------------------
int main()
{
    ReadParsingTable(P);
    ReadCfgRulesTable(CFG_RULES_TABLE);
    ReadInput();
    bool result = IsStringParsable();
    if (result) // STRING PARSING POSSIBILITY CHECK
        cout << "Our parser can parse the string of given tokens with " << error_count << " errors.\n";
    else
        cout << "Our parser can not parse the string of given tokens with " << error_count << " errors.\n";
    
    return 0;
}
// -------------------------------------------------------------------------------------------------------------------