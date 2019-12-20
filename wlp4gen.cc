#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <utility>
using namespace std;

// current procedure we are in
string procName;

// A class that stores the parse tree
class Tree {
 public:
    string rule; // current rule 
    string type; // type at the root of the tree
    vector<string> tokens; // list of tokens in rule
    vector<Tree> children; // list of child nodes
};

// An exception class thrown when an error is encountered 
// while constructing the symbol table
class DeclarationFailure {
    string message;

  public:
    DeclarationFailure(string message): message(move(message)) {}
    const string &what() const { return message; }
};

// Return a list of all terminals
vector<string> getTerminals() {
    vector<string> terminals;
    terminals.push_back("BOF");
    terminals.push_back("BECOMES");
    terminals.push_back("COMMA");
    terminals.push_back("ELSE");
    terminals.push_back("EOF");
    terminals.push_back("EQ");
    terminals.push_back("GE");
    terminals.push_back("GT");
    terminals.push_back("ID");
    terminals.push_back("IF");
    terminals.push_back("INT");
    terminals.push_back("LBRACE");
    terminals.push_back("LE");
    terminals.push_back("LPAREN");
    terminals.push_back("LT");
    terminals.push_back("MINUS");
    terminals.push_back("NE");
    terminals.push_back("NUM");
    terminals.push_back("PCT");
    terminals.push_back("PLUS");
    terminals.push_back("PRINTLN");
    terminals.push_back("RBRACE");
    terminals.push_back("RETURN");
    terminals.push_back("RPAREN");
    terminals.push_back("SEMI");
    terminals.push_back("SLASH");
    terminals.push_back("STAR");
    terminals.push_back("WAIN");
    terminals.push_back("WHILE");
    terminals.push_back("AMP");
    terminals.push_back("LBRACK");
    terminals.push_back("RBRACK");
    terminals.push_back("NEW");
    terminals.push_back("DELETE");
    terminals.push_back("NULL");
    return terminals;
}

// Return a vector containing each symbol in str that is separated by
// a single space
vector<string> separateSymbols(const string s) {
    vector<string> result;
    istringstream iss(s);
    for(string s; iss >> s; ) {
        result.emplace_back(s);
    }
    return result;
}

// Given rules which are a list of strings in the wlp4i format and terminals
// which is the list of all terminals of wlp4. Return a Tree whose root is
// the rules[index]
Tree buildParseTree(const vector<string> &rules, int &index, const vector<string> &terminals) {
    string rule;
    Tree t;
    vector<string> tokens;

    // Base case
    if (index < 0 || index >= rules.size()) {
        return t;
    }
    // Read the rule at index which is the root of t
    rule = rules[index];
    index += 1;
    tokens = separateSymbols(rule);
    t.rule = rule;
    t.tokens = tokens;
    t.type = "unset";

    // Rule goes to empty string
    if (t.tokens.size() <= 1) {
        return t;
    }
    // Terminal (leaf)
    if (find(terminals.begin(), terminals.end(), tokens[0]) != terminals.end()) {
        return t;
    }
    // Set the children of the root node
    for (int i = 1; i <= t.tokens.size() - 1; i++) {
        Tree child = buildParseTree(rules, index, terminals);
        t.children.push_back(child);
    }
    return t;
}

// Print symbolTable:
// - procedure name + procedure signature:
// - list of ID + type (only when procedure is wain)
void printSymbolTable(map<string, pair<vector<string>, map<string, pair<string, int>>>> symbolTable) {
    int counter = 0;
    for (auto const& proc : symbolTable) {
        if (counter != 0) {
            cerr << endl;
        }
        pair<vector<string>, map<string, pair<string, int>>> pair1;
        vector<string> sig;
        map<string, pair<string, int>> decls;
        pair<string, int> idLoc;
        pair1 = proc.second;
        sig = pair1.first;
        decls = pair1.second;

        // output procedure name + signature
        cerr << proc.first;
        for (auto &s : sig) {
            cerr << " " << s;
        }
        cerr << endl;
        for (auto const& dcl : decls) {
            idLoc = dcl.second;
            cerr << dcl.first << " " << idLoc.first << " " << idLoc.second << endl; // declaration   
        }
        counter++;
    }
}

// t.rule is arglist -> ...
int findNumArguments(Tree t) {
    if (t.tokens[0] != "arglist") {
        return 0; // something is wrong
    }
    int numArg = 0;
    if (t.rule == "arglist expr") { // arglist → expr
        numArg += 1;
    } else { // arglist → expr COMMA arglist
        numArg += 1;
        numArg += findNumArguments(t.children[2]);
    }
    return numArg;
}

// Given the complete parse tree, construct the symbol table 
// where st is a map of declarations so far.
// Output any declaration and scope errors to stderr.
void buildSymbolTable(
    map<string, pair<vector<string>, map<string, string>>> &symbolTable, const Tree &t, vector<string> &procNames, vector<string> &sig, map<string, string> &st) {
    // map<string, pair<vector<string>, map<string, string>>> symbolTable;
    map<string, pair<vector<string>, map<string, string>>> symbolTableTemp;
    // string procName = "";
    string name = "";
    string type = "";

    // Procedure declaration: wain
    if (t.tokens[0] == "main") {
        procName = "wain";

        // Check for duplicate procedure declaration
        if (find(procNames.begin(), procNames.end(), procName) != procNames.end()) {
            throw DeclarationFailure("ERROR: duplicate procedure declaration for \""+ procName + "\".");
        }
        procNames.push_back(procName);
        st.clear();
        sig.clear();
        if(t.children[3].children[0].rule == "type INT") sig.push_back("int");
        else sig.push_back("int*");
        if(t.children[5].children[0].rule == "type INT") sig.push_back("int");
        else sig.push_back("int*");
        symbolTable[procName] = make_pair(sig, st);
    }
    // Procedure declaration: not wain
    else if (t.tokens[0] == "procedure") {
        procName = t.children[1].tokens[1];

        // Check for duplicate procedure declaration
        if (find(procNames.begin(), procNames.end(), procName) != procNames.end()) {
            throw DeclarationFailure("ERROR: duplicate procedure declaration for \""+ procName + "\".");
        }
        procNames.push_back(procName);
        st.clear();
        sig.clear();
        symbolTable[procName] = make_pair(sig, st);
    }
    // Variable declarations
    else if (t.rule == "dcl type ID") {
        name = t.children[1].tokens[1];
        if (st.find(name) != st.end()) { // Duplicate declaration
            throw DeclarationFailure("ERROR: duplicate declaration for \""+ name + "\".");
        }
        if (t.children[0].rule == "type INT") type = "int";
        else type = "int*";
        st[name] = type;
        symbolTable[procName] = make_pair(sig, st);
    }
    // Using variables
    else if (t.rule == "factor ID" || t.rule == "lvalue ID") {
        name = t.children[0].tokens[1];
        if (st.find(name) == st.end()) { // Missing declaration
            throw DeclarationFailure("ERROR: missing declaration for \""+ name + "\".");
        }
    }
    // Parameter declarations
    else if (t.rule == "paramlist dcl" || t.rule == "paramlist dcl COMMA paramlist") {
        if(t.children[0].children[0].rule == "type INT") sig.push_back("int");
        else sig.push_back("int*");
        symbolTable[procName] = make_pair(sig, st);
    }
    // Function call
    else if (t.rule == "factor ID LPAREN RPAREN" || t.rule == "factor ID LPAREN arglist RPAREN") {
        name = t.children[0].tokens[1];
        vector<string> signature;
        map<string, string> varDecls;
        // Check if procedure ID is declared 
        if (find(procNames.begin(), procNames.end(), name) == procNames.end()) {
            throw DeclarationFailure("ERROR: missing declaration for procedure \""+ name + "\".");
        }
        // Check if number of arguments matches signature of procedure ID
        signature = (symbolTable.find(name)->second).first;
        if (t.rule == "factor ID LPAREN RPAREN") {
            if (signature.size() != 0) {
                throw DeclarationFailure("ERROR: missing argument when calling procedure \""+ name + "\".");
            }
        }
        else {
            int numArguments = findNumArguments(t.children[2]);
            if (numArguments != signature.size()) {
                throw DeclarationFailure("ERROR: number of arguments does not match procedure \""+ name + "\".");
            }
        }
        // Check if the procedure ID is declared as a variable in current procedure
        varDecls = (symbolTable.find(procName)->second).second;
        if (varDecls.find(name) != varDecls.end()) {
            throw DeclarationFailure("ERROR: cannot call procedure \""+ name + "\", it refers to a variable.");
        }
    }
    // procedure ends
    else if (t.rule == "RETURN return") {
        if (procName != "") {
            symbolTable[procName] = make_pair(sig, st);
        } else {
            cerr << "something's wrong" << endl;
        }
    }

    // Traverse through all children
    for (const auto &child : t.children) {
        buildSymbolTable(symbolTable, child, procNames, sig, st);
    }
}

// t.rule is arglist -> ...
// Return the a list of types within arglist
// vector<string> findArgumentType(Tree t, const map<string, pair<vector<string>, map<string, string>>> &symbolTable) {
//     vector<string> types;
//     if (t.tokens[0] != "arglist") {
//         return types; // something is wrong
//     } else if (t.rule == "arglist expr") { // arglist → expr
//         types.push_back(setType(t.children[0], symbolTable));
//     } else { // arglist → expr COMMA arglist
//         string type = setType(t.children[0], symbolTable);
//         types = findArgumentType(t.children[2], symbolTable);
//         types.insert(types.begin(), type);
//     }
//     return types;
// }


// Return and set the "type" field for t if its root is
// expr or lvalue. Output any type error.
string setType(Tree &t, const map<string, pair<vector<string>, map<string, string>>> &symbolTable) {
    map<string, string> varDecls;
    string name;
    string type;
    string typel;
    string typer;

    // Set t.type depending on the rule it stores
    if (t.children.size() == 0) {
        if (t.tokens[0] == "ID") {
            name = t.tokens[1];
            varDecls = (symbolTable.find(procName)->second).second;
            type = varDecls.at(name);
        } else if (t.tokens[0] == "NUM") {
            type = "int";
        } else if (t.tokens[0] == "NULL") {
            type = "int*";
        }
    } else if (t.rule == "expr term") {
        type = setType(t.children[0], symbolTable);
    } else if (t.rule == "expr expr PLUS term") {
        typel = setType(t.children[0], symbolTable);
        typer = setType(t.children[2], symbolTable);
        if (typel == "int" && typer == "int") {
            type = "int";
        } else if (typel == "int*" && typer == "int") {
            type = "int*";
        } else if (typel == "int" && typer == "int*") {
            type = "int*";
        } else {
            throw DeclarationFailure("ERROR: cannot add two variables with type int*.");
        }
    } else if (t.rule == "expr expr MINUS term") {
        typel = setType(t.children[0], symbolTable);
        typer = setType(t.children[2], symbolTable);
        if (typel == "int" && typer == "int") {
            type = "int";
        } else if (typel == "int*" && typer == "int") {
            type = "int*";
        } else if (typel == "int*" && typer == "int*") {
            type = "int";
        } else {
            throw DeclarationFailure("ERROR: cannot subtract an int* from int.");
        }
    } else if (t.rule == "term factor") {
        type = setType(t.children[0], symbolTable);
    } else if (t.rule == "term term STAR factor") {
        typel = setType(t.children[0], symbolTable);
        typer = setType(t.children[2], symbolTable);
        if (typel == "int" && typer == "int") {
            type = "int";
        } else {
            throw DeclarationFailure("ERROR: cannot multiply between type " + typel + " and " + typer + ".");
        }
    } else if (t.rule == "term term SLASH factor") {
        typel = setType(t.children[0], symbolTable);
        typer = setType(t.children[2], symbolTable);
        if (typel == "int" && typer == "int") {
            type = "int";
        } else {
            throw DeclarationFailure(
                "ERROR: cannot divide between type " + typel + " and " + typer + ".");
        }
    } else if (t.rule == "term term PCT factor") {
        typel = setType(t.children[0], symbolTable);
        typer = setType(t.children[2], symbolTable);
        if (typel == "int" && typer == "int") {
            type = "int";
        } else {
            throw DeclarationFailure(
                "ERROR: cannot % between type " + typel + " and " + typer + ".");
        }
    } else if (t.rule == "factor ID" || t.rule == "lvalue ID") {
        type = setType(t.children[0], symbolTable);
    } else if (t.rule == "factor NUM") {
        type = "int";
    } else if (t.rule == "factor NULL") {
        type = "int*";
    } else if (t.rule == "factor LPAREN expr RPAREN" || t.rule == "lvalue LPAREN lvalue RPAREN") {
        type = setType(t.children[1], symbolTable);
    } else if (t.rule == "factor AMP lvalue") {
        typer = setType(t.children[1], symbolTable);
        if (typer == "int") {
            type = "int*";
        } else {
            throw DeclarationFailure("ERROR: cannot place & before an int*.");
        }
    } else if (t.rule == "factor STAR factor" || t.rule == "lvalue STAR factor") {
        typer = setType(t.children[1], symbolTable);
        if (typer == "int*") {
            type = "int";
        } else {
            throw DeclarationFailure("ERROR: cannot dereference an int.");
        }
    } else if (t.rule == "factor NEW INT LBRACK expr RBRACK") {
        typer = setType(t.children[3], symbolTable);
        if (typer == "int") {
            type = "int*";
        } else {
            throw DeclarationFailure("ERROR: expr has type int * in new int[expr].");
        }
    } else if (t.rule == "factor ID LPAREN RPAREN") {
        type = "int";
    } else if (t.rule == "factor ID LPAREN arglist RPAREN") {
        name = t.children[0].tokens[1];
        vector<string> signature = (symbolTable.find(name)->second).first;
        // Get argument types
        vector<string> argument;
        Tree argTree = t.children[2];
        string argType;
        while(true) {
            argType = setType(argTree.children[0], symbolTable);
            argument.push_back(argType);
            if (argTree.tokens.size() == 2) break;
            else argTree = argTree.children[2];
        }
        // vector<string> argument = findArgumentType(t.children[2], symbolTable);
        if (signature.size() != argument.size()) {
            throw DeclarationFailure("ERROR: invalid number of arguments supplied for procedure. Expected: " 
                + to_string(signature.size()) + " but supplied: " + to_string(argument.size()) + ".");
        }
        for (int i = 0; i < signature.size(); i++) {
            if (signature[i] != argument[i]) {
                throw DeclarationFailure("ERROR: invalid arguments type for parameter " + to_string(i) + " Expected: " 
                    + signature[i] + " but supplied: " + argument[i] + ".");
            }
        }
        type = "int";
    } else if (t.tokens[0] == "main") {
        procName = "wain";
        string dcl2Rule = t.children[5].children[0].rule;
        string exprType = setType(t.children[11], symbolTable);
        if (dcl2Rule != "type INT") {
            throw DeclarationFailure("ERROR: 2nd argument for wain is invalid. Expected: int");
        }
        if (exprType != "int") {
            throw DeclarationFailure("ERROR: invalid return type for wain. Expected: int");
        }
    } else if (t.tokens[0] == "procedure") {
        string exprType = setType(t.children[9], symbolTable);
        procName = t.children[1].tokens[1];
        if (exprType != "int") {
            throw DeclarationFailure("ERROR: invalid return type for procedure " + procName + ". Expected: int.");
        }
    } else if (t.rule == "statement lvalue BECOMES expr SEMI") {
        typel = setType(t.children[0], symbolTable);
        typer = setType(t.children[2], symbolTable);
        if (typel != typer) {
            throw DeclarationFailure("ERROR: cannot assign use assignment between " + typel + " and " + typer + ".");
        }
    } else if (t.rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        typel = setType(t.children[2], symbolTable);
        if (typel != "int") {
            throw DeclarationFailure("ERROR: cannot print an int*.");
        }
    } else if (t.rule == "statement DELETE LBRACK RBRACK expr SEMI") {
        typel = setType(t.children[3], symbolTable);
        if (typel != "int*") {
            throw DeclarationFailure("ERROR: cannot delete[] an int.");
        }
    } else if (t.rule == "test expr EQ expr" || t.rule == "test expr NE expr" || t.rule == "test expr LT expr" || t.rule == "test expr LE expr" || t.rule == "test expr GE expr" || t.rule == "test expr GT expr") {
        typel = setType(t.children[0], symbolTable);
        typer = setType(t.children[2], symbolTable);
        if (typel != typer) {
            throw DeclarationFailure("ERROR: cannot compare between " + typel + " and " + typer + ".");
        }
    } else if (t.rule == "dcls dcls dcl BECOMES NUM SEMI") {
        Tree typeTree = t.children[1].children[0];
        if (typeTree.rule != "type INT") {
            throw DeclarationFailure("ERROR: cannot assign a number to an int*, ID: " + t.children[1].children[1].tokens[1] + ".");
        }
    } else if (t.rule == "dcls dcls dcl BECOMES NULL SEMI") {
        Tree typeTree = t.children[1].children[0];
        if (typeTree.rule != "type INT STAR") {
            throw DeclarationFailure("ERROR: cannot assign NULL to an int, ID: " + t.children[1].children[1].tokens[1] + ".");
        }
    }
    t.type = type;
    return type;
}

// Traverse through t, and call setType on every expr, lvalue, statement and test node
void findExprSetType (Tree &t, const map<string, pair<vector<string>, map<string, string>>> &symbolTable) {
    
    if (t.tokens[0] == "main") {
        procName = "wain";
    } else if (t.tokens[0] == "procedure") {
        procName = t.children[1].tokens[1];
    }
    if (t.tokens[0] == "expr" || t.tokens[0] == "lvalue" || 
    t.tokens[0] == "test" || t.tokens[0] == "statement" ||
    t.tokens[0] == "dcls" || t.tokens[0] == "main" ||
    t.tokens[0] == "procedure") {
        setType(t, symbolTable);
    }
    for (auto &child : t.children) {
        findExprSetType(child, symbolTable);
    }
}

// ----------------------------------------- A9/A10 -------------------------------------------

// Add a location firld to the original symbolTable
map<string, pair<vector<string>, map<string, pair<string, int>>>> addLoc(
    const map<string, pair<vector<string>, map<string, string>>> &symbolTable) {

    map<string, pair<vector<string>, map<string, pair<string, int>>>> symbolTableNew;
    int offset = -1; // temporary value

    for (auto const& proc : symbolTable) {
        string procedure = proc.first;
        pair<vector<string>, map<string, string>> procTable = proc.second;
        vector<string> sig = procTable.first;
        map<string, string> decls = procTable.second;
        pair<vector<string>, map<string, pair<string, int>>> newProcTable;
        map<string, pair<string, int>> newDecls;

        for (auto const& dcl : decls) {
            string id = dcl.first;
            string type = dcl.second;
            pair<string, int> typeLocation = make_pair(type, offset);
            newDecls[id] = typeLocation;
        }
        newProcTable = make_pair(sig, newDecls);
        symbolTableNew[procedure] = newProcTable;
    }
    return symbolTableNew;
}

// Update mips to include the mips starter code
string prologue() {
    string mips;
    mips += ".import print\n";
    mips += ".import init\n";
    mips += ".import new\n";
    mips += ".import delete\n";
    mips += "lis $4\n";
    mips += ".word 4\n";
    mips += "lis $11\n";
    mips += ".word 1\n";
    mips += "lis $10\n";
    mips += ".word print\n";
    mips += "sub $29, $30, $4\n";
    mips += "sub $30, $30, $4\n";
    mips += "sw $1, 0($30)\n";
    mips += "sub $30, $30, $4\n";
    mips += "sw $2, 0($30)\n";
    mips += "sub $30, $30, $4\n";
    return mips;
}

// Update mips to include the mips ending code
string epilogue() {
    string mips;
    mips += "add $30, $29, $4\n";
    mips += "jr $31\n";
    return mips;
}

// Add number of arg * 4 to each entry in the symbolTable for procedure procName
void updateSymbolTable(map<string, pair<vector<string>, map<string, pair<string, int>>>> &symbolTableNew, string procedureName) {
    pair<vector<string>, map<string, pair<string, int>>> &procTable = symbolTableNew.at(procedureName);
    map<string, pair<string, int>> &dcls = procTable.second;
    int numArg = procTable.first.size();
    int argTimes4 = numArg * 4;

    for (auto& dcl : dcls) {
        int &offset = dcl.second.second;
        offset += argTimes4;
    }
}

// Return mips to include the mips assembly code equivalent to the
// parse tree t. Update symbolTable's location field if necessary.
// - paramCount keeps a counter for the number of parameters.
// - offset keeps track of the offset from $29 for local variables
// - X, Y, Z are counters for if, while, delete statements respectively.
string generateMips(map<string, pair<vector<string>, map<string, pair<string, int>>>> &symbolTableNew, const Tree &t, int &paramCount, int &offset, int &X, int &Y, int &Z) {
    string mips = "";
    if (t.rule == "procedures procedure procedures") {
        mips += generateMips(symbolTableNew, t.children[1], paramCount, offset, X, Y, Z);
        mips += generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        return mips;
    } else if (t.tokens[0] == "main") {
        procName = "wain";
        mips += prologue();
        // Set $2 to zero if loader is mips.twoints
        if (t.children[3].children[0].rule == "type INT") {
            mips += "add $2, $0, $0\n";
        }
        // Call init
        mips += "wain:\n";             // label
        mips += "sw $31, 0($30)\n";    // save ($31)
        mips += "sub $30, $30, $4\n";
        mips += "lis $31\n";
        mips += ".word init\n";
        mips += "jalr $31\n";          // init
        mips += "lw $31, 4($30)\n";    // restore ($31)
        mips += "add $30, $30, $4\n";
        for(const auto &child : t.children) {
            mips += generateMips(symbolTableNew, child, paramCount, offset, X, Y, Z);
        }
        mips += epilogue();
        return mips;
    } else if (t.tokens[0] == "procedure") {
        procName = t.children[1].tokens[1];
        offset = 4;
        mips += "F" + procName + ":\n"; // label
        mips += "sub $29, $30, $0\n";   // set stack frame pointer
        mips += generateMips(symbolTableNew, t.children[3], paramCount, offset, X, Y, Z);
        mips += generateMips(symbolTableNew, t.children[6], paramCount, offset, X, Y, Z);
        updateSymbolTable(symbolTableNew, procName);
        mips += "sw $1, 0($30)\n";      // save registers
        mips += "sub $30, $30, $4\n";
        mips += "sw $2, 0($30)\n";
        mips += "sub $30, $30, $4\n";
        mips += "sw $5, 0($30)\n";
        mips += "sub $30, $30, $4\n";
        mips += "sw $6, 0($30)\n";
        mips += "sub $30, $30, $4\n";
        mips += "sw $7, 0($30)\n";
        mips += "sub $30, $30, $4\n";
        mips += generateMips(symbolTableNew, t.children[7], paramCount, offset, X, Y, Z);
        mips += generateMips(symbolTableNew, t.children[9], paramCount, offset, X, Y, Z);
        mips += "lw $7, 4($30)\n";      // pop registers
        mips += "add $30, $30, $4\n";
        mips += "lw $6, 4($30)\n";
        mips += "add $30, $30, $4\n";
        mips += "lw $5, 4($30)\n";
        mips += "add $30, $30, $4\n";
        mips += "lw $2, 4($30)\n";
        mips += "add $30, $30, $4\n";
        mips += "lw $1, 4($30)\n";
        mips += "add $30, $30, $4\n";
        mips += "add $30, $29, $0\n";
        mips += "jr $31\n";
        return mips;
    } else if (t.rule == "dcl type ID") {
        paramCount++;
        offset = offset - 4;
        // Update locatin of variable ID
        string id = t.children[1].tokens[1];
        pair<vector<string>, map<string, pair<string, int>>> procTable = symbolTableNew.at(procName);
        map<string, pair<string, int>> decls = procTable.second;
        pair<string, int> typeLocation = decls.at(id);
        pair<string, int> newTypeLocation = make_pair(typeLocation.first, offset);
        decls[id] = newTypeLocation;
        pair<vector<string>, map<string, pair<string, int>>> newProcTable = make_pair(procTable.first, decls);
        symbolTableNew[procName] = newProcTable;
    } else if (t.rule == "factor ID") {
        string id = t.children[0].tokens[1];
        int idOffset = symbolTableNew.at(procName).second.at(id).second;
        mips += "lw $3, " + to_string(idOffset) + "($29)\n";
        return mips;
    } else if (t.rule == "factor NUM") {
        string num = t.children[0].tokens[1];
        mips += "lis $3\n";
        mips += ".word " + num + "\n";
        return mips;
    } else if (t.rule == "factor NULL") {
        mips += "add $3, $11, $0\n";
        return mips;
    } else if (t.rule == "factor STAR factor") {
        string mipsFac = generateMips(symbolTableNew, t.children[1], paramCount, offset, X, Y, Z);
        mips += mipsFac;
        mips += "lw $3, 0($3)\n";
        return mips;
    } else if (t.rule == "factor AMP lvalue") {
        string mipsLval;
        string mipsFac;
        string id;
        Tree temp;
        int idOffset;
        // Get id drived from lvalue
        if (t.children[1].rule == "lvalue ID") {
            id = t.children[1].children[0].tokens[1];
        } else if (t.children[1].rule == "lvalue LPAREN lvalue RPAREN") {
            temp = t.children[1];
            while(temp.rule != "lvalue ID") {
                temp = temp.children[1];
            }
            id = temp.children[0].tokens[1];
        } else if (t.children[1].rule == "lvalue STAR factor") {
            // factor -> &(*factor) = factor
            mipsFac = generateMips(symbolTableNew, t.children[1].children[1], paramCount, offset, X, Y, Z);
            mips += mipsFac;
            return mips;
        }
        // Get lvalue's address from the symolTable offset
        idOffset = symbolTableNew.at(procName).second.at(id).second;
        mips += "lis $5\n";
        mips += ".word " + to_string(idOffset) + "\n";
        mips += "add $3, $5, $29\n";
        return mips;
    } else if (t.rule == "expr expr PLUS term") {
        string mipsl = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        string mipsr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        if (t.children[0].type == "int" && t.children[2].type == "int") {
            mips += mipsl;                 // code (expr)
            mips += "sw $3, 0($30)\n";     // push ($3)
            mips += "sub $30, $30, $4\n";
            mips += mipsr;                 // code (term)
            mips += "lw $5, 4($30)\n";     // pop ($5)
            mips += "add $30, $30, $4\n";
            mips += "add $3, $5, $3\n";    // $5 + $3
        } else if (t.children[0].type == "int" && t.children[2].type == "int*") {
            mips += mipsl;                 // code (expr)
            mips += "sw $3, 0($30)\n";     // push ($3)
            mips += "sub $30, $30, $4\n";
            mips += mipsr;                 // code (term)
            mips += "lw $5, 4($30)\n";     // pop ($5)
            mips += "add $30, $30, $4\n";
            mips += "mult $5, $4\n";       // expr * 4 
            mips += "mflo $5\n";
            mips += "add $3, $5, $3\n";    // expr * 4 + term
        } else if (t.children[0].type == "int*" && t.children[2].type == "int") {
            mips += mipsl;                 // code (expr)
            mips += "sw $3, 0($30)\n";     // push ($3)
            mips += "sub $30, $30, $4\n";
            mips += mipsr;                 // code (term)
            mips += "lw $5, 4($30)\n";     // pop ($5)
            mips += "add $30, $30, $4\n";
            mips += "mult $3, $4\n";       // term * 4 
            mips += "mflo $3\n";
            mips += "add $3, $5, $3\n";    // expr + term * 4
        }
        return mips;
    } else if (t.rule == "expr expr MINUS term") {
        string mipsl = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        string mipsr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        if (t.children[0].type == "int" && t.children[2].type == "int") {
            mips += mipsl;                 // code (expr)
            mips += "sw $3, 0($30)\n";     // push ($3)
            mips += "sub $30, $30, $4\n";
            mips += mipsr;                 // code (term)
            mips += "lw $5, 4($30)\n";     // pop ($5)
            mips += "add $30, $30, $4\n";  
            mips += "sub $3, $5, $3\n";    // $5 - $3
        } else if (t.children[0].type == "int*" && t.children[2].type == "int") {
            mips += mipsl;                 // code (expr)
            mips += "sw $3, 0($30)\n";     // push ($3)
            mips += "sub $30, $30, $4\n";
            mips += mipsr;                 // code (term)
            mips += "lw $5, 4($30)\n";     // pop ($5)
            mips += "add $30, $30, $4\n";  
            mips += "mult $3, $4\n";       // term * 4 
            mips += "mflo $3\n";
            mips += "sub $3, $5, $3\n";    // expr - term * 4
        } else if (t.children[0].type == "int*" && t.children[2].type == "int*") {
            mips += mipsl;                 // code (expr)
            mips += "sw $3, 0($30)\n";     // push ($3)
            mips += "sub $30, $30, $4\n";
            mips += mipsr;                 // code (term)
            mips += "lw $5, 4($30)\n";     // pop ($5)
            mips += "add $30, $30, $4\n";
            mips += "sub $3, $5, $3\n";    // expr - term
            mips += "div $3, $4\n";        // (expr - term) / 4
            mips += "mflo $3\n";
        }
        return mips;
    } else if (t.rule == "term term STAR factor") {
        string mipsl = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        string mipsr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        mips += mipsl;                 // code (expr)
        mips += "sw $3, 0($30)\n";     // push ($3)
        mips += "sub $30, $30, $4\n";
        mips += mipsr;                 // code (term)
        mips += "lw $5, 4($30)\n";     // pop ($5)
        mips += "add $30, $30, $4\n";
        mips += "mult $5, $3\n";       // $5 * $3
        mips += "mflo $3\n";
        return mips;
    } else if (t.rule == "term term SLASH factor") {
        string mipsl = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        string mipsr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        mips += mipsl;                 // code (expr)
        mips += "sw $3, 0($30)\n";     // push ($3)
        mips += "sub $30, $30, $4\n";
        mips += mipsr;                 // code (term)
        mips += "lw $5, 4($30)\n";     // pop ($5)
        mips += "add $30, $30, $4\n";
        mips += "div $5, $3\n";        // $5 / $3
        mips += "mflo $3\n";
        return mips;
    } else if (t.rule == "term term PCT factor") {
        string mipsl = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        string mipsr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        mips += mipsl;                 // code (expr)
        mips += "sw $3, 0($30)\n";     // push ($3)
        mips += "sub $30, $30, $4\n";
        mips += mipsr;                 // code (term)
        mips += "lw $5, 4($30)\n";     // pop ($5)
        mips += "add $30, $30, $4\n";
        mips += "div $5, $3\n";        // $5 / $3
        mips += "mfhi $3\n";
        return mips;
    } else if (t.rule == "statement PRINTLN LPAREN expr RPAREN SEMI") {
        string mipsExpr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        mips += mipsExpr;              // code (expr)
        mips += "sw $1, 0($30)\n";     // save ($1)
        mips += "sub $30, $30, $4\n";
        mips += "add $1, $3, $0\n";
        mips += "sw $31, 0($30)\n";    // save ($31)
        mips += "sub $30, $30, $4\n";
        mips += "jalr $10\n";          // print
        mips += "lw $31, 4($30)\n";    // restore ($31)
        mips += "add $30, $30, $4\n";
        mips += "lw $1, 4($30)\n";     // restore ($1)
        mips += "add $30, $30, $4\n";
        return mips;
    } else if (t.rule == "dcls dcls dcl BECOMES NUM SEMI") {
        // code(dcls)
        string mipsDcls = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        mips += mipsDcls;
        // code(dcl)
        string num = t.children[3].tokens[1];
        string id = t.children[1].children[1].tokens[1];
        mips += "lis $5\n";
        mips += ".word " + num + "\n";
        mips += "sw $5, 0($30)\n";
        mips += "sub $30, $30, $4\n";
        offset = offset - 4;
        // Update offset of variable ID
        pair<vector<string>, map<string, pair<string, int>>> procTable = symbolTableNew.at(procName);
        map<string, pair<string, int>> decls = procTable.second;
        pair<string, int> typeLocation = decls.at(id);
        pair<string, int> newTypeLocation = make_pair(typeLocation.first, offset);
        decls[id] = newTypeLocation;
        pair<vector<string>, map<string, pair<string, int>>> newProcTable = make_pair(procTable.first, decls);
        symbolTableNew[procName] = newProcTable;
        return mips;
    } else if (t.rule == "dcls dcls dcl BECOMES NULL SEMI") {
        // code(dcls)
        string mipsDcls = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        mips += mipsDcls;
        // code(dcl)
        string id = t.children[1].children[1].tokens[1];
        mips += "sw $11, 0($30)\n";
        mips += "sub $30, $30, $4\n";
        offset = offset - 4;
        // Update offset of variable ID
        pair<vector<string>, map<string, pair<string, int>>> procTable = symbolTableNew.at(procName);
        map<string, pair<string, int>> decls = procTable.second;
        pair<string, int> typeLocation = decls.at(id);
        pair<string, int> newTypeLocation = make_pair(typeLocation.first, offset);
        decls[id] = newTypeLocation;
        pair<vector<string>, map<string, pair<string, int>>> newProcTable = make_pair(procTable.first, decls);
        symbolTableNew[procName] = newProcTable;
        return mips;
    } else if (t.rule == "statement lvalue BECOMES expr SEMI") {
        string id;
        Tree temp;
        int idOffset;
        string mipsExpr;
        string mipsFac;
        // Find id derived from lvalue
        if (t.children[0].rule == "lvalue ID") {
            id = t.children[0].children[0].tokens[1];
        } else if (t.children[0].rule == "lvalue LPAREN lvalue RPAREN") {
            temp = t.children[0];
            while(temp.rule == "lvalue LPAREN lvalue RPAREN") {
                temp = temp.children[1];
            }
            id = temp.children[0].tokens[1];
        } else if(t.children[0].rule == "lvalue STAR factor") {
            mipsFac = generateMips(symbolTableNew, t.children[0].children[1], paramCount, offset, X, Y, Z);
            mipsExpr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
            mips += mipsFac;               // code(factor)
            mips += "sw $3, 0($30)\n";     // push ($3)
            mips += "sub $30, $30, $4\n";
            mips += mipsExpr;              // code (expr)
            mips += "lw $5, 4($30)\n";     // pop ($5)
            mips += "add $30, $30, $4\n";
            mips += "sw $3, 0($5)\n";
            return mips;
        }
        // Update id's value stored in stack 
        idOffset = symbolTableNew.at(procName).second.at(id).second;
        mipsExpr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        mips += mipsExpr;
        mips += "sw $3, " + to_string(idOffset) + "($29)\n";
        return mips;
    } else if (t.rule == "statement WHILE LPAREN test RPAREN LBRACE statements RBRACE") {
        string mipsTest = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        string mipsStmt = generateMips(symbolTableNew, t.children[5], paramCount, offset, X, Y, Z);
        mips += "loop" + to_string(Y) + ":\n";             // loop starts
        mips += mipsTest;                                  // code(test)
        mips += "beq $3, $0, done"+ to_string(Y) + "\n";   // if test fails
        mips += mipsStmt;                                  // code(statement)
        mips += "beq $0, $0, loop"+ to_string(Y) + "\n";   // loop again
        mips += "done" + to_string(Y) + ":\n";             // loop ends
        Y++; 
        return mips;
    } else if (t.rule == "statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE") {
        string mipsTest = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        string mipsStmt1 = generateMips(symbolTableNew, t.children[5], paramCount, offset, X, Y, Z);
        string mipsStmt2 = generateMips(symbolTableNew, t.children[9], paramCount, offset, X, Y, Z);
        mips += mipsTest;                                  // code(test)
        mips += "beq $3, $11, true"+ to_string(X) + "\n";  // if test succeeds
        mips += mipsStmt2;                                 // "else statement"
        mips += "beq $0, $0, endif"+ to_string(X) + "\n"; 
        mips += "true" + to_string(X) + ":\n";             // "if statement"
        mips += mipsStmt1;
        mips += "endif" + to_string(X) + ":\n";            // if ends
        X++; 
        return mips;
    } else if (t.rule == "test expr LT expr") {
        string mipsl = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        string mipsr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
         mips += mipsl;                 // code (expr)
        mips += "sw $3, 0($30)\n";     // push ($3)
        mips += "sub $30, $30, $4\n";
        mips += mipsr;                 // code (term)
        mips += "lw $5, 4($30)\n";     // pop ($5)
        mips += "add $30, $30, $4\n";
        if (t.children[0].type == "int") {
            mips += "slt $3, $5, $3\n";    // $3 = 1 if expr1 < expr2; 0 otherwise
        } else if (t.children[0].type == "int*") {
            mips += "sltu $3, $5, $3\n";   // $3 = 1 if expr1 < expr2; 0 otherwise
        }
        return mips;
    } else if (t.rule == "test expr GT expr") {
        string mipsl = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        string mipsr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        mips += mipsl;                 // code (expr)
        mips += "sw $3, 0($30)\n";     // push ($3)
        mips += "sub $30, $30, $4\n";
        mips += mipsr;                 // code (term)
        mips += "lw $5, 4($30)\n";     // pop ($5)
        mips += "add $30, $30, $4\n";
        if (t.children[0].type == "int") {
            mips += "slt $3, $3, $5\n";   // $3 = 1 if expr1 > expr2; 0 otherwise
        } else if (t.children[0].type == "int*") {
            mips += "sltu $3, $3, $5\n";  // $3 = 1 if expr1 > expr2; 0 otherwise
        }
        return mips;
    } else if (t.rule == "test expr NE expr") {
        string mipsl = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        string mipsr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        mips += mipsl;                 // code (expr)
        mips += "sw $3, 0($30)\n";     // push ($3)
        mips += "sub $30, $30, $4\n";
        mips += mipsr;                 // code (term)
        mips += "lw $5, 4($30)\n";     // pop ($5)
        mips += "add $30, $30, $4\n";
        if (t.children[0].type == "int") {
            mips += "slt $6, $3, $5\n";    // $6 = 1 if expr1 > expr2; 0 otherwise
            mips += "slt $7, $5, $3\n";    // $7 = 1 if expr2 > expr1; 0 otherwise
        } else if (t.children[0].type == "int*") {
            mips += "sltu $6, $3, $5\n";   // $6 = 1 if expr1 > expr2; 0 otherwise
            mips += "sltu $7, $5, $3\n";   // $7 = 1 if expr2 > expr1; 0 otherwise
        }
        mips += "add $3, $6, $7\n";    // $3 = 1 if $6 or $7 = 1 => not equal.
        return mips;
    } else if (t.rule == "test expr EQ expr") {
        string mipsl = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        string mipsr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        mips += mipsl;                 // code (expr)
        mips += "sw $3, 0($30)\n";     // push ($3)
        mips += "sub $30, $30, $4\n";
        mips += mipsr;                 // code (term)
        mips += "lw $5, 4($30)\n";     // pop ($5)
        mips += "add $30, $30, $4\n";
        if (t.children[0].type == "int") {
            mips += "slt $6, $3, $5\n";    // $6 = 1 if expr1 > expr2; 0 otherwise
            mips += "slt $7, $5, $3\n";    // $7 = 1 if expr2 > expr1; 0 otherwise
        } else if (t.children[0].type == "int*") {
            mips += "sltu $6, $3, $5\n";   // $6 = 1 if expr1 > expr2; 0 otherwise
            mips += "sltu $7, $5, $3\n";   // $7 = 1 if expr2 > expr1; 0 otherwise
        }
        mips += "add $3, $6, $7\n";    // $3 = 1 if $6 or $7 = 1 => not equal.
        mips += "sub $3, $11, $3\n";   // $3 = 1 if expr1 == expr2
        return mips;
    } else if (t.rule == "test expr LE expr") {
        string mipsl = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        string mipsr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        mips += mipsl;                 // code (expr)
        mips += "sw $3, 0($30)\n";     // push ($3)
        mips += "sub $30, $30, $4\n";
        mips += mipsr;                 // code (term)
        mips += "lw $5, 4($30)\n";     // pop ($5)
        mips += "add $30, $30, $4\n";
        if (t.children[0].type == "int") {
            mips += "slt $3, $3, $5\n";    // $3 = 0 if expr1 <= expr2; 1 otherwise
        } else if (t.children[0].type == "int*") {
            mips += "sltu $3, $3, $5\n";   // $3 = 0 if expr1 <= expr2; 1 otherwise
        }
        mips += "sub $3, $11, $3\n";   // $3 = 1 if expr1 <= expr2; 0 otherwise
        return mips;
    } else if (t.rule == "test expr GE expr") {
        string mipsl = generateMips(symbolTableNew, t.children[0], paramCount, offset, X, Y, Z);
        string mipsr = generateMips(symbolTableNew, t.children[2], paramCount, offset, X, Y, Z);
        mips += mipsl;                 // code (expr)
        mips += "sw $3, 0($30)\n";     // push ($3)
        mips += "sub $30, $30, $4\n";
        mips += mipsr;                 // code (term)
        mips += "lw $5, 4($30)\n";     // pop ($5)
        mips += "add $30, $30, $4\n";
        if (t.children[0].type == "int") {
            mips += "slt $3, $5, $3\n";    // $3 = 0 if expr1 >= expr2; 1 otherwise
        } else if (t.children[0].type == "int*") {
            mips += "sltu $3, $5, $3\n";   // $3 = 0 if expr1 >= expr2; 1 otherwise
        }
        mips += "sub $3, $11, $3\n";   // $3 = 1 if expr1 >= expr2; 0 otherwise
        return mips;
    } else if (t.rule == "factor NEW INT LBRACK expr RBRACK") {
        string mipsExpr = generateMips(symbolTableNew, t.children[3], paramCount, offset, X, Y, Z);
        mips += mipsExpr;
        mips += "add $1, $3, $0\n";
        // Call new
        mips += "sw $31, 0($30)\n";    // save ($31)
        mips += "sub $30, $30, $4\n";
        mips += "lis $31\n";
        mips += ".word new\n";
        mips += "jalr $31\n";          // new
        mips += "lw $31, 4($30)\n";    // restore ($31)
        mips += "add $30, $30, $4\n";
        mips += "bne $3, $0, 1\n";
        mips += "add $3, $11, $0\n";
        return mips;
    } else if (t.rule == "statement DELETE LBRACK RBRACK expr SEMI") {
        string mipsExpr = generateMips(symbolTableNew, t.children[3], paramCount, offset, X, Y, Z);
        mips += mipsExpr;
        mips += "beq $3, $11, skipDelete" + to_string(Z) + "\n"; // if NULL
        mips += "add $1, $3, $0\n";
        // Call delete
        mips += "sw $31, 0($30)\n";    // save ($31)
        mips += "sub $30, $30, $4\n";
        mips += "lis $31\n";
        mips += ".word delete\n";
        mips += "jalr $31\n";          // delete
        mips += "lw $31, 4($30)\n";    // restore ($31)
        mips += "add $30, $30, $4\n";
        mips += "skipDelete" + to_string(Z) + ":\n";
        Z++; 
        return mips;
    } else if (t.rule == "factor ID LPAREN RPAREN") {
        string procedureName = t.children[0].tokens[1];
        // procName = procedureName;
        mips += "sw $29, 0($30)\n";    // save ($29)
        mips += "sub $30, $30, $4\n";
        mips += "sw $31, 0($30)\n";    // save ($31)
        mips += "sub $30, $30, $4\n";
        mips += "lis $31\n";
        mips += ".word F" + procedureName + "\n";
        mips += "jalr $31\n";          // call procedure
        mips += "lw $31, 4($30)\n";    // restore ($31)
        mips += "add $30, $30, $4\n";
        mips += "lw $29, 4($30)\n";    // restore ($29)
        mips += "add $30, $30, $4\n";
        return mips;
    } else if (t.rule == "factor ID LPAREN arglist RPAREN") {
        string procedureName = t.children[0].tokens[1];
        int numArg = symbolTableNew.at(procedureName).first.size();
        Tree temp = t.children[2];
        string mipsExpr;
        mips += "sw $29, 0($30)\n";    // save ($29)
        mips += "sub $30, $30, $4\n";
        mips += "sw $31, 0($30)\n";    // save ($31)
        mips += "sub $30, $30, $4\n";
        int counter = 1;
        while(temp.rule == "arglist expr COMMA arglist") { // push arguments
        counter++;
            mipsExpr = generateMips(symbolTableNew, temp.children[0], paramCount, offset, X, Y, Z);
            mips += mipsExpr;
            mips += "sw $3, 0($30)\n";
            mips += "sub $30, $30, $4\n";
            temp = temp.children[2];
        }
        mipsExpr = generateMips(symbolTableNew, temp.children[0], paramCount, offset, X, Y, Z);
        mips += mipsExpr;
        mips += "sw $3, 0($30)\n";
        mips += "sub $30, $30, $4\n";
        mips += "lis $31\n";
        mips += ".word F" + procedureName + "\n";
        mips += "jalr $31\n";          // call procedure
        mips += "lis $5\n";            // pop arguments
        mips += ".word " + to_string(numArg) + "\n";
        mips += "multu $4, $5\n";
        mips += "mflo $5\n";
        mips += "add $30, $30, $5\n";
        mips += "lw $31, 4($30)\n";    // restore ($31)
        mips += "add $30, $30, $4\n";
        mips += "lw $29, 4($30)\n";    // restore ($29)
        mips += "add $30, $30, $4\n";
        return mips;
    }
    for(const auto &child : t.children) {
        mips += generateMips(symbolTableNew, child, paramCount, offset, X, Y, Z);
    }
    return mips;
}

int main() {
    int index = 0;

    // Get all terminals
    vector<string> terminals = getTerminals();

    // Read all wlp4i and store in a vector
    vector<string> wlp4i;
    string str;
    while(getline(cin, str)) {
        wlp4i.push_back(str);
    }
    // Build parse tree
    Tree parseTree = buildParseTree(wlp4i, index, terminals);
    // Build symbolTable
    map<string, string> st;
    vector<string> sig;
    vector<string> procNames;
    map<string, pair<vector<string>, map<string, string>>> symbolTable;
    try {
        buildSymbolTable(symbolTable, parseTree, procNames, sig, st);
        findExprSetType(parseTree, symbolTable);
    } catch (DeclarationFailure &f) {
        cerr << f.what() << endl;
        return 1;
    }

    // Reformat symbolTable
    map<string, pair<vector<string>, map<string, pair<string, int>>>> symbolTableNew = addLoc(symbolTable);
    // Generate assembly code
    // procName = "wain";
    string mips;
    int paramCount = 0;
    int offset = 4;
    int X = 0;
    int Y = 0;
    int Z = 0;
    mips += generateMips(symbolTableNew, parseTree, paramCount, offset, X, Y, Z);
    //printSymbolTable(symbolTableNew);
    cout << mips;
}
