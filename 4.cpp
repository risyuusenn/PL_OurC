#include <iostream>  // cin.peek(), cin.get()
#include <cctype>  // isspace(), isdigit(), isalpha()
#include <string>  // stod(), to_string()
#include <sstream>  
#include <vector>
#include <map>  // count()
#include <iomanip>  // fixed, setprecision()
#include <cmath>  // fabs()

using namespace std;

int uTestNum = -1;                // Project 1 要求之 file-scope variable
const float TOLERANCE = 0.0001;   // Project 1 要求之 tolerance for floats 
const int PRECISION = 3;          // project 1 要求之浮點數顯示精度

// OurC 符號Token字典
// 目前本Scanner設計只適用最多 2 char 長度的SYMBOL token
const vector<string> SYMBOL_DICTIONARY = {"+", "-", "*", "/", "%", "++", "--",
                                          "=", "+=", "-=", "*=", "/=", "%=",
                                          "==", "!=", "<", ">", "<=", ">=",
                                          "&&", "||", "&", "|", "^", "!", "?",
                                          "<<", ">>", 
                                          "(", ")", "[", "]", "{", "}",
                                          ";", ",", ":", "//"};

// OurC 保留字Token字典
const vector<string> KEYWORD_DICTIONARY = { "int", "float", "char", "string", "bool", "void", 
                                            "if", "else", "while", "do", "return",
                                            "cin", "cout",
                                            "ListAllVariables", "ListAllFunctions", "ListVariable", "ListFunction", "Done" };

enum class TokenType {
  EMPTY,        // 預設值
  IDENT,        // Identifiers，只存變數名稱
  CONST_INT, CONST_FLOAT, CONST_CHAR, CONST_STRING, CONST_BOOL, CONST_VOID,  // 數值
  SYMBOL,       // 各種 符號 or 運算子
  END_OF_FILE,  // EOF
};

class Token {
 public:
  TokenType type;
  string value;  // token的字串內容
  int line;  // 紀錄座標
  int column;  // 紀錄座標

  Token() {
    type = TokenType::EMPTY;
    value = ">>> Token : //>w<// <<<";
    line = -1;
    column = -1;
  }

  Token(TokenType type, string value, int line = -1 , int column = -1) {
    this->type = type;
    this->value = value;
    this->line = line;
    this->column = column;
  }

  Token(TokenType type, char value, int line = -1, int column = -1) {
    this->type = type;
    this->value = string(1, value);  // char轉string
    this->line = line;
    this->column = column;
  }
};

enum class ExceptionType {
  EMPTY,        // 預設值
  LEXICAL_ERR,
  SYNTACTIC_ERR,
  SEMANTIC_ERR,
  OTHER_ERR,
  RUNTIME_ERR,
  DONE,
  END_OF_FILE,
};

class OurException {
 public:
  ExceptionType error_type;
  string error_msg;

  OurException() {
    error_type = ExceptionType::EMPTY;
    error_msg = ">>> OurException : Never Gonna Give You Up <<<";
  } 
  OurException(ExceptionType error_type, const string & info, int line) {
    this->error_type = error_type; 
    if (error_type == ExceptionType::LEXICAL_ERR) {
      this->error_msg = "Line " + to_string(line) + " : unrecognized token with first char '" + info + "'\n";
    } else if (error_type == ExceptionType::SYNTACTIC_ERR) {
      this->error_msg = "Line " + to_string(line) + " : unexpected token '" + info + "'\n";
    } else if (error_type == ExceptionType::SEMANTIC_ERR) {
      this->error_msg = "Line " + to_string(line) + " : undefined identifier '" + info + "'\n";
    } else if (error_type == ExceptionType::OTHER_ERR) {
      this->error_msg = "Error\n";  // ???????????? 待研議 ???????????????
    } else if (error_type == ExceptionType::RUNTIME_ERR) {
      this->error_msg = info;
    } else {
      this->error_msg = "Error in : "+ info + "\n";  // 用於檢查bug
    }
  }
  OurException(ExceptionType error_type, Token token) : OurException(error_type, token.value, token.line) {}
};

class Scanner {
 private:
  char current_char = '\0';
  int current_line = 1;
  int current_column = 0;
  int start_line = 0;  // token的起始line
  int start_column = 0;  // token的起始column

  // 自定義GetNextChar()，會自動維護 current_行列 數值。
  char GetNextChar() {
    if (current_char == '\n') {  // 若前一個char是'\n'
      current_line++;
      current_column = 1;
    } else {  // 否則就右移一位
      current_column++;
    }
    return cin.get();  // 讀取一個字元並回傳
  }
 
  // 讀取下一個非空字元
  char GetNextNonWhiteChar() {
    current_char = GetNextChar();
    // 讀掉空白字元，遇到EOF直接return
    while (isspace(current_char) && current_char != EOF) {
      current_char = GetNextChar();
    }
    start_line = current_line;
    start_column = current_column;  // token的起始column
    return current_char;
  }
  
  Token ReadWhole_IDENT_BOOL() {
    string token_string = {current_char};  // token的第一個char
    char next_char = cin.peek();  // 看下一個char (還沒讀取)
    // 必須是 字母 or 數字 or '_'
    while (isdigit(next_char) || isalpha(next_char) || next_char == '_') {
      if (next_char == EOF) break;  // 不會讀掉EOF，讓下次Get Token 拿到 EOF token
      current_char = GetNextChar();
      token_string += current_char;
      next_char = cin.peek();
    }
    // BOOL
    if (token_string == "true" || token_string == "false") {
      return Token(TokenType::CONST_BOOL, token_string, start_line, start_column);
    }
    return Token(TokenType::IDENT, token_string, start_line, start_column);
  }

  Token ReadWhole_STRING() {
    string token_string = {current_char};
    while (cin.peek() != '\"' && cin.peek() != EOF) {  // 不會讀掉EOF，讓下次Get Token 拿到 EOF token
      current_char = GetNextChar();
      token_string += current_char;
    }
    if (cin.peek() == '\"') {
      current_char = GetNextChar();
      token_string += current_char;
      return Token(TokenType::CONST_STRING, token_string, start_line, start_column);
    }
    throw OurException(ExceptionType::OTHER_ERR, "", 0);  // 研議：是否有這種測資？
  }

  Token ReadWhole_CHAR() {
    string token_string = {current_char};
    if (cin.peek() != '\'' && cin.peek() != EOF) {  // 不會讀掉EOF，讓下次Get Token 拿到 EOF token
      current_char = GetNextChar();
      token_string += current_char;
    }
    if (cin.peek() == '\'') {
      current_char = GetNextChar();
      token_string += current_char;
      return Token(TokenType::CONST_CHAR, token_string, start_line, start_column);
    }
    throw OurException(ExceptionType::OTHER_ERR, "", 0);  // 研議：是否有這種測資？
  }

  Token ReadWhole_NUM() {
    string token_string = {current_char};  // token的第一個char
    bool has_dot = (current_char == '.');  // 如果第一個char就是'.'
    // 把下個chat存入token，直到不是數字
    while (isdigit(cin.peek()) && cin.peek() != EOF) {
      current_char = GetNextChar();
      token_string += current_char;
    }
    // 若第一次遇到'.'，繼續存char直到不是數字
    if (has_dot == false && cin.peek() == '.' && cin.peek() != EOF) {
      current_char = GetNextChar();
      token_string += current_char;
      has_dot = true;
      while (isdigit(cin.peek()) && cin.peek() != EOF) {
        current_char = GetNextChar();
        token_string += current_char;
      }
    }
    // 錯誤：只有"." 
    if (token_string == ".") {
      throw OurException(ExceptionType::LEXICAL_ERR, string(1, current_char), start_line);
    } 
    // 回傳token
    if (has_dot) {
      return Token(TokenType::CONST_FLOAT, token_string, start_line, start_column);
    }
    return Token(TokenType::CONST_INT, token_string, start_line, start_column);
  }

  // 查SYMBOL_DICTIONARY字典，檢驗此token是否可能成為合法symbol_token
  bool MayBeSymbol(const string & token_string) {
    for (const auto & symbol_token : SYMBOL_DICTIONARY) {  // 查SYMBOL_DICTIONARY
      if (symbol_token.size() < token_string.size()) {  // 此symbol_token長度不足，跳過，繼續查下一個symbol_token
        continue;
      } 
      // 檢查token是否符合此symbol_token
      bool may_be_symbol = true;
      for (int t = 0; t < token_string.size(); t++) {
        if (token_string[t] != symbol_token[t]) {
          may_be_symbol = false;
        }
      }
      if (may_be_symbol) {
        return true;
      }
    }
    return false;  // 沒找到任何符合的symbol_token
  }

  bool IsOneCharSymbol(const char one_char) {
    for (const auto & symbol_token : SYMBOL_DICTIONARY) {
      if (symbol_token.size() == 1 && symbol_token[0] == one_char) {
        return true;
      }
    }
    return false;
  }

  Token ReadWhole_Symbol() {
    string token_string = {current_char};  // token的第一個char
    // OurC的symbol token應該都只有2 char，所以只要再檢查下一個char
    if (cin.peek() != EOF && MayBeSymbol(token_string + (char)cin.peek())) {  // 成為合法 2_char symbol_token
      current_char = GetNextChar();
      token_string += current_char;
      return Token(TokenType::SYMBOL, token_string, start_line, start_column);
    } else if (IsOneCharSymbol(current_char)) {          // 合法 1_char symbol_token
      return Token(TokenType::SYMBOL, current_char, start_line, start_column);
    } else {                                             // 非法字元，丟出LEXICAL_ERR
      throw OurException(ExceptionType::LEXICAL_ERR, string(1, current_char), start_line);
    }
    return Token();  // 正常不會執行此行，為了避免Compiler Warning
  }

 public:
  Scanner() {
    current_char = '\0';
    current_line = 1;
    current_column = 0;
  }

  // 丟出OurException(LEXICAL_ERR 或 OTHER_ERR) 給 Parser處理
  Token GetNextToken() {
    current_char = GetNextNonWhiteChar();  // 讀取此token的第一個char
    Token token;
    if (isalpha(current_char)) {  // IDENT | BOOL
      token = ReadWhole_IDENT_BOOL();
    } else if (current_char == '\"') {  // STRING
      token = ReadWhole_STRING();
    } else if (current_char == '\'') {  // CHAR
      token = ReadWhole_CHAR();
    } else if (isdigit(current_char) || current_char == '.') {  // NUM
      token = ReadWhole_NUM();  // 若只有'.'，會LEXICAL_ERR
    } else if (MayBeSymbol(string(1, current_char))) {  // Symbol 
      token = ReadWhole_Symbol();  // ReadWhole_Symbol() 會檢查是否成功拼成SYMBOL，失敗會LEXICAL_ERR
      // 忽略單行註解，並重新get token
      if (token.value == "//") {
        IgnoreThisLine();
        token = GetNextToken();
      }
    } else if (current_char == EOF) {
      token = Token(TokenType::END_OF_FILE, current_char, start_line, start_column);  
    } else {
      throw OurException(ExceptionType::LEXICAL_ERR, string(1, current_char), start_line);
    }
    return token;
  }
  
  // 讀掉(忽略)整行input
  void IgnoreThisLine() {
    while (current_char != '\n' && current_char != EOF) {
      current_char = GetNextChar();
    }
  }

  void Clear() {
    IgnoreThisLine();
    ResetLine();
  }

  void ResetLine() {
    current_line = 0;
    current_column = 0;
  }

  void SetLine(int line) {  // 為了修bug: a \n a \n ;
    current_line = line;
    current_column = 0;
  }
};

class Tools {
 public:
  string TokenType_To_String(TokenType type) {
    if (type == TokenType::CONST_INT) return "int";
    if (type == TokenType::CONST_FLOAT) return "float";
    if (type == TokenType::CONST_CHAR) return "char";
    if (type == TokenType::CONST_STRING) return "string";
    if (type == TokenType::CONST_BOOL) return "boolean";
    if (type == TokenType::CONST_VOID) return "void";
    throw OurException(ExceptionType::EMPTY, "Interpreter::TokenType_To_String()", 0);
  }

  TokenType String_To_TokenType(string str) {
    if (str == "int") return TokenType::CONST_INT;
    if (str == "float") return TokenType::CONST_FLOAT;
    if (str == "char") return TokenType::CONST_CHAR;
    if (str == "string") return TokenType::CONST_STRING;
    if (str == "bool") return TokenType::CONST_BOOL;
    if (str == "void") return TokenType::CONST_VOID;
    throw OurException(ExceptionType::EMPTY, "Interpreter::String_To_TokenType()", 0);
  }

  bool IsSystemFunctionName(Token token) {
    if (token.value == "Done") return true;
    if (token.value == "ListAllVariables") return true;
    if (token.value == "ListAllFunctions") return true;
    if (token.value == "ListVariable") return true;
    if (token.value == "ListFunction") return true;
    return false;
  }

  bool IsKeyWord(Token token) {
    for (auto keyword : KEYWORD_DICTIONARY) {
      if (token.value == keyword) return true;
    }
    return false;
  }
};

enum class NodeType {
  Empty,  // 預設值
  LeafNode,  // 葉節點，用來存input token
  // 文法 : children定義
  user_input,  // 0:definition | statement
  definition,  // (0:VOID / 1:Identifier / 2:function_definition_without_ID) || (0:type_specifier / 1:Identifier / 2:function_definition_or_declarators)
  type_specifier,
  function_definition_or_declarators,
  rest_of_declarators,  // 
  function_definition_without_ID,
  formal_parameter_list,
  compound_statement,
  declaration,
  statement,
  expression,
  basic_expression,
  rest_of_Identifier_started_basic_exp,
  rest_of_PPMM_Identifier_started_basic_exp,
  sign,
  actual_parameter_list,
  assignment_operator,
  romce_and_romloe,
  rest_of_maybe_logical_OR_exp,
  maybe_logical_AND_exp,
  rest_of_maybe_logical_AND_exp,
  maybe_bit_OR_exp,
  rest_of_maybe_bit_OR_exp,
  maybe_bit_ex_OR_exp,
  rest_of_maybe_bit_ex_OR_exp,
  maybe_bit_AND_exp,
  rest_of_maybe_bit_AND_exp,
  maybe_equality_exp,
  rest_of_maybe_equality_exp,
  maybe_relational_exp,
  rest_of_maybe_relational_exp,
  maybe_shift_exp,
  rest_of_maybe_shift_exp,
  maybe_additive_exp,
  rest_of_maybe_additive_exp,
  maybe_mult_exp,
  rest_of_maybe_mult_exp,
  unary_exp,
  signed_unary_exp,
  unsigned_unary_exp
};

class ParserTreeNode {
 public:
  NodeType type;
  Token token;  
  vector<ParserTreeNode *> children;
  ParserTreeNode(NodeType type, Token token = Token()) {
    this->type = type;
    this->token = token;
  }
  ~ParserTreeNode() {
    for (auto child : children) { 
      delete child; 
    }
  }
  void AddChild(ParserTreeNode * parser_tree) {
    this->children.push_back(parser_tree);
  }
};

enum class DataType {
  EMPTY,  // 預設值
  SingleVariable,
  Array,
  Function
};

class MemoryData {
 public:
  DataType type;  // 資料型別

  // SingleVariable
  Token token;  
  // Array
  vector<Token> array_tokens; 
  TokenType array_type = TokenType::EMPTY;
  int array_size = 0;
  // Function
  ParserTreeNode * function = nullptr;  
  // 附加型別(&)：Reference
  bool is_reference = false;  
  MemoryData * reference_target = nullptr;  
  int reference_index = -1;  // -1: 表示單一變數 or 整個Array ； >=0: 表示Array的某element

  MemoryData(Token token) {
    this->type = DataType::SingleVariable;
    this->token = token;
  }
  MemoryData(TokenType array_type, int array_size) {
    this->type = DataType::Array;
    this->array_type = array_type;
    this->array_size = array_size;
    array_tokens.resize(array_size);
  }
  MemoryData(ParserTreeNode * function) {
    this->type = DataType::Function;
    this->function = function;
  }
  MemoryData(MemoryData * target, int index = -1) {
    this->type = target->type;
    this->is_reference = true;
    this->reference_target = target;
    this->reference_index = index;
  }
};

class Memory {
 private:
  vector<map<string, MemoryData*>> scopes;  // 用stack型式記憶體實作scopes：index 0 是 Global Scope
  vector<vector<map<string, MemoryData*>>> function_call_scopes;  // for function call

  // 在當前scope宣告新變數 or 函數 || 複寫已宣告的變數值
  string Write(string name, MemoryData * data) {
    string result_message;
    if (scopes.back().count(name) > 0) {  // 當前scope有此名稱變數，先刪掉舊指標再宣告，避免memory leak
      delete scopes.back()[name];  
      scopes.back()[name] = data;
      result_message = "New definition of " + name + " entered ...\n";
    } else {
      scopes.back()[name] = data;
      result_message = "Definition of " + name + " entered ...\n";
    }
    if (scopes.size() == 1) return result_message;  // // 只有Global Definition 需要 Print message
    else return "";
  }

  // 回傳 指向記憶體位置的指標
  MemoryData * Read(string name) {
    for (int t = scopes.size() - 1; t >= 0; t--) {
      if (scopes[t].count(name) > 0) {  // 若找到，會 == 1
        return scopes[t][name];
      }
    }
    return nullptr;  // 找不到：Undefined identifier
  }

  string DefaultValue(TokenType token_type) {
    if (token_type == TokenType::CONST_INT) return "0";
    if (token_type == TokenType::CONST_FLOAT) return "0";
    if (token_type == TokenType::CONST_BOOL) return "false";
    if (token_type == TokenType::CONST_CHAR) return "\0";
    if (token_type == TokenType::CONST_STRING) return "";
    return "";
  }

 public:
  Memory() {
    scopes.push_back(map<string, MemoryData*>());  // 初始化，建立第一個 map 作為全域作用域 (Global Scope)
  }

  // 建立新的一層local scope，例如進入 {} 或 function call
  void EnterScope() {
    scopes.push_back(map<string, MemoryData*>());
  }

  // 刪除目前local scope，例如退出 {} 或 function call的return
  void LeaveScope() {
    if (scopes.size() > 1) {
      for (auto [name, memory_data] : scopes.back()) {
        delete memory_data; 
      }
      scopes.pop_back();
      scopes.shrink_to_fit();  // 確保釋放記憶體
    }
  }

  void EnterFunctionScope() {
    vector<map<string, MemoryData*> > caller_locals;
    for (int i = 1; i < (int)scopes.size(); i++) {
      caller_locals.push_back(scopes[i]);
    }
    if (scopes.size() > 1) {
      scopes.erase(scopes.begin() + 1, scopes.end());
    }
    function_call_scopes.push_back(caller_locals);
    scopes.push_back(map<string, MemoryData*>());
  }

  void LeaveFunctionScope() {
    if (scopes.size() > 1) {
      for (auto [name, memory_data] : scopes.back()) {
        delete memory_data;
      }
      scopes.pop_back();
    }
    if (!function_call_scopes.empty()) {
      vector<map<string, MemoryData*> > caller_locals = function_call_scopes.back();
      function_call_scopes.pop_back();
      for (auto scope : caller_locals) {
        scopes.push_back(scope);
      }
    }
  }

  bool IsDeclaredSingleVariable(string name) {
    MemoryData * data = Read(name);
    if (data == nullptr) return false;
    return data->type == DataType::SingleVariable;
  }

  bool IsDeclaredArray(string name) {
    MemoryData * data = Read(name);
    if (data == nullptr) return false;
    return data->type == DataType::Array;
  }

  bool IsDeclaredFunction(string name) {
    MemoryData * data = Read(name + "()");
    if (data == nullptr) return false;
    return data->type == DataType::Function;
  }

  // Write()會回傳memory操作訊息
  string WriteSingleVariable(string name, Token token) {
    return Write(name, new MemoryData(token));
  }

  string WriteArray(string name, TokenType token_type, int array_size) {
    return Write(name, new MemoryData(token_type, array_size));
  }

  string WriteArrayIndex(string name, int array_index, Token token) {
    MemoryData * data = Read(name);  
    data->array_tokens[array_index] = token;
    return "";
  }

  string WriteFunction(string name, ParserTreeNode * function) {
    return Write(name + "()", new MemoryData(function));
  }

  string WriteReference(string name, MemoryData * target, int index = -1) {
    return Write(name, new MemoryData(target, index));
  }

  string Write_WithDefaultValue(string name, DataType data_type, TokenType token_type, int array_size = 0) { 
    if (data_type == DataType::SingleVariable) {
      return WriteSingleVariable(name, Token(token_type, DefaultValue(token_type)));
    } else if (data_type == DataType::Array) {
      WriteArray(name, token_type, array_size); 
      for (int index = 0; index < GetArraySize(name); index++) {
        WriteArrayIndex(name, index, Token(token_type, DefaultValue(token_type)));
      }
    }
    return "";  // for compiler  // 不會有需要系統自動定義function的狀況
  }

  Token GetSingleVariable(string name) {
    MemoryData * data = Read(name);
    return data->token;
  }

  vector<Token> GetArray(string name) {
    MemoryData * data = Read(name);
    return data->array_tokens;
  }

  TokenType GetArrayType(string name) {
    MemoryData * data = Read(name);
    return data->array_type;
  }

  int GetArraySize(string name) {
    MemoryData * data = Read(name);
    return data->array_size;
  }

  Token GetArrayIndex(string name, int array_index) {
    MemoryData * data = Read(name);
    return data->array_tokens[array_index];
  }

  ParserTreeNode * GetFunction(string name) {
    MemoryData * data = Read(name + "()");
    return data->function;
  }

  MemoryData * GetRaw(string name) {
    return Read(name);
  }

  vector<string> GetAllVariablesName() {  // global
    vector<string> result;
    for (auto [name, data] : scopes[0]) {
      if (data->type == DataType::SingleVariable || data->type == DataType::Array) {
        result.push_back(name);
      }
    }
    return result;
  }

  vector<string> GetAllFunctionsName() {  // global
    vector<string> result;
    for (auto [name, data] : scopes[0]) {
      if (data->type == DataType::Function) {
        result.push_back(name);
      }
    }
    return result;
  }

  ~Memory() {
    for (auto scope : scopes) {
      for (auto [name, data] : scope) {
        delete data;
      }
    }
  }
};

class Parser {
 private:
  Scanner scanner;          // Parser 內建 Scanner
  Memory * memory;          // 由Interpreter提供
  vector<Token> token_log;  // 紀錄當前指令讀過的所有token，用於檢查identifier的 SEMANTIC_ERR
  Tools tools;  // 相關工具

  // 用於實作PeekToken()
  Token lookahead;
  bool has_lookahead = false;

  bool keep_lookahead_after_error = false;  // 修P2 SEMANTIC_ERR後換行token的bug  a \n ;
  vector<string> function_parse_stack;  // P4: 允許 function 內遞迴呼叫自己

  // 必須先PeekToken 再 GetToken，才能正確判斷SEMANTIC_ERR
  Token GetToken() {
    if (has_lookahead) {
      has_lookahead = false;
      token_log.push_back(lookahead);
      return lookahead;
    }
    Token next_token = scanner.GetNextToken();
    token_log.push_back(next_token);
    if (next_token.type == TokenType::END_OF_FILE) throw OurException(ExceptionType::END_OF_FILE, "", 0);  // 讀到EOF
    return next_token;
  }

  Token PeekToken() {
    if (!has_lookahead) {
      Token input_token = scanner.GetNextToken();
      if (input_token.type == TokenType::END_OF_FILE) throw OurException(ExceptionType::END_OF_FILE, "", 0);
      lookahead = input_token;
      has_lookahead = true;
      try {
        CheckIdentIsDefined();
      } catch (OurException & e) {
        if (e.error_type == ExceptionType::SEMANTIC_ERR) {
          Token ident_token = token_log[token_log.size() - 1];
          if (lookahead.line > ident_token.line) keep_lookahead_after_error = true;
          else keep_lookahead_after_error = false;
        }
        throw;
      }
    }
    return lookahead;
  }

  // 用於檢查 SEMANTIC_ERR  
  bool Is_DefinedVariable(Token token) {
    if (memory->IsDeclaredSingleVariable(token.value)) return true;  // 查memory是否有定義
    if (memory->IsDeclaredArray(token.value)) return true;           // 查memory是否有定義
    return false;
  }
  bool Is_DefinedFunction(Token token) {
    if (memory->IsDeclaredFunction(token.value)) return true;  // 查memory是否有定義
    for (string name : function_parse_stack) {
      if (name == token.value) return true;  // 允許遞迴呼叫正在定義的 function
    }
    return false;
  }
  bool Is_Declaring() {
    if (token_log.size() <= 1) return false;
    int pre1 = token_log.size() - 2; // ident 前一個 token
    // 情況1：int x / float x / string x / void F()
    if (Is_type_specifier(token_log[pre1]) || token_log[pre1].value == "void") {
      return true;
    }
    // 情況2：int & x
    if (token_log[pre1].value == "&" && pre1 - 1 >= 0 && Is_type_specifier(token_log[pre1 - 1])) {
      return true;
    }
    // 情況3：int a, b, c;
    if (token_log[pre1].value == ",") {
      for (int t = pre1 - 1; t >= 0; t--) {
        string v = token_log[t].value;
        if (v == ";" || v == "{" || v == "}" || v == ")" || v == "return") return false;  // 遇到這些就代表不是同一個宣告區間了
        if (Is_type_specifier(token_log[t]) || v == "void") return true;
      }
    }
    return false;
  }
  void CheckIdentIsDefined() {
    if (token_log.size() >= 1) {
      Token ident_token = token_log[token_log.size() - 1];
      if (ident_token.type != TokenType::IDENT) return;
      // 是否為OurC保留字(if, else, int ...) 且 非OurC系統function
      if (tools.IsKeyWord(ident_token) && tools.IsSystemFunctionName(ident_token) == false) return;
      // OurC system function：只有後面接 '(' 時才通過SEMANTIC_ERR
      if (tools.IsSystemFunctionName(ident_token)) {
        if (lookahead.value == "(") return;  // 若沒接 '('，繼續往下當普通變數檢查
      }
      // 是否為使用者定義function
      if (lookahead.value == "(") {
        if (Is_DefinedFunction(ident_token) == false && Is_Declaring() == false) {
          throw OurException(ExceptionType::SEMANTIC_ERR, ident_token);
        }
      // 是否為使用者定義variable
      } else {
        if (Is_DefinedVariable(ident_token) == false && Is_Declaring() == false) {
          throw OurException(ExceptionType::SEMANTIC_ERR, ident_token);
        }
      }
    }
  }
  
  // Parser假執行用：宣告一般變數，讓後續token的SEMANTIC_ERR檢查查得到
  void FakeDeclareVariable(const string & name, const string & type_name) {
    memory->Write_WithDefaultValue(name, DataType::SingleVariable, tools.String_To_TokenType(type_name));
  }
  // Parser假執行用：宣告陣列，讓後續token的SEMANTIC_ERR檢查查得到
  void FakeDeclareArray(const string & name, const string & type_name, int array_size) {
    memory->Write_WithDefaultValue(name, DataType::Array, tools.String_To_TokenType(type_name), array_size);
  }
  // Parser假執行不需要宣告Function，也不會在function內宣告function

  // =====================================================================
  // ============================== 所有文法 ==============================
  // 輔助文法判斷
  bool Is_type_specifier(Token token) {
    if (token.value == "int") return true;
    if (token.value == "char") return true;
    if (token.value == "float") return true;
    if (token.value == "string") return true;
    if (token.value == "bool") return true;
    return false;
  }

  bool Is_sign(Token token) {
    if (token.value == "+" || token.value == "-" || token.value == "!") return true;
    return false;
  }

  bool Is_Constant(Token token) {
    if (token.type == TokenType::CONST_BOOL) return true;
    if (token.type == TokenType::CONST_CHAR) return true;
    if (token.type == TokenType::CONST_FLOAT) return true;
    if (token.type == TokenType::CONST_INT) return true;
    if (token.type == TokenType::CONST_STRING) return true;
    return false;
  }

  bool Is_assignment_operator(Token token) {
    if (token.value == "=") return true;
    if (token.value == "*=") return true;
    if (token.value == "/=") return true;
    if (token.value == "%=") return true;
    if (token.value == "+=") return true;
    if (token.value == "-=") return true;
    return false;
  }

  bool Is_basic_expression(Token token) {
    if (PeekToken().type == TokenType::IDENT) return true;
    if (PeekToken().value == "++" || PeekToken().value == "--") return true;
    if (Is_sign(PeekToken())) return true;
    if (Is_Constant(PeekToken()) || PeekToken().value == "(") return true;
    return false;
  }

  bool Is_expression(Token token) {
    return Is_basic_expression(token);
  }

  bool Is_actual_parameter_list(Token token) {
    return Is_basic_expression(token);
  }

  // LeafNode輔助文法 
  ParserTreeNode * Identifier() {
    // 可以 int cin 但不能 int if
    if (PeekToken().type == TokenType::IDENT && 
      (!tools.IsKeyWord(PeekToken()) || tools.IsSystemFunctionName(PeekToken()) || PeekToken().value == "cin" || PeekToken().value == "cout")) {
      return new ParserTreeNode(NodeType::LeafNode, GetToken());
    } else {
      throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    }
  }

  ParserTreeNode * Constant() {
    Token token = GetToken();
    if (token.type == TokenType::CONST_BOOL || token.type == TokenType::CONST_CHAR || token.type == TokenType::CONST_FLOAT ||
        token.type == TokenType::CONST_INT  || token.type == TokenType::CONST_STRING) {
      return new ParserTreeNode(NodeType::LeafNode, token);
    } else {
      throw OurException(ExceptionType::SYNTACTIC_ERR, token);
    }
  }

  // 官方文法
  ParserTreeNode * definition() {  // VOID Identifier function_definition_without_ID | type_specifier Identifier function_definition_or_declarators 
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::definition);
    if (PeekToken().value == "void") {  // VOID Identifier function_definition_without_ID
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(Identifier());
      string function_name = parser_tree->children[1]->token.value;
      function_parse_stack.push_back(function_name);
      try {
        parser_tree->AddChild(function_definition_without_ID());
        function_parse_stack.pop_back();
      } catch (...) {
        function_parse_stack.pop_back();
        delete parser_tree;
        throw;
      }
    } else {  // type_specifier Identifier function_definition_or_declarators 
      parser_tree->AddChild(type_specifier());
      parser_tree->AddChild(Identifier());
      if (PeekToken().value != "(") {
        FakeDeclareVariable(parser_tree->children[1]->token.value, parser_tree->children[0]->children[0]->token.value);
        parser_tree->AddChild(function_definition_or_declarators(parser_tree->children[0]->children[0]->token.value, parser_tree->children[1]->token.value));
      } else {
        string function_name = parser_tree->children[1]->token.value;
        function_parse_stack.push_back(function_name);
        try {
          parser_tree->AddChild(function_definition_or_declarators(parser_tree->children[0]->children[0]->token.value, parser_tree->children[1]->token.value));
          function_parse_stack.pop_back();
        } catch (...) {
          function_parse_stack.pop_back();
          delete parser_tree;
          throw;
        }
      }
    }
    return parser_tree;
  }

  ParserTreeNode * type_specifier() {  // INT | CHAR | FLOAT | STRING | BOOL 
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::type_specifier);
    if (Is_type_specifier(PeekToken())) {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
    } else {
      throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    }
    return parser_tree;
  }
  
  ParserTreeNode * function_definition_or_declarators(string type_name, string first_name) {  // function_definition_without_ID | rest_of_declarators
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::function_definition_or_declarators);
    if (PeekToken().value == "(") {
      parser_tree->AddChild(function_definition_without_ID());
    } else {
      parser_tree->AddChild(rest_of_declarators(type_name, first_name));  
    }
    return parser_tree;
  }

  ParserTreeNode * rest_of_declarators(string type_name, string first_name) {  // [ '[' Constant ']' ] { ',' Identifier [ '[' Constant ']' ] } ';'
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_declarators);
    if (PeekToken().value == "[") {  // [ '[' Constant ']' ]
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(Constant());
      FakeDeclareArray(first_name, type_name, stoi(parser_tree->children[1]->token.value));  // Parser 假執行宣告 Array
      if (PeekToken().value == "]") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    }
    while (PeekToken().value == ",") {  // { ',' Identifier [ '[' Constant ']' ] } ';'
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      ParserTreeNode * ident_node = Identifier();
      parser_tree->AddChild(ident_node);
      FakeDeclareVariable(ident_node->token.value, type_name);  // Parser 假執行宣告 Variable
      if (PeekToken().value == "[") {  // [ '[' Constant ']' ]
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        ParserTreeNode * size_node = Constant();
        parser_tree->AddChild(size_node);
        FakeDeclareArray(ident_node->token.value, type_name, stoi(size_node->token.value));  // Parser 假執行宣告 Array
        if (PeekToken().value == "]") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
    }
    if (PeekToken().value == ";") {  // ';'
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
    } else {
      throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    }
    return parser_tree;
  }

  ParserTreeNode * function_definition_without_ID() {  // '(' [ VOID | formal_parameter_list ] ')' compound_statement 
    memory->EnterScope();  // Parser 假執行 新增scope
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::function_definition_without_ID);
    try {
      if (PeekToken().value == "(") { 
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        if (PeekToken().value == "void") {
          parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        } else if (Is_type_specifier(PeekToken())) {
          parser_tree->AddChild(formal_parameter_list());
        }
        if (PeekToken().value == ")") {
          parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        } else {
          throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
        }
        parser_tree->AddChild(compound_statement());
      } else {
        throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
      memory->LeaveScope();  // Parser 假執行 退出scope
      return parser_tree;
    } catch (...) {
      memory->LeaveScope();  // Parser 假執行 退出scope
      delete parser_tree;
      throw;
    }
  }

  ParserTreeNode * formal_parameter_list() {  // type_specifier [ '&' ] Identifier [ '[' Constant ']' ] { ',' type_specifier [ '&' ] Identifier [ '[' Constant ']' ] }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::formal_parameter_list);
    while (true) {
      ParserTreeNode * type_node = type_specifier();
      string type_name = type_node->children[0]->token.value;
      parser_tree->AddChild(type_node);
      if (PeekToken().value == "&") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      }
      ParserTreeNode * ident_node = Identifier();
      parser_tree->AddChild(ident_node);
      FakeDeclareVariable(ident_node->token.value, type_name);  // Parser 假執行宣告 Variable 
      if (PeekToken().value == "[") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        ParserTreeNode * size_node = Constant();
        parser_tree->AddChild(size_node);
        FakeDeclareArray(ident_node->token.value, type_name, stoi(size_node->token.value));  // Parser 假執行宣告 Array
        if (PeekToken().value == "]") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
      if (PeekToken().value == ",") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      else break;
    }
    return parser_tree;
  }

  ParserTreeNode * compound_statement() {  // '{' { declaration | statement } '}'
    memory->EnterScope();
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::compound_statement);
    try {
      if (PeekToken().value == "{") { 
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        while (PeekToken().value != "}") {
          if (Is_type_specifier(PeekToken())) {
            parser_tree->AddChild(declaration());
          } else {
            parser_tree->AddChild(statement());
          }
        }
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));  // }
      } else {
        throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
      memory->LeaveScope();
      return parser_tree;
    } catch(...) {
      memory->LeaveScope();
      delete parser_tree;
      throw;
    }
  }

  ParserTreeNode * declaration () {  //  type_specifier Identifier rest_of_declarators
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::declaration);
    parser_tree->AddChild(type_specifier());
    parser_tree->AddChild(Identifier());
    FakeDeclareVariable(parser_tree->children[1]->token.value, parser_tree->children[0]->children[0]->token.value);  // Parser 假執行宣告 Variable  
    parser_tree->AddChild(rest_of_declarators(parser_tree->children[0]->children[0]->token.value, parser_tree->children[1]->token.value));
    return parser_tree;
  }

  ParserTreeNode * statement() {  // ';' | expression ';' | RETURN [ expression ] ';' | compound_statement | IF '(' expression ')' statement [ ELSE statement ] | WHILE '(' expression ')' statement | DO statement WHILE '(' expression ')' ';' 
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::statement);
    if (PeekToken().value == ";") {  // ';'
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
    } else if (PeekToken().value == "return") {  // RETURN [ expression ] ';'
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      if (PeekToken().value != ";") {
        parser_tree->AddChild(expression());
      }
      if (PeekToken().value == ";") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    } else if (PeekToken().value == "{") {  // compound_statement
      parser_tree->AddChild(compound_statement());
    } else if (PeekToken().value == "if") {  // IF '(' expression ')' statement [ ELSE statement ]
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      if (PeekToken().value == "(") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        parser_tree->AddChild(expression());
        if (PeekToken().value == ")") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      } else {
        throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
      parser_tree->AddChild(statement());
      if (PeekToken().value == "else") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        parser_tree->AddChild(statement());
      }
    } else if (PeekToken().value == "while") {  // WHILE '(' expression ')' statement
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      if (PeekToken().value == "(") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        parser_tree->AddChild(expression());
        if (PeekToken().value == ")") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      } else {
        throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
      parser_tree->AddChild(statement());
    } else if (PeekToken().value == "do") {  // DO statement WHILE '(' expression ')' ';'
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(statement());
      if (PeekToken().value == "while") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        if (PeekToken().value == "(") {
          parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
          parser_tree->AddChild(expression());
          if (PeekToken().value == ")") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
          else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
        } else {
          throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
        }
        if (PeekToken().value == ";") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      } else {
        throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
    } else if (PeekToken().value == "else") {  // 單獨的 else 為錯誤
      throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    } else if (Is_expression(PeekToken())) {  // expression ';' 文法順序移動到這，為了避免P2 9-2錯誤。
      parser_tree->AddChild(expression());
      if (PeekToken().value == ";") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    } else {
      throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    }
    return parser_tree;
  }

  ParserTreeNode * expression() {  // basic_expression { ',' basic_expression }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::expression);
    parser_tree->AddChild(basic_expression());
    while (PeekToken().value == ",") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(basic_expression());
    }
    return parser_tree;
  }

  ParserTreeNode * basic_expression() {  // Identifier rest_of_Identifier_started_basic_exp | ( PP | MM ) Identifier rest_of_PPMM_Identifier_started_basic_exp | sign { sign } signed_unary_exp romce_and_romloe | ( Constant | '(' expression ')' ) romce_and_romloe
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::basic_expression);
    if (PeekToken().type == TokenType::IDENT) {  // Identifier rest_of_Identifier_started_basic_exp
      parser_tree->AddChild(Identifier());  
      parser_tree->AddChild(rest_of_Identifier_started_basic_exp());
    } else if (PeekToken().value == "++" || PeekToken().value == "--") {  // ( PP | MM ) Identifier rest_of_PPMM_Identifier_started_basic_exp
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(Identifier());
      parser_tree->AddChild(rest_of_PPMM_Identifier_started_basic_exp());
    } else if (Is_sign(PeekToken())) {  // sign { sign } signed_unary_exp romce_and_romloe
      parser_tree->AddChild(sign());
      while (Is_sign(PeekToken())) {
        parser_tree->AddChild(sign());
      }
      parser_tree->AddChild(signed_unary_exp());
      parser_tree->AddChild(romce_and_romloe());
    } else if (Is_Constant(PeekToken()) || PeekToken().value == "(") {  // ( Constant | '(' expression ')' ) romce_and_romloe
      if (Is_Constant(PeekToken())) {
        parser_tree->AddChild(Constant());
      } else {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        parser_tree->AddChild(expression());
        if (PeekToken().value == ")") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
      parser_tree->AddChild(romce_and_romloe());
    } else {
      throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    }
    return parser_tree;
  }

  // 文法判斷順序顛倒，不確定是否會錯，應該不會
  ParserTreeNode * rest_of_Identifier_started_basic_exp() {  // [ '[' expression ']' ] ( assignment_operator basic_expression | [ PP | MM ] romce_and_romloe ) | '(' [ actual_parameter_list ] ')' romce_and_romloe
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_Identifier_started_basic_exp);
    if (PeekToken().value == "(") {  // '(' [ actual_parameter_list ] ')' romce_and_romloe
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      if (Is_actual_parameter_list(PeekToken())) parser_tree->AddChild(actual_parameter_list());
      if (PeekToken().value == ")") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        parser_tree->AddChild(romce_and_romloe());
      } else {
        throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
    } else {  // [ '[' expression ']' ] ( assignment_operator basic_expression | [ PP | MM ] romce_and_romloe )
      if (PeekToken().value == "[") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        parser_tree->AddChild(expression());
        if (PeekToken().value == "]") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
      if (Is_assignment_operator(PeekToken())) {  // assignment_operator basic_expression
        parser_tree->AddChild(assignment_operator());
        parser_tree->AddChild(basic_expression());
      } else {  // [ PP | MM ] romce_and_romloe
        if (PeekToken().value == "++" || PeekToken().value == "--") {
          parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        }
        parser_tree->AddChild(romce_and_romloe());
      }
    }
    return parser_tree;
  }

  ParserTreeNode * rest_of_PPMM_Identifier_started_basic_exp() {  // [ '[' expression ']' ] romce_and_romloe
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_PPMM_Identifier_started_basic_exp);
    if (PeekToken().value == "[") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(expression());
      if (PeekToken().value == "]") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    }
    parser_tree->AddChild(romce_and_romloe());
    return parser_tree;
  }

  ParserTreeNode * sign() {  // '+' | '-' | '!'
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::sign);
    if (Is_sign(PeekToken())) {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
    } else {
      throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    }
    return parser_tree;
  }

  ParserTreeNode * actual_parameter_list() {  // basic_expression { ',' basic_expression }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::actual_parameter_list);
    parser_tree->AddChild(basic_expression());
    while (PeekToken().value == ",") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(basic_expression());
    }
    return parser_tree;
  }

  ParserTreeNode * assignment_operator() {  // '=' | TE | DE | RE | PE | ME
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::assignment_operator);
    if (Is_assignment_operator(PeekToken())) parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
    else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    return parser_tree;
  }

  ParserTreeNode * romce_and_romloe() {  // rest_of_maybe_logical_OR_exp [ '?' basic_expression ':' basic_expression ]
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::romce_and_romloe);
    parser_tree->AddChild(rest_of_maybe_logical_OR_exp());
    if (PeekToken().value == "?") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(basic_expression());
      if (PeekToken().value == ":") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      parser_tree->AddChild(basic_expression());
    }
    return parser_tree;
  }

  ParserTreeNode * rest_of_maybe_logical_OR_exp() {  // rest_of_maybe_logical_AND_exp { OR maybe_logical_AND_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_maybe_logical_OR_exp);
    parser_tree->AddChild(rest_of_maybe_logical_AND_exp());
    while (PeekToken().value == "||") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_logical_AND_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * maybe_logical_AND_exp() {  // maybe_bit_OR_exp { AND maybe_bit_OR_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::maybe_logical_AND_exp);
    parser_tree->AddChild(maybe_bit_OR_exp());
    while (PeekToken().value == "&&") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_bit_OR_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * rest_of_maybe_logical_AND_exp() {  // rest_of_maybe_bit_OR_exp { AND maybe_bit_OR_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_maybe_logical_AND_exp);
    parser_tree->AddChild(rest_of_maybe_bit_OR_exp());
    while (PeekToken().value == "&&") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_bit_OR_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * maybe_bit_OR_exp() {  // maybe_bit_ex_OR_exp { '|' maybe_bit_ex_OR_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::maybe_bit_OR_exp);
    parser_tree->AddChild(maybe_bit_ex_OR_exp());
    while (PeekToken().value == "|") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_bit_ex_OR_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * rest_of_maybe_bit_OR_exp() {  // rest_of_maybe_bit_ex_OR_exp { '|' maybe_bit_ex_OR_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_maybe_bit_OR_exp);
    parser_tree->AddChild(rest_of_maybe_bit_ex_OR_exp());
    while (PeekToken().value == "|") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_bit_ex_OR_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * maybe_bit_ex_OR_exp() {  // maybe_bit_AND_exp { '^' maybe_bit_AND_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::maybe_bit_ex_OR_exp);
    parser_tree->AddChild(maybe_bit_AND_exp());
    while (PeekToken().value == "^") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_bit_AND_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * rest_of_maybe_bit_ex_OR_exp() {  // rest_of_maybe_bit_AND_exp { '^' maybe_bit_AND_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_maybe_bit_ex_OR_exp);
    parser_tree->AddChild(rest_of_maybe_bit_AND_exp());
    while (PeekToken().value == "^") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_bit_AND_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * maybe_bit_AND_exp() {  // maybe_equality_exp { '&' maybe_equality_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::maybe_bit_AND_exp);
    parser_tree->AddChild(maybe_equality_exp());
    while (PeekToken().value == "&") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_equality_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * rest_of_maybe_bit_AND_exp() {  // rest_of_maybe_equality_exp { '&' maybe_equality_exp } 
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_maybe_bit_AND_exp);
    parser_tree->AddChild(rest_of_maybe_equality_exp());
    while (PeekToken().value == "&") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_equality_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * maybe_equality_exp() {  // maybe_relational_exp { ( EQ | NEQ ) maybe_relational_exp} 
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::maybe_equality_exp);
    parser_tree->AddChild(maybe_relational_exp());
    while (PeekToken().value == "==" || PeekToken().value == "!=") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_relational_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * rest_of_maybe_equality_exp() {  // rest_of_maybe_relational_exp { ( EQ | NEQ ) maybe_relational_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_maybe_equality_exp);
    parser_tree->AddChild(rest_of_maybe_relational_exp());
    while (PeekToken().value == "==" || PeekToken().value == "!=") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_relational_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * maybe_relational_exp() {  // maybe_shift_exp { ( '<' | '>' | LE | GE ) maybe_shift_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::maybe_relational_exp);
    parser_tree->AddChild(maybe_shift_exp());
    while (PeekToken().value == "<" || PeekToken().value == ">" || PeekToken().value == "<=" || PeekToken().value == ">=") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_shift_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * rest_of_maybe_relational_exp() {  // rest_of_maybe_shift_exp { ( '<' | '>' | LE | GE ) maybe_shift_exp } 
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_maybe_relational_exp);
    parser_tree->AddChild(rest_of_maybe_shift_exp());
    while (PeekToken().value == "<" || PeekToken().value == ">" || PeekToken().value == "<=" || PeekToken().value == ">=") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_shift_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * maybe_shift_exp() {  // maybe_additive_exp { ( LS | RS ) maybe_additive_exp } 
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::maybe_shift_exp);
    parser_tree->AddChild(maybe_additive_exp());
    while (PeekToken().value == "<<" || PeekToken().value == ">>") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_additive_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * rest_of_maybe_shift_exp() {  // rest_of_maybe_additive_exp { ( LS | RS ) maybe_additive_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_maybe_shift_exp);
    parser_tree->AddChild(rest_of_maybe_additive_exp());
    while (PeekToken().value == "<<" || PeekToken().value == ">>") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_additive_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * maybe_additive_exp() {  // maybe_mult_exp { ( '+' | '-' ) maybe_mult_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::maybe_additive_exp);
    parser_tree->AddChild(maybe_mult_exp());
    while (PeekToken().value == "+" || PeekToken().value == "-") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_mult_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * rest_of_maybe_additive_exp() {  // rest_of_maybe_mult_exp { ( '+' | '-' ) maybe_mult_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_maybe_additive_exp);
    parser_tree->AddChild(rest_of_maybe_mult_exp());
    while (PeekToken().value == "+" || PeekToken().value == "-") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(maybe_mult_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * maybe_mult_exp() {  // unary_exp rest_of_maybe_mult_exp
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::maybe_mult_exp);
    parser_tree->AddChild(unary_exp());
    parser_tree->AddChild(rest_of_maybe_mult_exp());
    return parser_tree;
  }

  ParserTreeNode * rest_of_maybe_mult_exp() {  // { ( '*' | '/' | '%' ) unary_exp }
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::rest_of_maybe_additive_exp);
    while (PeekToken().value == "*" || PeekToken().value == "/" || PeekToken().value == "%") {
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(unary_exp());
    }
    return parser_tree;
  }

  ParserTreeNode * unary_exp() {  // sign { sign } signed_unary_exp | unsigned_unary_exp | ( PP | MM ) Identifier [ '[' expression ']' ]
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::unary_exp);
    if (Is_sign(PeekToken())) {  // sign { sign } signed_unary_exp
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      while (Is_sign(PeekToken())) {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      }
      parser_tree->AddChild(signed_unary_exp());
    } else if (PeekToken().type == TokenType::IDENT || Is_Constant(PeekToken()) || PeekToken().value == "(") {  // unsigned_unary_exp
      parser_tree->AddChild(unsigned_unary_exp());
    } else {  // ( PP | MM ) Identifier [ '[' expression ']' ]
      if (PeekToken().value == "++" || PeekToken().value == "--") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      }
      parser_tree->AddChild(Identifier());
      if (PeekToken().value == "[") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        parser_tree->AddChild(expression());
        if (PeekToken().value == "]") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
    }
    return parser_tree;
  }

  ParserTreeNode * signed_unary_exp() {  // Identifier [ '(' [ actual_parameter_list ] ')' | '[' expression ']' ] | Constant | '(' expression ')'
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::signed_unary_exp);
    if (PeekToken().type == TokenType::IDENT) {  // Identifier [ '(' [ actual_parameter_list ] ')' | '[' expression ']' ]
      parser_tree->AddChild(Identifier());
      if (PeekToken().value == "(") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        if (Is_actual_parameter_list(PeekToken())) {
          parser_tree->AddChild(actual_parameter_list());
        }
        if (PeekToken().value == ")") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      } else if (PeekToken().value == "[") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        parser_tree->AddChild(expression());
        if (PeekToken().value == "]") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      }
    } else if (Is_Constant(PeekToken())) {  // Constant
      parser_tree->AddChild(Constant());
    } else if (PeekToken().value == "(") {  // '(' expression ')'
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(expression());
      if (PeekToken().value == ")") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    } else {
      throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    }
    return parser_tree;
  }

  ParserTreeNode * unsigned_unary_exp() {  // Identifier [ '(' [ actual_parameter_list ] ')' | [ '[' expression ']' ] [ ( PP | MM ) ] ] | Constant | '(' expression ')' 
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::unsigned_unary_exp);
    if (PeekToken().type == TokenType::IDENT) {  // Identifier [ '(' [ actual_parameter_list ] ')' | [ '[' expression ']' ] [ ( PP | MM ) ] ]
      parser_tree->AddChild(Identifier());
      if (PeekToken().value == "(") {
        parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        if (Is_actual_parameter_list(PeekToken())) {
          parser_tree->AddChild(actual_parameter_list());
        }
        if (PeekToken().value == ")") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
      } else {
        if (PeekToken().value == "[") {
          parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
          parser_tree->AddChild(expression());
          if (PeekToken().value == "]") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
          else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
        }
        if (PeekToken().value == "++" || PeekToken().value == "--") {
          parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
        }
      }
    } else if (Is_Constant(PeekToken())) {  // Constant
      parser_tree->AddChild(Constant());
    } else if (PeekToken().value == "(") {  // '(' expression ')'
      parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      parser_tree->AddChild(expression());
      if (PeekToken().value == ")") parser_tree->AddChild(new ParserTreeNode(NodeType::LeafNode, GetToken()));
      else throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    } else {
      throw OurException(ExceptionType::SYNTACTIC_ERR, GetToken());
    }
    return parser_tree;
  }
  // ============================== 所有文法 ==============================
  // =====================================================================
 public:
  Parser() {
    throw OurException(ExceptionType::EMPTY, "Parser::Parser()", 0);  // Interpreter必須傳memory給Parser
  }

  Parser(Memory * memory) {
    this->memory = memory;
  }

  // 文法入口，會throw OurException 給 Interpreter 處理
  ParserTreeNode * user_input() {  // definition | statement
    Clear();  // 初始化
    memory->EnterScope();  // {...} 被視為一道指令，為了正常檢查Identifider，需要在Parser時建立一層假執行memory scope
    ParserTreeNode * parser_tree = new ParserTreeNode(NodeType::user_input);
    try {
      if (PeekToken().value == "void" || Is_type_specifier(PeekToken())) { 
        parser_tree->AddChild(definition());
      } else {
        parser_tree->AddChild(statement());
      }
      memory->LeaveScope();
      return parser_tree;
    } catch (...) {
      memory->LeaveScope();  // Parser 錯誤時，退出假執行memory scope
      delete parser_tree;
      throw;
    }
  }

  void Clear() {
    // 處置殘留PeekToken值
    if (keep_lookahead_after_error) {
      keep_lookahead_after_error = false;
      lookahead.line = 1;
      scanner.SetLine(1);  // 避免P2 bug: a \n a \n ;
    } else if (has_lookahead) {
      lookahead.line = 1;
      scanner.SetLine(1);
    }else {
      lookahead = Token();
      scanner.ResetLine();  // 重新計算input line
    }
    // 清除token_log
    token_log.clear();
  }

  void IgnoreThisLine() {
    if (keep_lookahead_after_error) return;
    has_lookahead = false;
    lookahead = Token();
    scanner.IgnoreThisLine();
  }
};

class PrettyPrinter {
 private:
  Memory * memory;  // 查 function 名稱
  Tools tools;

  void PrintIndent (int n) { for (int i = 0; i < n; i++) cout << " "; }
  bool IsPPMM (string str) { return str == "++" || str == "--"; }
  bool NeedSpace(Token cur, Token pre) {
    if (cur.value == "[" || cur.value == ",") return false;
    if (cur.value == ")" && pre.value == "(") return false;
    // function call: F(...) / Done(...) / ListFunction(...)
    if (cur.value == "(" && (memory->IsDeclaredFunction(pre.value) || tools.IsSystemFunctionName(pre))) return false;
    // i++ / i-- / ++i / --i
    if (IsPPMM(cur.value) && pre.type == TokenType::IDENT) return false;
    if (IsPPMM(pre.value) && cur.type == TokenType::IDENT) return false;
    return true;
  }
  void Print(ParserTreeNode * node, int & indent, bool & is_line_start, Token & prev_token, int & else_if_indent) {
    if (node == nullptr) return;
    if (node->type != NodeType::LeafNode) {  // 遞迴直到LeafNode
      for (auto child : node->children) {
        Print(child, indent, is_line_start, prev_token, else_if_indent);
      }
      return;
    }
    Token cur = node->token;
    // else-if 結束
    if (prev_token.value == "}" && cur.value != "else" && else_if_indent > 0) {
      indent -= 2;
      else_if_indent--;
    }
    if (cur.value == "}") indent -= 2;
    // 處理空白/縮排
    if (is_line_start) {
      PrintIndent(indent);
      is_line_start = false;
    } else if (cur.value == "if" && prev_token.value == "else") {  // else-if 開始
      cout << "\n";
      indent += 2;
      else_if_indent++;
      PrintIndent(indent);
    } else if (NeedSpace(cur, prev_token)) {
      cout << " ";
    }
    cout << cur.value;
    if (cur.value == ";" || cur.value == "{" || cur.value == "}") {
      cout << "\n";
      is_line_start = true;
      if (cur.value == "{") indent += 2;
    }
    prev_token = cur;
  }

 public:
  PrettyPrinter(Memory * memory) { this->memory = memory; }

  void Print(ParserTreeNode * node) {
    int indent = 0;
    bool is_line_start = true;
    Token prev_token;
    int else_if_indent = 0;
    Print(node, indent, is_line_start, prev_token, else_if_indent);
  }
};

class ALU {
 private:
  string DecodeEscapeCharacters(string token_value) {
    string decoded_value;
    for (size_t char_index = 0; char_index < token_value.size(); ++char_index) {
      if (token_value[char_index] == '\\' && char_index + 1 < token_value.size()) {
        char escaped_char = token_value[++char_index];
        if (escaped_char == 'n') decoded_value += '\n';
        else if (escaped_char == 't') decoded_value += '\t';
        else if (escaped_char == '\\') decoded_value += '\\';
        else if (escaped_char == '"') decoded_value += '"';
        else if (escaped_char == '\'') decoded_value += '\'';
        else decoded_value += escaped_char;
      } else {
        decoded_value += token_value[char_index];
      }
    }
    return decoded_value;
  }

 public:
  Token GetDefaultToken(TokenType token_type) {
    if (token_type == TokenType::CONST_INT) return Token(token_type, "0");
    if (token_type == TokenType::CONST_FLOAT) return Token(token_type, "0");
    if (token_type == TokenType::CONST_BOOL) return Token(token_type, "false");
    if (token_type == TokenType::CONST_CHAR) return Token(token_type, string("\0", 1));
    if (token_type == TokenType::CONST_STRING) return Token(token_type, "");
    return Token(TokenType::CONST_VOID, "");
  }

  Token NormalizeToken(Token token) {
    if (token.value == "" && token.type != TokenType::CONST_STRING) return GetDefaultToken(token.type);
    if (token.type == TokenType::CONST_STRING) {
      if (token.value.size() >= 2 && token.value.front() == '"' && token.value.back() == '"') {
        token.value = token.value.substr(1, token.value.size() - 2);
      }
      token.value = DecodeEscapeCharacters(token.value);
    } else if (token.type == TokenType::CONST_CHAR) {
      if (token.value.size() >= 2 && token.value.front() == '\'' && token.value.back() == '\'') {
        token.value = token.value.substr(1, token.value.size() - 2);
      }
      token.value = DecodeEscapeCharacters(token.value);
    }
    if (token.type == TokenType::CONST_BOOL) {
      if (token.value == "1") token.value = "true";
      if (token.value == "0") token.value = "false";
    }
    return token;
  }

  double ConvertToDouble(Token token) {
    token = NormalizeToken(token);
    if (token.type == TokenType::CONST_BOOL) return (token.value == "true") ? 1 : 0;
    if (token.type == TokenType::CONST_CHAR) return token.value.empty() ? 0 : (unsigned char)token.value[0];
    if (token.type == TokenType::CONST_STRING) return atof(token.value.c_str());
    if (token.value == "") return 0;
    return atof(token.value.c_str());
  }

  long long ConvertToInteger(Token token) {
    return (long long)ConvertToDouble(token);
  }

  bool IsTrue(Token token) {
    token = NormalizeToken(token);
    if (token.type == TokenType::CONST_BOOL) return token.value == "true" || token.value == "1";
    if (token.type == TokenType::CONST_STRING || token.type == TokenType::CONST_CHAR) {
      return !token.value.empty() && token.value != string("\0", 1);
    }
    return fabs(ConvertToDouble(token)) > TOLERANCE;
  }

  string ConvertTokenToString(Token token) {
    token = NormalizeToken(token);
    if (token.type == TokenType::CONST_STRING || token.type == TokenType::CONST_CHAR) return token.value;
    if (token.type == TokenType::CONST_BOOL) return IsTrue(token) ? "true" : "false";
    if (token.type == TokenType::CONST_INT) return to_string((long long)ConvertToDouble(token));
    if (token.type == TokenType::CONST_FLOAT) {
      stringstream output_stream;
      output_stream << fixed << setprecision(PRECISION) << ConvertToDouble(token);
      string formatted_number = output_stream.str();
      while (formatted_number.size() > 1 && formatted_number.back() == '0') formatted_number.pop_back();
      if (!formatted_number.empty() && formatted_number.back() == '.') formatted_number += '0';
      return formatted_number;
    }
    return token.value;
  }

  Token MakeNumberToken(double number_value, bool is_float) {
    if (!is_float) return Token(TokenType::CONST_INT, to_string((long long)number_value));
    stringstream output_stream;
    output_stream << fixed << setprecision(10) << number_value;
    string number_string = output_stream.str();
    while (number_string.size() > 1 && number_string.back() == '0') number_string.pop_back();
    if (!number_string.empty() && number_string.back() == '.') number_string += '0';
    return Token(TokenType::CONST_FLOAT, number_string);
  }

  Token MakeBoolToken(bool boolean_value) {
    return Token(TokenType::CONST_BOOL, boolean_value ? "true" : "false");
  }

  Token CastToken(Token token, TokenType target_type) {
    token = NormalizeToken(token);
    if (target_type == TokenType::EMPTY) return token;
    if (target_type == TokenType::CONST_STRING) return Token(target_type, ConvertTokenToString(token));
    if (target_type == TokenType::CONST_CHAR) {
      string string_value = ConvertTokenToString(token);
      return Token(target_type, string_value.empty() ? string("\0", 1) : string(1, string_value[0]));
    }
    if (target_type == TokenType::CONST_BOOL) return MakeBoolToken(IsTrue(token));
    if (target_type == TokenType::CONST_INT) return Token(target_type, to_string((long long)ConvertToDouble(token)));
    if (target_type == TokenType::CONST_FLOAT) return MakeNumberToken(ConvertToDouble(token), true);
    return token;
  }

  Token CalculateBinaryOperation(Token left_value, string op, Token right_value) {
    left_value = NormalizeToken(left_value);
    right_value = NormalizeToken(right_value);
    if (op == "+" && (left_value.type == TokenType::CONST_STRING || right_value.type == TokenType::CONST_STRING ||
        left_value.type == TokenType::CONST_CHAR || right_value.type == TokenType::CONST_CHAR)) {
      return Token(TokenType::CONST_STRING, ConvertTokenToString(left_value) + ConvertTokenToString(right_value));
    }
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
      bool is_float_result = left_value.type == TokenType::CONST_FLOAT || right_value.type == TokenType::CONST_FLOAT;
      double left_number = ConvertToDouble(left_value);
      double right_number = ConvertToDouble(right_value);
      double result_number = 0;
      if (op == "+") result_number = left_number + right_number;
      if (op == "-") result_number = left_number - right_number;
      if (op == "*") result_number = left_number * right_number;
      if (op == "/") {
        if (fabs(right_number) <= TOLERANCE) throw OurException(ExceptionType::OTHER_ERR, "", 0);
        result_number = left_number / right_number;
      }
      if (op == "%") result_number = ConvertToInteger(left_value) % ConvertToInteger(right_value);
      return MakeNumberToken(result_number, is_float_result && op != "%");
    }
    if (op == "<<" || op == ">>") {
      long long left_number = ConvertToInteger(left_value);
      long long right_number = ConvertToInteger(right_value);
      return Token(TokenType::CONST_INT, to_string(op == "<<" ? (left_number << right_number) : (left_number >> right_number)));
    }
    if (op == "&&") return MakeBoolToken(IsTrue(left_value) && IsTrue(right_value));
    if (op == "||") return MakeBoolToken(IsTrue(left_value) || IsTrue(right_value));
    if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
      bool result = false;
      if (left_value.type == TokenType::CONST_STRING || right_value.type == TokenType::CONST_STRING ||
          left_value.type == TokenType::CONST_CHAR || right_value.type == TokenType::CONST_CHAR) {
        string left_string = ConvertTokenToString(left_value);
        string right_string = ConvertTokenToString(right_value);
        if (op == "==") result = left_string == right_string;
        if (op == "!=") result = left_string != right_string;
        if (op == "<") result = left_string < right_string;
        if (op == ">") result = left_string > right_string;
        if (op == "<=") result = left_string <= right_string;
        if (op == ">=") result = left_string >= right_string;
      } else {
        double difference = ConvertToDouble(left_value) - ConvertToDouble(right_value);
        if (op == "==") result = fabs(difference) <= TOLERANCE;
        if (op == "!=") result = fabs(difference) > TOLERANCE;
        if (op == "<") result = difference < -TOLERANCE;
        if (op == ">") result = difference > TOLERANCE;
        if (op == "<=") result = difference <= TOLERANCE;
        if (op == ">=") result = difference >= -TOLERANCE;
      }
      return MakeBoolToken(result);
    }
    return Token(TokenType::CONST_VOID, "");
  }
};

class Interpreter {
 private:
  struct ReturnSignal {
    Token value;
    ReturnSignal(Token v) : value(v) {}
  };

  Parser * parser;  
  Memory * memory;
  PrettyPrinter * pretty_printer;
  vector<ParserTreeNode*> parser_tree_log;  // 記下所有code的parser tree。最後一次刪除。
  Tools tools;  // 相關工具
  ALU alu;

  // type
  bool IsTypeSpecifier(const string & type_name) {
    return type_name == "int" || type_name == "float" || type_name == "char" || type_name == "string" || type_name == "bool";
  }

  bool IsConstant(Token token) {
    return token.type == TokenType::CONST_INT || token.type == TokenType::CONST_FLOAT ||
           token.type == TokenType::CONST_CHAR || token.type == TokenType::CONST_STRING ||
           token.type == TokenType::CONST_BOOL;
  }

  // operator
  bool IsAssignmentOp(const string & op) {
    return op == "=" || op == "+=" || op == "-=" || op == "*=" || op == "/=" || op == "%=";
  }

  // parser tree 攤平
  vector<Token> CollectTokens(ParserTreeNode * node) {
    vector<Token> collected_tokens;
    CollectTokensInto(node, collected_tokens);
    return collected_tokens;
  }

  void CollectTokensInto(ParserTreeNode * node, vector<Token> & collected_tokens) {
    if (!node) return;
    if (node->type == NodeType::LeafNode) {
      collected_tokens.push_back(node->token);
      return;
    }
    for (auto child : node->children) CollectTokensInto(child, collected_tokens);
  }

  // token range
  int FindMatching(const vector<Token> &tokens, int left_index, string left_symbol, string right_symbol) {
    int depth = 0;
    for (int token_index = left_index; token_index < (int)tokens.size(); ++token_index) {
      if (tokens[token_index].value == left_symbol) depth++;
      else if (tokens[token_index].value == right_symbol) {
        depth--;
        if (depth == 0) return token_index;
      }
    }
    return left_index;
  }

  vector<pair<int,int> > SplitArguments(const vector<Token> &tokens, int left_index, int right_index) {
    vector<pair<int,int> > result;
    int segment_start = left_index, parentheses_count = 0, brace_count = 0, bracket_count = 0;
    for (int token_index = left_index; token_index <= right_index; ++token_index) {
      bool should_cut = (token_index == right_index);
      if (!should_cut) {
        string token_value = tokens[token_index].value;
        if (token_value == "(") parentheses_count++;
        else if (token_value == ")") parentheses_count--;
        else if (token_value == "{") brace_count++;
        else if (token_value == "}") brace_count--;
        else if (token_value == "[") bracket_count++;
        else if (token_value == "]") bracket_count--;
        else if (token_value == "," && parentheses_count == 0 && brace_count == 0 && bracket_count == 0) should_cut = true;
      }
      if (should_cut) {
        if (segment_start < token_index) result.push_back({segment_start, token_index});
        segment_start = token_index + 1;
      }
    }
    return result;
  }

  // statement
  int FindStatementEnd(const vector<Token> &tokens, int start, int end) {
    if (start >= end) return start;
    if (tokens[start].value == "{") return FindMatching(tokens, start, "{", "}") + 1;
    if (tokens[start].value == "if") {
      int right_parenthesis = FindMatching(tokens, start + 1, "(", ")");
      int then_statement_end = FindStatementEnd(tokens, right_parenthesis + 1, end);
      if (then_statement_end < end && tokens[then_statement_end].value == "else") return FindStatementEnd(tokens, then_statement_end + 1, end);
      return then_statement_end;
    }
    if (tokens[start].value == "while") {
      int right_parenthesis = FindMatching(tokens, start + 1, "(", ")");
      return FindStatementEnd(tokens, right_parenthesis + 1, end);
    }
    if (tokens[start].value == "do") {
      int body_end = FindStatementEnd(tokens, start + 1, end);
      int right_parenthesis = FindMatching(tokens, body_end + 1, "(", ")");
      return right_parenthesis + 2;
    }
    int parentheses_count = 0, bracket_count = 0;
    for (int token_index = start; token_index < end; ++token_index) {
      if (tokens[token_index].value == "(") parentheses_count++;
      else if (tokens[token_index].value == ")") parentheses_count--;
      else if (tokens[token_index].value == "[") bracket_count++;
      else if (tokens[token_index].value == "]") bracket_count--;
      else if (tokens[token_index].value == ";" && parentheses_count == 0 && bracket_count == 0) return token_index + 1;
    }
    return end;
  }

  // memory
  pair<MemoryData*, int> NormalizeReference(MemoryData * memory_data, int index = -1) {
    while (memory_data && memory_data->is_reference) {
      if (memory_data->reference_index >= 0) index = memory_data->reference_index;
      memory_data = memory_data->reference_target;
    }
    return {memory_data, index};
  }

  void CheckArrayIndex(MemoryData * memory_data, int index, string name = "") {
    pair<MemoryData*, int> normalized_reference = NormalizeReference(memory_data, index);
    memory_data = normalized_reference.first;
    index = normalized_reference.second;
    if (memory_data && memory_data->type == DataType::Array && (index < 0 || index >= memory_data->array_tokens.size())) {
      string error_message = "Runtime error : array '" + name + "' index out of range!\n";
      throw OurException(ExceptionType::RUNTIME_ERR, error_message, 0);
    }
  }

  Token ReadValue(MemoryData * memory_data, int index = -1) {
    auto normalized_reference = NormalizeReference(memory_data, index);
    memory_data = normalized_reference.first;
    index = normalized_reference.second;
    if (!memory_data) return Token(TokenType::CONST_VOID, "");
    if (memory_data->type == DataType::Array) {
      if (index >= 0) {
        CheckArrayIndex(memory_data, index);
        return alu.NormalizeToken(memory_data->array_tokens[index]);
      }
      if (memory_data->array_type == TokenType::CONST_CHAR) {
        string char_array_string;
        for (Token array_token : memory_data->array_tokens) {
          array_token = alu.NormalizeToken(array_token);
          if (array_token.value == string("\0", 1)) break;
          char_array_string += array_token.value;
        }
        return Token(TokenType::CONST_STRING, char_array_string);
      }
      return Token(TokenType::CONST_VOID, "");
    }
    return alu.NormalizeToken(memory_data->token);
  }

  void WriteValue(MemoryData * memory_data, int index, Token value) {
    pair<MemoryData*, int> normalized_reference = NormalizeReference(memory_data, index);
    memory_data = normalized_reference.first;
    index = normalized_reference.second;
    if (!memory_data) return;
    if (memory_data->type == DataType::Array) {
      if (index >= 0) {
        CheckArrayIndex(memory_data, index);
        memory_data->array_tokens[index] = alu.CastToken(value, memory_data->array_type);
      } else if (memory_data->array_type == TokenType::CONST_CHAR) {
        string string_value = alu.ConvertTokenToString(value);
        for (int array_index = 0; array_index < (int)memory_data->array_tokens.size(); ++array_index) {
          string char_string = (array_index < (int)string_value.size()) ? string(1, string_value[array_index]) : string("\0", 1);
          memory_data->array_tokens[array_index] = Token(TokenType::CONST_CHAR, char_string);
        }
      }
    } else {
      memory_data->token = alu.CastToken(value, memory_data->token.type);
    }
  }

  // expression
  Token Evaluate(const vector<Token> &tokens, int left_index, int right_index) {
    int current_pos = left_index;
    return ParseComma(tokens, current_pos, right_index);
  }

  pair<MemoryData*, int> GetAssignLocation(const vector<Token> &tokens, int left_index, int right_index) {
    if (left_index >= right_index) return {nullptr, -1};
    MemoryData *memory_data = memory->GetRaw(tokens[left_index].value);
    int index = -1;
    if (left_index + 1 < right_index && tokens[left_index + 1].value == "[") {
      int right_bracket = FindMatching(tokens, left_index + 1, "[", "]");
      index = alu.ConvertToInteger(Evaluate(tokens, left_index + 2, right_bracket));
    }
    return NormalizeReference(memory_data, index);
  }

  Token ParseComma(const vector<Token> &tokens, int &current_pos, int end) {
    Token result_value = ParseAssign(tokens, current_pos, end);
    while (current_pos < end && tokens[current_pos].value == ",") { 
      current_pos++; 
      result_value = ParseAssign(tokens, current_pos, end); 
    }
    return result_value;
  }

  Token ParseAssign(const vector<Token> &tokens, int &current_pos, int end) {
    int left_start = current_pos;
    Token left = ParseConditional(tokens, current_pos, end);
    int left_end = current_pos;
    if (current_pos < end && IsAssignmentOp(tokens[current_pos].value)) {
      string op = tokens[current_pos++].value;
      Token right = ParseAssign(tokens, current_pos, end);
      pair<MemoryData*, int> assign_target = GetAssignLocation(tokens, left_start, left_end);
      Token old_value = ReadValue(assign_target.first, assign_target.second);
      Token new_value = (op == "=") ? right : alu.CalculateBinaryOperation(old_value, op.substr(0, 1), right);
      WriteValue(assign_target.first, assign_target.second, new_value);
      return ReadValue(assign_target.first, assign_target.second);
    }
    return left;
  }

  int FindConditionalColon(const vector<Token> &tokens, int question_pos, int end) {
    int parentheses_count = 0, bracket_count = 0, nested_conditional_count = 0;
    for (int token_index = question_pos + 1; token_index < end; ++token_index) {
      string token_value = tokens[token_index].value;
      if (token_value == "(") parentheses_count++;
      else if (token_value == ")") parentheses_count--;
      else if (token_value == "[") bracket_count++;
      else if (token_value == "]") bracket_count--;
      else if (parentheses_count == 0 && bracket_count == 0 && token_value == "?") nested_conditional_count++;
      else if (parentheses_count == 0 && bracket_count == 0 && token_value == ":") {
        if (nested_conditional_count == 0) return token_index;
        nested_conditional_count--;
      }
    }
    return -1;
  }

  int FindConditionalEnd(const vector<Token> &tokens, int start, int end) {
    int parentheses_count = 0, bracket_count = 0, nested_conditional_count = 0;
    for (int token_index = start; token_index < end; ++token_index) {
      string token_value = tokens[token_index].value;
      if (token_value == "(") parentheses_count++;
      else if (token_value == ")") parentheses_count--;
      else if (token_value == "[") bracket_count++;
      else if (token_value == "]") bracket_count--;
      else if (parentheses_count == 0 && bracket_count == 0 && token_value == "?") nested_conditional_count++;
      else if (parentheses_count == 0 && bracket_count == 0 && token_value == ":") {
        if (nested_conditional_count > 0) nested_conditional_count--;
      } else if (parentheses_count == 0 && bracket_count == 0 && nested_conditional_count == 0 && token_value == ",") {
        return token_index;
      }
    }
    return end;
  }

  Token ParseConditional(const vector<Token> &tokens, int &current_pos, int end) {
    Token condition_value = ParseLogicalOr(tokens, current_pos, end);
    if (current_pos < end && tokens[current_pos].value == "?") {
      int question_pos = current_pos;
      int colon_pos = FindConditionalColon(tokens, question_pos, end);
      if (colon_pos < 0) return condition_value;
      int conditional_end = FindConditionalEnd(tokens, colon_pos + 1, end);
      Token result;
      if (alu.IsTrue(condition_value)) {
        int branch_pos = question_pos + 1;
        result = Evaluate(tokens, branch_pos, colon_pos);
      } else {
        int branch_pos = colon_pos + 1;
        result = Evaluate(tokens, branch_pos, conditional_end);
      }
      current_pos = conditional_end;
      return result;
    }
    return condition_value;
  }

  Token ParseLogicalOr(const vector<Token> &tokens, int &current_pos, int end) {
    Token result_value = ParseLogicalAnd(tokens, current_pos, end);
    while (current_pos < end && tokens[current_pos].value == "||") { 
      current_pos++; 
      result_value = alu.CalculateBinaryOperation(result_value, "||", ParseLogicalAnd(tokens, current_pos, end)); 
    }
    return result_value;
  }

  Token ParseLogicalAnd(const vector<Token> &tokens, int &current_pos, int end) {
    Token result_value = ParseEquality(tokens, current_pos, end);
    while (current_pos < end && tokens[current_pos].value == "&&") { 
      current_pos++; 
      result_value = alu.CalculateBinaryOperation(result_value, "&&", ParseEquality(tokens, current_pos, end)); 
    }
    return result_value;
  }

  Token ParseEquality(const vector<Token> &tokens, int &current_pos, int end) {
    Token result_value = ParseRelation(tokens, current_pos, end);
    while (current_pos < end && (tokens[current_pos].value == "==" || tokens[current_pos].value == "!=")) { 
      string op = tokens[current_pos++].value; 
      result_value = alu.CalculateBinaryOperation(result_value, op, ParseRelation(tokens, current_pos, end)); 
    }
    return result_value;
  }

  Token ParseRelation(const vector<Token> &tokens, int &current_pos, int end) {
    Token result_value = ParseShift(tokens, current_pos, end);
    while (current_pos < end && (tokens[current_pos].value == "<" || tokens[current_pos].value == ">" || tokens[current_pos].value == "<=" || tokens[current_pos].value == ">=")) {
      string op = tokens[current_pos++].value;
      result_value = alu.CalculateBinaryOperation(result_value, op, ParseShift(tokens, current_pos, end));
    }
    return result_value;
  }

  Token ParseShift(const vector<Token> &tokens, int &current_pos, int end) {
    Token left_value = ParseAdditive(tokens, current_pos, end);
    while (current_pos < end && (tokens[current_pos].value == "<<" || tokens[current_pos].value == ">>")) {
      string op = tokens[current_pos++].value;
      Token right_value = ParseAdditive(tokens, current_pos, end);
      if (left_value.type == TokenType::IDENT && left_value.value == "cout" && op == "<<") {
        cout << alu.ConvertTokenToString(right_value);
        left_value = (current_pos < end && tokens[current_pos].value == "<<") ? Token(TokenType::IDENT, "cout") : right_value;
      } else if (left_value.type == TokenType::IDENT && left_value.value == "cin" && op == ">>") {
        // 本Project 沒實作 cin
        left_value = Token(TokenType::IDENT, "cin");
      } else {
        left_value = alu.CalculateBinaryOperation(left_value, op, right_value);
      }
    }
    return left_value;
  }

  Token ParseAdditive(const vector<Token> &tokens, int &current_pos, int end) {
    Token result_value = ParseMultiplicative(tokens, current_pos, end);
    while (current_pos < end && (tokens[current_pos].value == "+" || tokens[current_pos].value == "-")) { 
      string op = tokens[current_pos++].value; 
      result_value = alu.CalculateBinaryOperation(result_value, op, ParseMultiplicative(tokens, current_pos, end)); 
    }
    return result_value;
  }

  Token ParseMultiplicative(const vector<Token> &tokens, int &current_pos, int end) {
    Token result_value = ParseUnary(tokens, current_pos, end);
    while (current_pos < end && (tokens[current_pos].value == "*" || tokens[current_pos].value == "/" || tokens[current_pos].value == "%")) { 
      string op = tokens[current_pos++].value; 
      result_value = alu.CalculateBinaryOperation(result_value, op, ParseUnary(tokens, current_pos, end)); 
    }
    return result_value;
  }

  Token Calculate_PP_MM(Token old_value, string op) {
    Token one(TokenType::CONST_INT, "1");
    if (op == "++") return alu.CalculateBinaryOperation(old_value, "+", one);
    return alu.CalculateBinaryOperation(old_value, "-", one);
  }

  Token ParseUnary(const vector<Token> &tokens, int &current_pos, int end) {
    if (current_pos < end && (tokens[current_pos].value == "+" || tokens[current_pos].value == "-" || tokens[current_pos].value == "!")) {
      string op = tokens[current_pos++].value;
      Token value_token = ParseUnary(tokens, current_pos, end);
      if (op == "+") return value_token;
      if (op == "-") return alu.MakeNumberToken(-alu.ConvertToDouble(value_token), value_token.type == TokenType::CONST_FLOAT);
      return alu.MakeBoolToken(!alu.IsTrue(value_token));
    }
    if (current_pos < end && (tokens[current_pos].value == "++" || tokens[current_pos].value == "--")) {
      string op = tokens[current_pos++].value;
      int target_start_pos = current_pos;
      ParsePrimary(tokens, current_pos, end);
      pair<MemoryData*, int> target_location = GetAssignLocation(tokens, target_start_pos, current_pos);
      Token old_value = ReadValue(target_location.first, target_location.second);
      Token new_value = Calculate_PP_MM(old_value, op);
      WriteValue(target_location.first, target_location.second, new_value);
      return ReadValue(target_location.first, target_location.second);
    }
    return ParsePrimary(tokens, current_pos, end);
  }

  Token ParsePrimary(const vector<Token> &tokens, int &current_pos, int end) {
    if (current_pos >= end) return Token(TokenType::CONST_VOID, "");
    Token current_token = tokens[current_pos++];
    if (IsConstant(current_token)) return alu.NormalizeToken(current_token);
    if (current_token.value == "(") {
      int right_parenthesis = FindMatching(tokens, current_pos - 1, "(", ")");
      Token value_token = Evaluate(tokens, current_pos, right_parenthesis);
      current_pos = right_parenthesis + 1;
      return value_token;
    }
    if (current_token.type == TokenType::IDENT) {
      if (current_token.value == "cout" || current_token.value == "cin") return Token(TokenType::IDENT, current_token.value);
      if (current_pos < end && tokens[current_pos].value == "(") {
        int right_parenthesis = FindMatching(tokens, current_pos, "(", ")");
        vector<vector<Token> > args;
        for (auto argument_segment : SplitArguments(tokens, current_pos + 1, right_parenthesis)) {
          args.push_back(vector<Token>(tokens.begin() + argument_segment.first, tokens.begin() + argument_segment.second));
        }
        current_pos = right_parenthesis + 1;
        return CallFunction(current_token.value, args);
      }
      int lvalue_start = current_pos - 1;
      if (current_pos < end && tokens[current_pos].value == "[") {
        int right_parenthesis = FindMatching(tokens, current_pos, "[", "]");
        current_pos = right_parenthesis + 1;
      }
      auto lvalue = GetAssignLocation(tokens, lvalue_start, current_pos);
      Token old_value = ReadValue(lvalue.first, lvalue.second);
      if (current_pos < end && (tokens[current_pos].value == "++" || tokens[current_pos].value == "--")) {
        string op = tokens[current_pos++].value;
        Token new_value = alu.CalculateBinaryOperation(old_value, op == "++" ? "+" : "-", Token(TokenType::CONST_INT, "1"));
        WriteValue(lvalue.first, lvalue.second, new_value);
      }
      return old_value;
    }
    return current_token;
  }

  // statement
  void ExecuteDeclaration(const vector<Token> &tokens, int &current_pos, int end, bool should_print_message) {
    TokenType type = tools.String_To_TokenType(tokens[current_pos++].value);
    while (current_pos < end) {
      if (tokens[current_pos].value == ";") { current_pos++; break; }
      if (tokens[current_pos].value == ",") { current_pos++; continue; }
      string name = tokens[current_pos++].value;
      if (current_pos < end && tokens[current_pos].value == "[") {
        int array_size = stoi(tokens[current_pos + 1].value);
        string message = memory->WriteArray(name, type, array_size);
        if (should_print_message) cout << message;
        current_pos += 3;
      } else {
        string message = memory->WriteSingleVariable(name, alu.GetDefaultToken(type));
        if (should_print_message) cout << message;
      }
    }
  }

  void ExecuteBlock(const vector<Token> &tokens, int &current_pos, int end) {
    while (current_pos < end) ExecuteStatement(tokens, current_pos, end);
  }

  void ExecuteStatement(const vector<Token> &tokens, int &current_pos, int end) {
    if (current_pos >= end) return;
    if (tokens[current_pos].value == ";") { 
      current_pos++; 
      return; 
    }
    if (IsTypeSpecifier(tokens[current_pos].value)) { 
      ExecuteDeclaration(tokens, current_pos, end, false); 
      return; 
    }
    if (tokens[current_pos].value == "{") {
      int right_brace = FindMatching(tokens, current_pos, "{", "}");
      memory->EnterScope();
      try {
        int statement_pos = current_pos + 1;
        ExecuteBlock(tokens, statement_pos, right_brace);
      } catch (...) {
        memory->LeaveScope();
        throw;
      }
      memory->LeaveScope();
      current_pos = right_brace + 1;
      return;
    }
    if (tokens[current_pos].value == "if") {
      int left_parenthesis = current_pos + 1;
      int right_parenthesis = FindMatching(tokens, left_parenthesis, "(", ")");
      bool condition_is_true = alu.IsTrue(Evaluate(tokens, left_parenthesis + 1, right_parenthesis));
      int then_start = right_parenthesis + 1;
      int then_end = FindStatementEnd(tokens, then_start, end);
      int else_start = -1;
      int else_end = -1;
      if (then_end < end && tokens[then_end].value == "else") {
        else_start = then_end + 1;
        else_end = FindStatementEnd(tokens, else_start, end);
      }
      if (condition_is_true) { 
        int statement_pos = then_start; 
        ExecuteStatement(tokens, statement_pos, then_end); 
      } else if (else_start != -1) { 
        int statement_pos = else_start; 
        ExecuteStatement(tokens, statement_pos, else_end); 
      }
      current_pos = (else_end != -1) ? else_end : then_end;
      return;
    }
    if (tokens[current_pos].value == "while") {
      int left_parenthesis = current_pos + 1;
      int right_parenthesis = FindMatching(tokens, left_parenthesis, "(", ")");
      int body_start = right_parenthesis + 1;
      int body_end = FindStatementEnd(tokens, body_start, end);
      while (alu.IsTrue(Evaluate(tokens, left_parenthesis + 1, right_parenthesis))) {
        int statement_pos = body_start;
        ExecuteStatement(tokens, statement_pos, body_end);
      }
      current_pos = body_end;
      return;
    }
    if (tokens[current_pos].value == "do") {
      int body_start = current_pos + 1;
      int body_end = FindStatementEnd(tokens, body_start, end);
      int left_parenthesis = body_end + 1;
      int right_parenthesis = FindMatching(tokens, left_parenthesis, "(", ")");
      do { 
        int statement_pos = body_start; 
        ExecuteStatement(tokens, statement_pos, body_end); 
      } while (alu.IsTrue(Evaluate(tokens, left_parenthesis + 1, right_parenthesis)));
      current_pos = right_parenthesis + 2;
      return;
    }
    if (tokens[current_pos].value == "return") {
      int semicolon_pos = current_pos;
      while (semicolon_pos < end && tokens[semicolon_pos].value != ";") semicolon_pos++;
      Token return_value = (current_pos + 1 < semicolon_pos) ? Evaluate(tokens, current_pos + 1, semicolon_pos) : Token(TokenType::CONST_VOID, "");
      current_pos = semicolon_pos + 1;
      throw ReturnSignal(return_value);
    }
    int semicolon_pos = current_pos;
    int parentheses_count = 0, bracket_count = 0;
    while (semicolon_pos < end) {
      if (tokens[semicolon_pos].value == "(") parentheses_count++;
      else if (tokens[semicolon_pos].value == ")") parentheses_count--;
      else if (tokens[semicolon_pos].value == "[") bracket_count++;
      else if (tokens[semicolon_pos].value == "]") bracket_count--;
      else if (tokens[semicolon_pos].value == ";" && parentheses_count == 0 && bracket_count == 0) break;
      semicolon_pos++;
    }
    Evaluate(tokens, current_pos, semicolon_pos);
    current_pos = semicolon_pos + 1;
  }

  Token CallFunction(string function_name, vector<vector<Token> > actual_arguments) {
    if (function_name == "Done") Done();
    if (function_name == "ListAllVariables") { ListAllVariables(); return Token(TokenType::CONST_VOID, ""); }
    if (function_name == "ListAllFunctions") { ListAllFunctions(); return Token(TokenType::CONST_VOID, ""); }
    if (function_name == "ListVariable") {
      if (!actual_arguments.empty()) ListVariable(alu.ConvertTokenToString(Evaluate(actual_arguments[0], 0, actual_arguments[0].size())));
      return Token(TokenType::CONST_VOID, "");
    }
    if (function_name == "ListFunction") {
      if (!actual_arguments.empty()) ListFunction(alu.ConvertTokenToString(Evaluate(actual_arguments[0], 0, actual_arguments[0].size())));
      return Token(TokenType::CONST_VOID, "");
    }
    ParserTreeNode *function_node = memory->GetFunction(function_name);
    vector<Token> function_tokens = CollectTokens(function_node);
    int left_parenthesis = 2, right_parenthesis = FindMatching(function_tokens, left_parenthesis, "(", ")");
    int left_brace = right_parenthesis + 1;
    while (left_brace < (int)function_tokens.size() && function_tokens[left_brace].value != "{") left_brace++;
    int right_brace = FindMatching(function_tokens, left_brace, "{", "}");
    vector<pair<int,int> > formal_parameters = SplitArguments(function_tokens, left_parenthesis + 1, right_parenthesis);
    // args
    vector<Token> copied_arguments(actual_arguments.size());
    vector<pair<MemoryData*,int> > reference_arguments(actual_arguments.size(), {nullptr, -1});
    for (int argument_index = 0; argument_index < (int)actual_arguments.size(); ++argument_index) {
      copied_arguments[argument_index] = Evaluate(actual_arguments[argument_index], 0, actual_arguments[argument_index].size());
      reference_arguments[argument_index] = GetAssignLocation(actual_arguments[argument_index], 0, actual_arguments[argument_index].size());
    }
    memory->EnterFunctionScope();
    try {
      for (int argument_index = 0; argument_index < (int)formal_parameters.size(); ++argument_index) {
        int formal_start = formal_parameters[argument_index].first;
        int formal_end = formal_parameters[argument_index].second;
        if (formal_start >= formal_end || function_tokens[formal_start].value == "void") continue;
        TokenType type = tools.String_To_TokenType(function_tokens[formal_start++].value);
        bool is_reference_parameter = false, is_array_parameter = false;
        if (formal_start < formal_end && function_tokens[formal_start].value == "&") { 
          is_reference_parameter = true; 
          formal_start++; 
        }
        string formal_name = function_tokens[formal_start++].value;
        if (formal_start < formal_end && function_tokens[formal_start].value == "[") is_array_parameter = true;
        if (argument_index >= (int)actual_arguments.size()) {
          memory->WriteSingleVariable(formal_name, alu.GetDefaultToken(type));
        } else if (is_reference_parameter || is_array_parameter) {
          memory->WriteReference(formal_name, reference_arguments[argument_index].first, reference_arguments[argument_index].second);
        } else {
          memory->WriteSingleVariable(formal_name, alu.CastToken(copied_arguments[argument_index], type));
        }
      }
      int body_pos = left_brace + 1;
      ExecuteBlock(function_tokens, body_pos, right_brace);
    } catch (ReturnSignal return_signal) {
      memory->LeaveFunctionScope();
      return return_signal.value;
    } catch (...) {
      memory->LeaveFunctionScope();
      throw;
    }
    memory->LeaveFunctionScope();
    return Token(TokenType::CONST_VOID, "");
  }

  void DefineFunction(vector<Token> &tokens, ParserTreeNode *parser_tree) {
    string function_name = tokens[1].value;
    cout << memory->WriteFunction(function_name, parser_tree);
  }

  void DefineVariables(vector<Token> &tokens) {
    int current_pos = 0;
    ExecuteDeclaration(tokens, current_pos, tokens.size(), true);
  }

  void ExecuteUserInput(ParserTreeNode *parser_tree) {
    vector<Token> tokens = CollectTokens(parser_tree);
    if (tokens.empty()) return;
    if ((tokens[0].value == "void" || IsTypeSpecifier(tokens[0].value)) && tokens.size() > 2 && tokens[2].value == "(") {
      DefineFunction(tokens, parser_tree);
      return;
    }
    if (IsTypeSpecifier(tokens[0].value)) {
      DefineVariables(tokens);
      return;
    }
    int current_pos = 0;
    ExecuteStatement(tokens, current_pos, tokens.size());
    cout << "Statement executed ...\n";
  }

  // ==================================================================================
  // ============================== OurC系統內建functions ==============================
  void ListAllVariables() {
    vector<string> names = memory->GetAllVariablesName();
    for (string name : names) {
      cout << name << "\n";
    }
  }

  void ListAllFunctions() {
    vector<string> names = memory->GetAllFunctionsName();
    for (string name : names) {
      cout << name << "\n";
    }
  }

  void ListVariable(string name) {
    if (memory->IsDeclaredSingleVariable(name)) {
      Token token = memory->GetSingleVariable(name);
      cout << tools.TokenType_To_String(token.type) << " " << name << " ;\n";
    } else if (memory->IsDeclaredArray(name)) {
      cout << tools.TokenType_To_String(memory->GetArrayType(name)) << " " << name << "[ " << memory->GetArraySize(name) << " ] ;\n";
    } else {
      throw OurException(ExceptionType::EMPTY, "Interpreter::ListVariable()", 0);  // 似乎沒有這種測資
    }
  }

  void ListFunction(string name) {
    ParserTreeNode * function = memory->GetFunction(name);
    pretty_printer->Print(function);
  }

  void Done() {
    throw OurException(ExceptionType::DONE, "", 0);
  }
  // ============================== OurC系統內建functions ==============================
  // ==================================================================================

 public:
  Interpreter() {
    memory = new Memory();
    parser = new Parser(memory);
    pretty_printer = new PrettyPrinter(memory);
  }

  bool Run() {
    cout << "> ";
    ParserTreeNode * one_command = nullptr;
    try {
      one_command = parser->user_input();
      ExecuteUserInput(one_command);
      parser_tree_log.push_back(one_command);
    } catch (OurException e) {
      delete one_command;
      if (e.error_type == ExceptionType::DONE || e.error_type == ExceptionType::END_OF_FILE) return false;
      if (e.error_type == ExceptionType::RUNTIME_ERR) {
        cout << e.error_msg;
        cout << "Statement executed ...\n";
      } else {
        cout << e.error_msg;
        parser->IgnoreThisLine();
      }
    }
    return true;
  }
  
  ~Interpreter() {
    for (ParserTreeNode * parser_tree : parser_tree_log) {
      delete parser_tree;
    }
    delete memory;  // memory不會刪除parser tree
    delete parser;
    delete pretty_printer;
  }
};


int main() {
  cin >> uTestNum;  // Project 1 要求：讀取測試編號
  cout << "Our-C running ...\n";
  Interpreter OurC_interpreter;
  while (OurC_interpreter.Run()) {}
  cout << "Our-C exited ...";
}


//  放一包乖乖
//////////////////////////////////////////////////////////////////////////////////////
//                                `-+syhddmmmddhyo+:`                              
//                             .+hmmdddddddddddddddmmds/`      ``...`              
//                          `/hmddddddddddddddddddddddddmy++osyhyyyhhs.            
//                        `ommddddddddddddddddddddddddddddmmdys+++syhhh:           
//            .`         /mmddddddddddddddddddddddddddddddddmmhyyyyyyyyh/          
//        `:sdNy`      .ymddddddddddddddddddddddddddddddddddddmdhhhyyyyyh-         
//    `.+hmmmddmo     -mmddddddddddddddddddddddddddddddddddddddmmhhhhhhhh+         
//   odmmdddddddms` `+mmddddddddddddddddddddddddddddddddddddddddmmddhhhhh+         
//   ymdmmmmmmmmdmmdmmdddddddddddddmmddddddddddddddddddddddddddddmd:ydhhd:         
//   :Nmmmmmmmmmmmmmmddy+::+ydddms/:::/+osydmdddddddddddddddddddddN-`:+o:          
//    ymmmmmmmmmmmmmmdo.`.``/hmm/+hdd/``````-+ydmdddddddddddddddddmy               
//    .mmmmmmmmmmmmmmh:`-o+`:hm/`-o:.```````os/./ymddddddddddddddddN`              
//     /Nmmmmmmmmmmmmh:..::-od/``dMs````````/hNm:`-ymddddddddddddddN-              
//      sNmmmmmmmmmmmd+-:/-.`..`.hh:`...``:hh..:```:NdmmmmmmmdmddddN/              
//   `::/dmmmmmmmmmmmmdo:```./d/````-..:`-mdd.````.dmdmmmmmmmmmmmdmmy              
// ./::-..+dmmmmmmmmmmh/````-dNms-```..``.+/`````-dmds/:-:ohmmmmmmmmN.             
// -/.``--`/dmmmdysydmy.````omNd+.-::-...-:os:..`-hs-``.``./hmmmmmmmmd.       .os` 
// /:---.`.`-/sy:```omy-````hddo` `s.``/NNNNo...`````-+o/``/dmmmmmmmmmms:..:+ymmN: 
// :/.``.``````-``./dNh/````syyyhshd-``:mNmy.````````.```./hmmmmmmmmmmmmmmmmmmmmmd 
//  .::-```...````+dNNNs-```//::/+ooosyhdds.``````-----:+ydmmmmmmmmmmmmmmmmmmmmmmN:
//    `:/````..``-shhdmms-``./::/:::::::+/``````.+dmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmd
//      ./:-```.-oyyhyhhhy+.`.:/::::///:.````..:ymNNmmo:/ymmmmmmmmmmmmmmmmmmmmmmmmN
//        `:y:./yyyyyyyyhh///:..--:--.````..-/ymNNNNNy.``-hNNNNNmmmmmmmmmmmmmmNmh+-
//        `ymdyhyyyyyyhhy/---o+///:::::://oyhmNNNNNNNo```:osyho/---:ymmNNNNmho:`   
//        `+hhhhhyyyyhhs-----y--o/------:+hhhhhhhddds.````````..-..-+dNds+-`       
//          `.:+oossyyyy:---:ssoy:------+hyyyyhhyyys-``....```..--..//`            
//                     syysyysssho:-----ohyyyyhyhhyy/`````..``````-o.              
//                    `hssssssoyhhyo+++syyhhhhyyhyyh+:::-..-:::::::.               
//                    :hysssyo/yhhssyysssssyhyhhhyhdmmy....`                       
//                    shyyyyyyhhhhyyyyyyyyyyhh/-+syhdo`                            
//                   `hhhhhhhhhhhhhhhhhhhhhhhhy`                                   
//                    -:yhyyyysyhhyyyyyyyyyyyyh-                                   
//                      -hysssssyydsyyssssssssyh.                                  
//                       /hsysyyyyd-.+yyyssyyssyh-                                 
//           -/+oo++/-`   +hhyhhhso`  .ohyyyyyyhho:`   `-:/++++/:.                 
//        -+ooooooooooso+/ssss.yy:      `//+ds/oysss+ossoooooooo+os/.              
//     `:o+:/oooooooooooooooossyh:          oyyssooooooooooooooo+/:os+`            
//    -ssoooooooooooooooooosssssy+          .hyssssssooooooooooooooooss-           
//    /syysssssssssssssssyyyyyyyh-           shyyyysyyysssssssssssssssyy           
//      `-/+ossyyyysso+/:-./++//.             .---` `.-:/+oossssoo++/:-`    
//////////////////////////////////////////////////////////////////////////////////////