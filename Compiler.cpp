/*
program        = main_decl ;

main_decl      = "module" "launch" "(" ")" block ;

statement      = var_decl | assign_stmt | transmit_stmt | receive_stmt
               | scan_stmt | drift_stmt | boost_stmt | land_stmt | block ;

block          = "{" { statement } "}" ;

var_decl       = type IDENTIFIER [ "=" expression ] ";" ;

type           = "num" | "decimal" | "text" | "flag" ;

assign_stmt    = IDENTIFIER "=" expression ";" ;

transmit_stmt = "transmit" expression { "," expression } ";"

receive_stmt   = "receive" IDENTIFIER ";" ;

land_stmt      = "land" expression ";" ;

scan_stmt      = "scan" "(" expression ")" block
                 [ "orbit" block ] ;

drift_stmt     = "drift" "(" expression ")" block ;

boost_stmt     = "boost" "(" type IDENTIFIER "=" expression ";"
                             expression ";"
                             IDENTIFIER ( "++" | "--" ) ")"
                 block ;

expression     = comparison { ( "+" | "-" ) comparison } ;

comparison     = term { ( ">" | "<" | ">=" | "<=" | "==" | "!=" ) term } ;

term           = factor { ( "*" | "/" ) factor } ;

factor         = NUMBER | STRING | BOOLEAN
               | IDENTIFIER
               | "(" expression ")" ;
               
*/
#include <iostream>
#include <fstream>
#include <regex>
#include <vector>
#include <unordered_map>
#include <string>
#include <stack>
using namespace std;
//Scanner 
class Token
{
public:
    string category;
    string value;
    int line;
};

unordered_map<string, string> KEYWORD_CATEGORIES = {
    {"module", "FUNCTION_DEF"},
    {"launch", "PROGRAM_START"},
    {"num", "DATA_TYPE_NUM"},
    {"decimal", "Floating value"},
    {"text", "String value"},
    {"flag", "Boolean value"},
    {"transmit", "OUTPUT_STATEMENT"},
    {"receive", "INPUT_STATEMENT"},
    {"scan", "IF_STATEMENT"},
    {"orbit", "ELSE_STATEMENT"},
    {"drift", "WHILE_LOOP"},
    {"boost", "FOR_LOOP"},
    {"land", "RETURN_STATEMENT"},
    {"import", "IMPORT_STATEMENT"},
    {"galaxy", "NAMESPACE"},
    {"true", "Boolean value"},
    {"false", "Boolean value"} };
unordered_map<string, string> OPERATORS_CATEGORIES = {
    {"+", "PLUS"},
    {"-", "MINUS"},
    {"*", "MULTIPLY"},
    {"/", "DIVIDE"},
    {"=", "ASSIGN"},
    {"==", "EQUAL"},
    {"!=", "NOT_EQUAL"},
    {"<", "LESS"},
    {">", "GREATER"},
    {"<=", "LESS_EQUAL"},
    {">=", "GREATER_EQUAL"} };
unordered_map<string, string> SYMBOLS_CATEGORIES = {
    {"{", "LBRACE"},
    {"}", "RBRACE"},
    {"(", "LPAREN"},
    {")", "RPAREN"},
    {";", "SEMICOLON"},
    {",", "COMMA"} };
vector<Token> tokenize(string filename)
{
    regex token_regex(
        R"((//.*)|(\"[^\"\n]*\")|(\d+(\.\d+)?)|([A-Za-z_]\w*)|(==|!=|<=|>=|[+\-*/=<>])|([{}();,])|(\n)|([ \t\r]+)|(.))");
    ifstream file(filename);
    if (!file)
    {
        cout << "Cannot open file" << endl;
        exit(1);
    }
    string code((istreambuf_iterator<char>(file)),
        istreambuf_iterator<char>());
    vector<Token> tokens;
    int line_num = 1;
    for (auto it = sregex_iterator(code.begin(), code.end(), token_regex);
        it != sregex_iterator(); ++it)
    {
        smatch match = *it;
        string value = match.str();
        string category;
        if (match[1].matched)
            category = "COMMENT";
        else if (match[2].matched)
            category = "STRING";
        else if (match[3].matched)
            category = "NUMBER";
        else if (match[5].matched)
            category = "IDENTIFIER";
        else if (match[6].matched)
            category = "OPERATOR";
        else if (match[7].matched)
            category = "SYMBOL";
        else if (match[8].matched)
        {
            line_num++;
            continue;
        }
        else if (match[9].matched)
            continue;
        else
        {
            cout << "Unknown token " << value << " at line " << line_num << endl;
            continue;
        }
        if (category == "IDENTIFIER" && KEYWORD_CATEGORIES.count(value))
            category = KEYWORD_CATEGORIES[value];
        if (category == "OPERATOR")
            category = OPERATORS_CATEGORIES[value];
        if (category == "SYMBOL")
            category = SYMBOLS_CATEGORIES[value];
        if (category == "COMMENT")
            continue;
        tokens.push_back({ category, value, line_num });
    }
    return tokens;
}
void printTokens(const vector<Token>& tokens) {
    cout << "\n=========== TOKENS ===========\n\n";
    cout << "Type\t\tValue\t\tLine\n";
    cout << "--------------------------------------\n";

    for (auto& t : tokens) {
        cout << t.category << "\t\t" << t.value << "\t\t" << t.line << "\n";
    }

    cout << "\n==============================\n";
}
//PARSER PHASE 
struct ASTNode
{
    string type;
    string value;
    int line;
    vector<ASTNode*> children;
};
ASTNode* makeNode(string type, string value = "", int line = 0)
{
    ASTNode* node = new ASTNode();
    node->type = type;
    node->value = value;
    node->line = line;
    return node;
}
void printTree(ASTNode* node, string prefix = "", bool isLast = true)
{
    if (!node)
        return;
    cout << prefix;
    cout << (isLast ? "|-- " : "|-- ");
    cout << node->type;
    if (!node->value.empty())
        cout << " : " << node->value;
    if (node->line > 0 && node->children.empty())
        cout << " (line " << node->line << ")";
    cout << endl;
    string newPrefix = prefix + (isLast ? "    " : "|   ");
    for (size_t i = 0; i < node->children.size(); i++)
    {
        printTree(node->children[i],
            newPrefix,
            i == node->children.size() - 1);
    }
}
void freeTree(ASTNode* node)
{
    if (!node)
        return;
    for (auto child : node->children)
        freeTree(child);
    delete node;
}
//  RECURSIVE-DESCENT PARSER
class Parser
{
    vector<Token> tokens;
    int pos = 0;
    Token current()
    {
        if (pos < (int)tokens.size())
            return tokens[pos];
        return { "EOF", "", -1 };
    }
    Token advance()
    {
        return tokens[pos++];
    }
    Token expect(string type)
    {
        Token t = current();
        if (t.category != type)
        {
            cout << "Parser error at line " << t.line
                << ": expected " << type
                << " but got '" << t.value << "'" << endl;
            exit(1);
        }
        return advance();
    }
    Token expectValue(string val)
    {
        Token t = current();
        if (t.value != val)
        {
            cout << "Error at line " << t.line
                << ": expected '" << val
                << "' but got '" << t.value << "'" << endl;
            exit(1);
        }
        return advance();
    }
    bool check(string type)
    {
        return current().category == type;
    }
    bool isType()
    {
        return check("DATA_TYPE_NUM") ||
            check("Floating value") ||
            check("String value") ||
            check("Boolean value");
    }
    ASTNode* parseProgram()
    {
        ASTNode* node = makeNode("Program");
        node->children.push_back(parseMain());
        return node;
    }
    ASTNode* parseMain()
    {
        ASTNode* node = makeNode("Main");
        Token t = expect("FUNCTION_DEF");
        node->children.push_back(makeNode("Keyword", t.value, t.line));
        t = expect("PROGRAM_START");
        node->children.push_back(makeNode("Keyword", t.value, t.line));
        expect("LPAREN");
        expect("RPAREN");
        node->children.push_back(parseBlock());
        return node;
    }
    ASTNode* parseBlock()
    {
        ASTNode* node = makeNode("Block");
        expect("LBRACE");

        while (!check("RBRACE"))
        {
            if (check("EOF"))
            {
                cout << "Parser error: missing closing '}'" << endl;
                exit(1);
            }
            node->children.push_back(parseStatement());
        }
        expect("RBRACE");
        return node;
    }
    ASTNode* parseStatement()
    {
        if (isType())
            return parseVar();
        if (check("IDENTIFIER"))
            return parseAssign();
        if (check("OUTPUT_STATEMENT"))
            return parseTransmit();
        if (check("INPUT_STATEMENT"))
            return parseReceive();
        if (check("IF_STATEMENT"))
            return parseScan();
        if (check("WHILE_LOOP"))
            return parseDrift();
        if (check("FOR_LOOP"))
            return parseBoost();
        if (check("RETURN_STATEMENT"))
            return parseLand();
        if (check("LBRACE"))
            return parseBlock();
        cout << "Parser Error at line " << current().line << endl;
        exit(1);
    }
    ASTNode* parseVar()
    {
        ASTNode* node = makeNode("VarDecl");
        Token t = advance();
        node->children.push_back(makeNode("Type", t.value, t.line));
        t = expect("IDENTIFIER");
        node->children.push_back(makeNode("Id", t.value, t.line));
        if (check("ASSIGN"))
        {
            advance();
            node->children.push_back(parseExpr());
        }
        expect("SEMICOLON");
        return node;
    }
    ASTNode* parseAssign()
    {
        ASTNode* node = makeNode("Assign");
        Token t = expect("IDENTIFIER");
        node->children.push_back(makeNode("Id", t.value, t.line));
        expect("ASSIGN");
        node->children.push_back(parseExpr());
        expect("SEMICOLON");
        return node;
    }
    ASTNode* parseTransmit()
    {
        ASTNode* node = makeNode("Transmit");
        advance();

        node->children.push_back(parseExpr());

        while (check("COMMA")) {
            advance();
            node->children.push_back(parseExpr());
        }

        expect("SEMICOLON");
        return node;
    }
    ASTNode* parseReceive()
    {
        ASTNode* node = makeNode("Receive");
        advance();
        Token t = expect("IDENTIFIER");
        node->children.push_back(makeNode("Id", t.value, t.line));
        expect("SEMICOLON");
        return node;
    }
    ASTNode* parseLand()
    {
        ASTNode* node = makeNode("Land");
        advance();
        node->children.push_back(parseExpr());
        expect("SEMICOLON");
        return node;
    }
    ASTNode* parseScan()
    {
        ASTNode* node = makeNode("Scan");
        advance();
        expect("LPAREN");
        node->children.push_back(parseExpr());
        expect("RPAREN");
        node->children.push_back(parseBlock());
        if (check("ELSE_STATEMENT"))
        {
            advance();
            node->children.push_back(parseBlock());
        }
        return node;
    }
    ASTNode* parseDrift()
    {
        ASTNode* node = makeNode("Drift");
        advance();
        expect("LPAREN");
        node->children.push_back(parseExpr());
        expect("RPAREN");
        node->children.push_back(parseBlock());
        return node;
    }
    ASTNode* parseBoost()
    {
        ASTNode* node = makeNode("Boost");
        advance();
        expect("LPAREN");
        ASTNode* init = makeNode("BoostInit");
        Token t = advance();
        init->children.push_back(makeNode("Type", t.value, t.line));
        t = expect("IDENTIFIER");
        init->children.push_back(makeNode("Id", t.value, t.line));
        expect("ASSIGN");
        init->children.push_back(parseExpr());
        expect("SEMICOLON");
        node->children.push_back(init);
        ASTNode* cond = makeNode("BoostCondition");
        cond->children.push_back(parseExpr());
        expect("SEMICOLON");
        node->children.push_back(cond);
        ASTNode* update = makeNode("BoostUpdate");
        t = expect("IDENTIFIER");
        update->children.push_back(makeNode("Id", t.value, t.line));
        string op = current().value;
        if (op == "+")
        {
            advance();
            expectValue("+");
            update->children.push_back(makeNode("Op", "++", t.line));
        }
        else if (op == "-")
        {
            advance();
            expectValue("-");
            update->children.push_back(makeNode("Op", "--", t.line));
        }
        else
        {
            cout << "Parser error at line " << current().line
                << ": expected ++ or --" << endl;
            exit(1);
        }
        node->children.push_back(update);
        expect("RPAREN");
        node->children.push_back(parseBlock());
        return node;
    }
    ASTNode* parseExpr()
    {
        ASTNode* left = parseComparison();
        while (check("PLUS") || check("MINUS"))
        {
            Token op = advance();
            ASTNode* right = parseComparison();
            ASTNode* bin = makeNode("Op", op.value, op.line);
            bin->children.push_back(left);
            bin->children.push_back(right);
            left = bin;
        }
        return left;
    }
    ASTNode* parseComparison()
    {
        ASTNode* left = parseTerm();
        while (check("GREATER") || check("LESS") ||
            check("GREATER_EQUAL") || check("LESS_EQUAL") ||
            check("EQUAL") || check("NOT_EQUAL"))
        {
            Token op = advance();
            ASTNode* right = parseTerm();
            ASTNode* bin = makeNode("Op", op.value, op.line);
            bin->children.push_back(left);
            bin->children.push_back(right);
            left = bin;
        }
        return left;
    }
    ASTNode* parseTerm()
    {
        ASTNode* left = parseFactor();
        while (check("MULTIPLY") || check("DIVIDE"))
        {
            Token op = advance();
            ASTNode* right = parseFactor();
            ASTNode* bin = makeNode("Op", op.value, op.line);
            bin->children.push_back(left);
            bin->children.push_back(right);
            left = bin;
        }
        return left;
    }
    ASTNode* parseFactor()
    {
        Token t = current();
        if (t.category == "NUMBER")
        {
            advance();
            return makeNode("Number", t.value, t.line);
        }
        if (t.category == "STRING")
        {
            advance();
            return makeNode("String", t.value, t.line);
        }
        if (t.category == "Boolean value")
        {
            advance();
            return makeNode("Boolean", t.value, t.line);
        }
        if (t.category == "IDENTIFIER")
        {
            advance();
            return makeNode("Id", t.value, t.line);
        }
        if (check("LPAREN"))
        {
            advance();
            ASTNode* e = parseExpr();
            expect("RPAREN");
            return e;
        }
        cout << "Parser error at line " << t.line << endl;
        exit(1);
    }

public:
    Parser(vector<Token> t) : tokens(t) {}
    ASTNode* parse()
    {
        return parseProgram();
    }
};
//  SEMANTIC ANALYZER PHASE 
//1- Symbol table 
struct Symbol
{
    string name, type;
    int line, scope;
};

class SymbolTable
{
private:
    unordered_map<string, vector<Symbol>> table;
    vector<vector<string>> scopeStack;

    vector<Symbol> allSymbols;

public:
    SymbolTable() { scopeStack.push_back({}); }

    void pushScope() { scopeStack.push_back({}); }

    void popScope()
    {
        if (scopeStack.size() <= 1)
            return;
        for (const string& name : scopeStack.back())
        {
            table[name].pop_back();
            if (table[name].empty())
                table.erase(name);
        }
        scopeStack.pop_back();
    }

    Symbol* declare(const string& name, const string& type, int line)
    {
        int currentScope = scopeStack.size() - 1;
        for (const string& n : scopeStack.back())
        {
            if (n == name)
            {
                cout << "Semantic Error: Redeclaration of '" << name << "' at line " << line << "\n";
                return nullptr;
            }
        }
        table[name].push_back({ name, type, line, currentScope });
        scopeStack.back().push_back(name);

        allSymbols.push_back({ name, type, line, currentScope });
        return &table[name].back();
    }

    Symbol* lookup(const string& name)
    {
        auto it = table.find(name);
        if (it != table.end() && !it->second.empty())
            return &it->second.back();
        return nullptr;
    }

    void printTable()
    {
        cout << "\n=========== SYMBOL TABLE ===========\n\n";
        cout << "Name\tType\tScope\tLine\n";
        cout << "------------------------------------\n";
        for (auto& sym : allSymbols)
        {
            cout << sym.name << "\t" << sym.type << "\t" << sym.scope << "\t" << sym.line << "\n";
        }
        cout << "\n====================================\n";
    }
};
// 2- Semantic process
class Semantic
{
private:
    SymbolTable table;
    int depth = 0;
    bool inBoost = false;

public:
    string getType(ASTNode* node)
    {
        if (node->type == "Number")
            return node->value.find('.') != string::npos ? "decimal" : "num";
        if (node->type == "String")
            return "text";
        if (node->type == "Boolean")
            return "flag";
        if (node->type == "Id")
        {
            Symbol* s = table.lookup(node->value);
            if (!s)
            {
                cout << "Semantic Error: Undeclared '" << node->value << "' at line " << node->line << endl;
                exit(1);
            }
            return s->type;
        }
        if (node->type == "Op")
        {
            string left = getType(node->children[0]), right = getType(node->children[1]), op = node->value;
            if (op == ">" || op == "<" || op == ">=" || op == "<=" || op == "==" || op == "!=")
            {
                if (left == right)
                    return "flag";
                cout << "Semantic Error: invalid comparison between " << left << " and " << right << endl;
                exit(1);
            }
            if (left == "num" && right == "num")
                return "num";
            if (left == "decimal" || right == "decimal")
                return "decimal";
            if (left == "text" && right == "text" && op == "+")
                return "text";
            cout << "Semantic Error: invalid operation between " << left << " and " << right << endl;
            exit(1);
        }
        return "";
    }

    void analyze(ASTNode* node)
    {
        if (!node)
            return;
        if (node->type == "Boost")
            inBoost = true;
        if (node->type == "Block")
        {
            if (depth > 0 && !inBoost)
                table.pushScope();
            if (inBoost)
                inBoost = false;
            depth++;
        }

        if (node->type == "BoostInit")
        {
            table.pushScope();
            depth++;
            string type = node->children[0]->value, name = node->children[1]->value;
            int line = node->children[1]->line;
            table.declare(name, type, line);
            string et = getType(node->children[2]);
            if (et != type)
            {
                cout << "Semantic Error at line " << line << ": cannot assign " << et << " to " << type << endl;
                exit(1);
            }
            analyze(node->children[2]);
            return;
        }

        if (node->type == "VarDecl")
        {
            string type = node->children[0]->value, name = node->children[1]->value;
            int line = node->children[1]->line;
            table.declare(name, type, line);
            if (node->children.size() > 2)
            {
                string et = getType(node->children[2]);
                if (et != type)
                {
                    cout << "Semantic Error at line " << line << ": cannot assign " << et << " to " << type << endl;
                    exit(1);
                }
                analyze(node->children[2]);
            }
            return;
        }

        if (node->type == "Assign")
        {
            string name = node->children[0]->value;
            Symbol* s = table.lookup(name);
            if (!s)
            {
                cout << "Semantic Error: '" << name << "' not declared at line " << node->children[0]->line << endl;
                exit(1);
            }
            string et = getType(node->children[1]);
            bool ok = (et == s->type) || (s->type == "decimal" && et == "num");
            if (!ok)
            {
                cout << "Semantic Error: cannot assign " << et << " to " << s->type << " at line " << node->children[0]->line << endl;
                exit(1);
            }
            analyze(node->children[1]);
            return;
        }

        if (node->type == "Scan" || node->type == "Drift")
        {
            if (getType(node->children[0]) != "flag")
            {
                cout << "Semantic Error: condition must be flag (boolean)\n";
                exit(1);
            }
        }
        if (node->type == "Transmit")
        {
            for (auto child : node->children)
            {
                getType(child); 
            }
        }

        if (node->type == "Id")
        {
            if (!table.lookup(node->value))
            {
                cout << "Semantic Error: '" << node->value << "' not declared at line " << node->line << endl;
                exit(1);
            }
        }

        for (auto child : node->children)
            analyze(child);

        if (node->type == "Block")
        {
            depth--;
            if (depth > 0)
                table.popScope();
        }

        if (node->type == "Boost")
        {
            depth--;
            table.popScope();
        }
    }

    void printSymbolTable() { table.printTable(); }
};
// 2- Code Generator

class CodeGenerator {
    int indent = 0;

    string tab() { return string(indent * 4, ' '); }

    string mapType(const string& t) {
        if (t == "num") return "int";
        if (t == "decimal") return "double";
        if (t == "text") return "string";
        if (t == "flag") return "bool";
        return t;
    }
    string genConcat(ASTNode* node) {

        if (node->type != "Op" || node->value != "+") {
            return " << " + genExpr(node);
        }

        return genConcat(node->children[0]) +
               genConcat(node->children[1]);
    }

    string genExpr(ASTNode* node) {
        if (!node) return "";

        if (node->type == "Number") return node->value;
        if (node->type == "String") {
            string val = node->value;


            if (!val.empty() && val.front() == '"' && val.back() == '"') {
                val = val.substr(1, val.size() - 2);
            }

            return "\"" + val + "\"";
        }
        if (node->type == "Boolean") return node->value;
        if (node->type == "Id") return node->value;

        if (node->type == "Op") {
            string l = genExpr(node->children[0]);
            string r = genExpr(node->children[1]);
            return "(" + l + " " + node->value + " " + r + ")";
        }

        return "";
    }


    string gen(ASTNode* node) {
        if (!node) return "";

        if (node->type == "Program") {
            string code;
            code += "#include <iostream>\n";
            code += "using namespace std;\n\n";

            for (auto c : node->children)
                code += gen(c);

            return code;
        }



        if (node->type == "Main") {
            string code = "int main() ";
            code += gen(node->children.back());
            return code;
        }

        if (node->type == "Block") {
            string code = "{\n";
            indent++;

            for (auto c : node->children)
                code += tab() + gen(c);

            indent--;
            code += tab() + "}\n";
            return code;
        }

        if (node->type == "VarDecl") {
            string type = mapType(node->children[0]->value);
            string name = node->children[1]->value;

            string code = type + " " + name;

            if (node->children.size() > 2)
                code += " = " + genExpr(node->children[2]);

            return code + ";\n";
        }

        if (node->type == "Assign") {
            return node->children[0]->value + " = "
                + genExpr(node->children[1]) + ";\n";
        }

        if (node->type == "Transmit") {
            string code = "cout";
            for (auto child : node->children) {
                code += " << " + genExpr(child);
            }
            code += " << endl;\n";
            return code;
        }

        if (node->type == "Receive") {
            return "cin >> " + node->children[0]->value + ";\n";
        }

        if (node->type == "Land") {
            return "return " + genExpr(node->children[0]) + ";\n";
        }

        if (node->type == "Scan") {
            string code = "if (" + genExpr(node->children[0]) + ") ";
            code += gen(node->children[1]);

            if (node->children.size() > 2) {
                code += tab() + "else ";
                code += gen(node->children[2]);
            }

            return code;
        }

        if (node->type == "Drift") {
            return "while (" + genExpr(node->children[0]) + ") "
                + gen(node->children[1]);
        }

        if (node->type == "Boost") {
            ASTNode* init = node->children[0];
            ASTNode* cond = node->children[1];
            ASTNode* upd = node->children[2];
            ASTNode* blk = node->children[3];

            string type = mapType(init->children[0]->value);
            string name = init->children[1]->value;
            string val = genExpr(init->children[2]);

            string condition = genExpr(cond->children[0]);

            string uname = upd->children[0]->value;
            string op = upd->children[1]->value;

            return "for (" + type + " " + name + " = " + val + "; "
                + condition + "; " + uname + op + ") "
                + gen(blk);
        }

        return "";
    }



public:
    string generate(ASTNode* tree) {
        return gen(tree);
    }
};
int main()
{
    vector<Token> tokens = tokenize("test.nova");
    printTokens(tokens);
    Parser parser(tokens);
    ASTNode* tree = parser.parse();

    cout << "\n=========== AST ===========\n\n";
    printTree(tree);
    cout << "\n===========================\n";

    Semantic semantic;
    semantic.analyze(tree);
    semantic.printSymbolTable();
    CodeGenerator gen;
    string code = gen.generate(tree);

    ofstream out("output.cpp");
    out << code;
    out.close();

    cout << "\n======= GENERATED C++ =======\n\n";
    cout << code << endl;
    cout << "\n======= Output =======\n\n";
    cout.flush();

    system("g++ output.cpp -o prog.exe");
    system("prog.exe");

    freeTree(tree);
    return 0;
}
