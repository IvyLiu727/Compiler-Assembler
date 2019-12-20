#include <iostream>
#include <sstream>
#include <limits.h>
#include <algorithm>
#include <map> 
#include "scanner.h"
using namespace std;

/*
 * C++ Starter code for CS241 A3
 * All code requires C++14, so if you're getting compile errors make sure to
 * use -std=c++14.
 *
 * This file contains the main function of your program. By default, it just
 * prints the scanned list of tokens back to standard output.
 */

// Stores the token's lexeme in label and the pc value of its
// instruction for the third token in beq and bne which is a label
struct iLabel {
  string label;
  int pc;
};

// Outputs the assembled statement byte by byte using chars
void outputBytes(int instr) {
  char c = instr >> 24;
  cout << c;
  c = instr >> 16;
  cout << c;
  c = instr >> 8;
  cout << c;
  c = instr;
  cout << c;
}

// Returns the integer representation of instruction instrType 
// with operands op1 (op2 and op3) using bit shit and binary op.
int assembleInstr(
  string instrType, int64_t op1, int64_t op2, int64_t op3) {
  int instr;
  if (instrType == ".word") {
    instr = op1;
  } else if (instrType == "jr") {
    instr = (op1 << 21) | 8;
  } else if (instrType == "jalr") {
    instr = (op1 << 21) | 9;
  } else if (instrType == "add") {
    instr = (op1 << 11) | (op2 << 21) | (op3 << 16) | 32;
  } else if (instrType == "sub") {
    instr = (op1 << 11) | (op2 << 21) | (op3 << 16) | 34;
  } else if (instrType == "slt") {
    instr = (op1 << 11) | (op2 << 21) | (op3 << 16) | 42;
  } else if (instrType == "sltu") {
    instr = (op1 << 11) | (op2 << 21) | (op3 << 16) | 43;
  } else if (instrType == "beq") {
    instr = (op1 << 21) | (op2 << 16) | (op3 & 0xffff) | (4 << 26);
  } else if (instrType == "bne") {
    instr = (op1 << 21) | (op2 << 16) | (op3 & 0xffff) | (5 << 26);
  } else if (instrType == "lis") {
    instr = (op1 << 11) | 20;
  } else if (instrType == "mfhi") {
    instr = (op1 << 11) | 16;
  } else if (instrType == "mflo") {
    instr = (op1 << 11) | 18;
  } else if (instrType == "mult") {
    instr = (op1 << 21) | (op2 << 16) | 24;
  } else if (instrType == "multu") {
    instr = (op1 << 21) | (op2 << 16) | 25;
  } else if (instrType == "div") {
    instr = (op1 << 21) | (op2 << 16) | 26;
  } else if (instrType == "divu") {
    instr = (op1 << 21) | (op2 << 16) | 27;
  } else if (instrType == "sw") {
    instr = (op1 << 16) | (op2 & 0xffff) | (op3 << 21) | (43 << 26);
  } else if (instrType == "lw") {
    instr = (op1 << 16) | (op2 & 0xffff) | (op3 << 21) | (35 << 26);
  }
  return instr;
}

// Outputs the symbol table stored in labels to stderr
void outputSymbolTable(map<string, int> labels) {
  for (auto label : labels) {
    cerr << label.first << " " << label.second << endl;
  }
}

// Return a vector denoting valid types of operands for 
// instruction lexeme, each element of the vector is a
//  vector of the possible operand types at that position
vector<vector<Token::Kind>> setTypes(string lexeme) {
  vector<Token::Kind> op1Types;
  vector<Token::Kind> op2Types;
  vector<Token::Kind> op3Types;
  vector<Token::Kind> op4Types;
  vector<Token::Kind> op5Types;
  vector<Token::Kind> op6Types;
  vector<vector<Token::Kind>> operandTypes;
  if (lexeme == ".word" || lexeme == "jalr") {
    op1Types.push_back(Token::INT);
    op1Types.push_back(Token::HEXINT);
    op1Types.push_back(Token::ID);
    operandTypes.push_back(op1Types);
  }
  else if (lexeme == "jr" || lexeme == "jalr") {
    op1Types.push_back(Token::REG);
    operandTypes.push_back(op1Types);
  }
  else if (lexeme == "add" || lexeme == "sub"
      || lexeme == "slt" || lexeme == "sltu") {
    op1Types.push_back(Token::REG);
    op2Types.push_back(Token::COMMA);
    op3Types.push_back(Token::REG);
    op4Types.push_back(Token::COMMA);
    op5Types.push_back(Token::REG);
    operandTypes.push_back(op1Types);
    operandTypes.push_back(op2Types);
    operandTypes.push_back(op3Types);
    operandTypes.push_back(op4Types);
    operandTypes.push_back(op5Types);
  }
  else if (lexeme == "beq" || lexeme == "bne") {
    op1Types.push_back(Token::REG);
    op2Types.push_back(Token::COMMA);
    op3Types.push_back(Token::REG);
    op4Types.push_back(Token::COMMA);
    op5Types.push_back(Token::INT);
    op5Types.push_back(Token::HEXINT);
    op5Types.push_back(Token::ID);
    operandTypes.push_back(op1Types);
    operandTypes.push_back(op2Types);
    operandTypes.push_back(op3Types);
    operandTypes.push_back(op4Types);
    operandTypes.push_back(op5Types);
  } else if (lexeme == "lis" || lexeme == "mfhi" || lexeme == "mflo") {
    op1Types.push_back(Token::REG);
    operandTypes.push_back(op1Types);
  } else if (lexeme == "mult" || lexeme == "multu" || 
    lexeme == "div" || lexeme == "divu") {
    op1Types.push_back(Token::REG);
    op2Types.push_back(Token::COMMA);
    op3Types.push_back(Token::REG);
    operandTypes.push_back(op1Types);
    operandTypes.push_back(op2Types);
    operandTypes.push_back(op3Types);
  } else if (lexeme == "lw" || lexeme == "sw") {
    op1Types.push_back(Token::REG);
    op2Types.push_back(Token::COMMA);
    op3Types.push_back(Token::INT);
    op3Types.push_back(Token::HEXINT);
    op4Types.push_back(Token::LPAREN);
    op5Types.push_back(Token::REG);
    op6Types.push_back(Token::RPAREN);
    operandTypes.push_back(op1Types);
    operandTypes.push_back(op2Types);
    operandTypes.push_back(op3Types);
    operandTypes.push_back(op4Types);
    operandTypes.push_back(op5Types);
    operandTypes.push_back(op6Types);
  }
  return operandTypes;
}

// Throws an error if Token tp is out of bound
void checkBounds(Token *tp, int t = 0) {
  Token::Kind kind = tp->getKind();
  int64_t instr = tp->toLong();

  if (t == 1) { // i in lw, sw, beq, bne
    if (kind == Token::INT) {
      if (instr > SHRT_MAX || instr < SHRT_MIN) {
        throw ScanningFailure("ERROR: Constant out of bound.");
      }
    } else if (kind == Token::HEXINT) {
      if (instr > USHRT_MAX) {
        throw ScanningFailure("ERROR: Constant out of bound.");
      }
    } else if (kind == Token::REG) { 
      if (instr > 31 || instr < 0) {
        throw ScanningFailure("ERROR: Constant out of bound.");
      }
    }
  } else { // i after .word
    if (kind == Token::INT || kind == Token::HEXINT) {
      if (instr > UINT_MAX || instr < INT_MIN) {
        throw ScanningFailure("ERROR: Constant out of bound.");
      }
    } else if (kind == Token::REG) { 
      if (instr > 31 || instr < 0) {
        throw ScanningFailure("ERROR: Constant out of bound.");
      }
    }
  }
}

int main() {
  string line;
  vector<int> outputQueue;         // queue of instructions  
  vector<vector<Token>> prog;      // whole program in tokens
  map<string, int> labels;         // map for symbol table
  vector<string> operandLabels;    // all the labels that are operands
  int pcValue = 0;
  string label;                    // label without ":"
  string operandLabel;             // label appears as an operand
  Token prevToken = Token{Token::ID, "add"}; // random initialization value
  vector<iLabel> labelPC;  // vector of vector<label, pc at label>

  try {
    while (getline(cin, line)) {

      vector<Token> tokenLine = scan(line);
      prog.push_back(tokenLine);
      bool hasInstr = 0;
      int tokenCount = 0;
      int operandCur = 0;
      int operandMax = 0;
      vector<vector<Token::Kind>> operandTypes;
      vector<Token::Kind> op;

      // Pass 1
      for (auto &token : tokenLine) {

        tokenCount++;
        Token::Kind kind = token.getKind();
        Token::Kind prevKind = prevToken.getKind();
        string lexeme = token.getLexeme();
        string prevLexeme = prevToken.getLexeme();

        // First token has to be LABEL, WORD or ID 
        // (jr,jalr,add,sub,slt,sltu,beq or bne)
        if (tokenCount == 1 && kind == Token::LABEL) {
          label = lexeme;
          label.pop_back();
          // Check for duplicate label
          if (labels.find(label) == labels.end()) {
            labels[label] = pcValue;
          }
          else {
            throw ScanningFailure("ERROR: Duplicate symbol " + label);
          }
        }
        else if (tokenCount == 1 && kind == Token::WORD) {
          hasInstr = 1;
          operandTypes = setTypes(lexeme); 
          operandMax = operandTypes.size();
        }
        else if (tokenCount == 1 && kind == Token::ID) {
          hasInstr = 1;
          if (lexeme == "jr" || lexeme == "jalr" ||
              lexeme == "add" || lexeme == "sub" ||
              lexeme == "slt" || lexeme == "sltu" ||
              lexeme == "beq" || lexeme == "bne" ||
              lexeme == "lis" || lexeme == "mfhi" ||
              lexeme == "mflo" ||
              lexeme == "mult" || lexeme == "multu" ||
              lexeme == "div" || lexeme == "divu" ||
              lexeme == "sw" || lexeme == "lw") {
            operandTypes = setTypes(lexeme);
            operandMax = operandTypes.size();
          }
          else {
            throw ScanningFailure("ERROR: Invalid directive ." + lexeme);
          }
        }
        else if (tokenCount == 1) {
          throw ScanningFailure("ERROR: Invalid directive ." + lexeme);
        }
        // If prevToken is a LABEL, token has to be a LABEL, WORD or ID
        // (jr,jalr,add,sub,slt,sltu,beq,bne,lis,mfhi or mflo)
        else if (prevKind == Token::LABEL) {
          if (kind == Token::LABEL) {
            label = lexeme;
            label.pop_back();
            // Check for duplicate label
            if (labels.find(label) == labels.end()) {
              labels[label] = pcValue;
            }
            else {
              throw ScanningFailure("ERROR: Duplicate symbol " + label);
            }
          }
          else if (kind == Token::WORD) {
            hasInstr = 1;
            operandMax = 1;
          }
          else if (kind == Token::ID) {
            hasInstr = 1;
            if (lexeme == "jr" || lexeme == "jalr" ||
                lexeme == "add" || lexeme == "sub" ||
                lexeme == "slt" || lexeme == "sltu" ||
                lexeme == "beq" || lexeme == "bne" ||
                lexeme == "lis" || lexeme == "mfhi" ||
                lexeme == "mflo" ||
                lexeme == "mult" || lexeme == "multu" ||
                lexeme == "div" || lexeme == "divu" ||
                lexeme == "sw" || lexeme == "lw") {
              operandTypes = setTypes(lexeme);
              operandMax = operandTypes.size();
            }
            else {
              throw ScanningFailure("ERROR: Invalid directive ." + lexeme);
            }
          }
          else {
            throw ScanningFailure(
              "ERROR: Expecting opcode, label, or directive, but got " + lexeme);
          }
        }
        // If prevToken is a WORD, token should be a INT/HEXINT or LABEL
        else if (prevKind == Token::WORD) {
          operandCur = 1;
          if (kind == Token::INT || kind == Token::HEXINT) {
            checkBounds(&token);
          }
          else if (kind == Token::ID) {
            operandLabel = lexeme;
            operandLabels.push_back(operandLabel);
          }
          else {
            throw ScanningFailure("ERROR: Invalid operand after " + prevLexeme);
          }
        }
        // If prevToken is a valid instruction keyword, token should be REG
        else if (prevKind == Token::ID && (
          prevLexeme == "jr" || prevLexeme == "jalr" ||
          prevLexeme == "add" || prevLexeme == "sub" ||
          prevLexeme == "slt" || prevLexeme == "sltu" ||
          prevLexeme == "beq" || prevLexeme == "bne" ||
          prevLexeme == "lis" || prevLexeme == "mfhi" ||
          prevLexeme == "mflo" ||
          prevLexeme == "mult" || prevLexeme == "multu" ||
          prevLexeme == "div" || prevLexeme == "divu" ||
          prevLexeme == "sw" || prevLexeme == "lw")) {
          operandCur = 1;
          op = operandTypes[operandCur-1];
          if (find(op.begin(), op.end(), kind) == op.end()) {
            throw ScanningFailure("ERROR: Invalid operand after " + prevLexeme);
          }
          else if (kind == Token::REG) { 
            checkBounds(&token);
          }
          else {
            cerr << "Something wrong with setTypes." << endl;
          }
        }
        // If prevToken is an INT/HEX, token is a COMMA; otherwise invalid
        else if (prevKind == Token::INT || prevKind == Token::HEXINT) {
          operandCur += 1;
          op = operandTypes[operandCur-1];
          if (operandCur > operandMax) {
            throw ScanningFailure(
              "ERROR: Expected end of line, but there is more stuff");
          }
          else if (find(op.begin(), op.end(), kind) == op.end()) {
            throw ScanningFailure("ERROR: Invalid operand after " + prevLexeme);
          }
        }
        // If prevToken is a REG, token is COMMA or RPAREN; otherwise invalid
        else if (prevKind == Token::REG) {
          operandCur += 1;
          op = operandTypes[operandCur-1];
          if (operandCur > operandMax) {
            throw ScanningFailure(
              "ERROR: Expected end of line, but there is more stuff");
          }
          else if (find(op.begin(), op.end(), kind) == op.end()) {
            throw ScanningFailure("ERROR: Invalid operand after " + prevLexeme);
          }
        }
        // If prevToken is a COMMA, token is either REG, INT/HEX or ID 
        else if (prevKind == Token::COMMA) {
          operandCur += 1;
          op = operandTypes[operandCur-1];
          if (operandCur > operandMax) {
            throw ScanningFailure(
              "Should not happen, something wrong with operandMax.");
          }
          else if (find(op.begin(), op.end(), kind) == op.end()) {
            throw ScanningFailure("ERROR: Invalid operand after " + prevLexeme);
          }
          else if (kind == Token::REG) {
            checkBounds(&token);
          }
          else if (kind == Token::INT || kind == Token::HEXINT) {
            checkBounds(&token, 1);
          }
          else if (kind == Token::ID) {
            operandLabel = lexeme;
            operandLabels.push_back(operandLabel);
            iLabel pair{operandLabel, pcValue};
            labelPC.push_back(pair);
          }
        }
        // If prevToken is a LPAREN, token should be REG; otherwise invalid
        else if (prevKind == Token::LPAREN) {
          operandCur += 1;
          op = operandTypes[operandCur-1];
          if (operandCur > operandMax) {
            throw ScanningFailure(
              "Should not happen, something wrong with operandMax.");
          }
          else if (find(op.begin(), op.end(), kind) == op.end()) {
            throw ScanningFailure("ERROR: Invalid operand after " + prevLexeme);
          }
          else if (kind == Token::REG) {
            checkBounds(&token);
          }
        }
        // If prevToken is a RPAREN, token is invalid
        else if (prevKind == Token::LPAREN) {
            throw ScanningFailure(
              "ERROR: Expected end of line, but there is more stuff");
        }
        // Any other tokens is invalid
        else {
          throw ScanningFailure("ERROR: Invalid directive ." + prevLexeme);
        }
        // Keep track of the previous token
        prevToken = token;
      }

      // Check if this line ends properly. If there is an instruction,
      // increment memory address
      if (hasInstr) {
        if (operandMax != operandCur) {
          throw ScanningFailure("ERROR: Missing operand after " 
            + prevToken.getLexeme());
        }
        pcValue += 4;
      }
    }
    // Check if each label in operand exists in symbol table
    for (auto &label : operandLabels) {
      if (labels.find(label) == labels.end()) {
        throw ScanningFailure("ERROR: No such label: " + label);
      }
    }
    // Check if each label in operand of bne/beq satisfies 
    // -32768 < (i-l-4)/4 < 32767.
    for (auto &pair : labelPC) {
      string lex = pair.label;
      int l = pair.pc;
      int i = labels[lex];
      int v = (i - l - 4) / 4;
      if (v > SHRT_MAX || v < SHRT_MIN) {
        throw ScanningFailure("ERROR: Constant out of bound.");
      }
    }
  }
  catch (ScanningFailure &f) {
    cerr << f.what() << endl;
    return 1;
  }

  // Pass 2. 
  // Assuming program is valid, translate into machine code
  int pc = -4; // pc address (initial = no instruction = -4)
  for (auto &tokenLine : prog) {
    int64_t op1 = 0;
    int64_t op2 = 0;
    int64_t op3 = 0;
    int operandCur = 0; // Number of operands read in
    int operandMax = 0; // Number of operands allowed
    int numOpCount = 0; // Number of operands that are numbers
    vector<vector<Token::Kind>> operandTypes;
    string instrType = "";

    for (auto &token : tokenLine) {
      Token::Kind kind = token.getKind();
      string lexeme = token.getLexeme();

      if (instrType != "" && operandMax == 0) {
        operandTypes = setTypes(instrType);
        operandMax = operandTypes.size();
      }

      if (kind == Token::WORD) {
        instrType = lexeme;
        pc += 4;
      }
      else if (kind == Token::ID) {
        if (instrType == "beq" || instrType == "bne") {
          numOpCount += 1;
          operandCur += 1;
          int val = labels[lexeme];
          if (numOpCount == 1) op1 = (val - pc - 4) / 4; // shouldn't happen
          else if (numOpCount == 2) op2 = (val - pc - 4) / 4; // shouldn't happen
          else if (numOpCount == 3) op3 = (val - pc - 4) / 4;
        }
        else if (instrType == ".word") {
          numOpCount += 1;
          operandCur += 1;
          op1 = labels[lexeme];
        }
        else if (lexeme == "jr" || lexeme == "jalr" ||
            lexeme == "add" || lexeme == "sub" ||
            lexeme == "slt" || lexeme == "sltu" ||
            lexeme == "beq" || lexeme == "bne"  ||
            lexeme == "lis" || lexeme == "mfhi" ||
            lexeme == "mflo" ||
            lexeme == "mult" || lexeme == "multu" ||
            lexeme == "div" || lexeme == "divu" ||
            lexeme == "sw" || lexeme == "lw") {
          if (instrType == "") {
            instrType = lexeme;
            pc += 4;
          }
        }
      }
      else if (kind == Token::COMMA || 
        kind == Token::LPAREN ||
        kind == Token::RPAREN) {
        operandCur += 1;
      }
      else if (kind == Token::INT || 
               kind == Token::HEXINT ||
               kind == Token::REG) {
        operandCur += 1;
        numOpCount += 1;
        int64_t op = token.toLong();
        if (numOpCount == 1) op1 = op;
        else if (numOpCount == 2) op2 = op;
        else if (numOpCount == 3) op3 = op;
      }
      // If read end of the line, assemble and output it
      if (operandCur == operandMax && operandMax != 0) {
        int64_t instr = assembleInstr(instrType, op1, op2, op3);
        outputQueue.push_back(instr);
      }
    }
  }
  // Output equivalent MIPS machine language program
  // and the symbol table
  for (const auto &instr : outputQueue) {
    outputBytes(instr);
  }
  outputSymbolTable(labels);

  return 0;
}
