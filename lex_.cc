#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <cctype>
#include <optional>

#include <ctime>


#define BUFFER_SIZE 25
#define __EOF__ '\0'

const std::string ROLL_NO = "22L-6895_";

//[review] last space


/*state to integer maping*/
enum STATE : u_int8_t {
    ERROR_STATE,
    START,
    N1,
    N_DECIMAL,
    N_DECIMAL_N,
    N_EXP,
    N_EXP_ADD_SUB,
    N_EXP_N,
    N_FINAL,
    I1,
    I_UND,
    I_FINAL,
    O_LT,
    O_GT,
    O_ET,
    O_ADD,
    O_SUBTRACT,
    O_NOT,
    O_OR,
    O_AND,
    P_COLON,
    P_FINAL,
    O_FINAL,
    SL1,
    SL_FINAL,
    K_CHECK,
    K_OUTPUT,
    K_OUTPUT_LT,
    K_INPUT_SUB,
    K_INPUT,
    K_FINAL
};


std::string getStateName(STATE state) {
    switch (state) {
        case ERROR_STATE: return "ERROR_STATE";
        case START: return "START";
        case N1: return "N1";
        case N_DECIMAL: return "N_DECIMAL";
        case N_DECIMAL_N: return "N_DECIMAL_N";
        case N_EXP: return "N_EXP";
        case N_EXP_ADD_SUB: return "N_EXP_ADD_SUB";
        case N_EXP_N: return "N_EXP_N";
        case N_FINAL: return "N_FINAL";
        case I1: return "I1";
        case I_UND: return "I_UND";
        case I_FINAL: return "I_FINAL";
        case O_LT: return "O_LT";
        case O_GT: return "O_GT";
        case O_ET: return "O_ET";
        case O_ADD: return "O_ADD";
        case O_SUBTRACT: return "O_SUBTRACT";
        case O_NOT: return "O_NOT";
        case O_OR: return "O_OR";
        case O_AND: return "O_AND";
        case P_COLON: return "P_COLON";
        case P_FINAL: return "P_FINAL";
        case O_FINAL: return "O_FINAL";
        case SL1: return "SL1";
        case SL_FINAL: return "SL_FINAL";
        case K_CHECK: return "K_CHECK";
        case K_OUTPUT: return "K_OUTPUT";
        case K_OUTPUT_LT: return "K_OUTPUT_LT";
        case K_INPUT_SUB: return "K_INPUT_SUB";
        case K_INPUT: return "K_INPUT";
        case K_FINAL: return "K_FINAL";
        default: return "UNKNOWN_STATE";
    }
}

enum TOKEN_CLASS {
    ERROR,
    Keyword,
    Operator,
    Identifier,
    Number,
    String_Literal,
    Punctuation
};



enum DATA_TYPE {
    T_DEFAULT
};



std::string tokenClassToString(TOKEN_CLASS token) {
    switch (token) {
        case ERROR: return "ERROR";
        case Keyword: return "Keyword";
        case Operator: return "Operator";
        case Identifier: return "Identifier";
        case Number: return "Number";
        case String_Literal: return "String_Literal";
        case Punctuation: return "Punctuation";
        default: return "Unknown";
    }
}

std::string dataTypeToString(DATA_TYPE type) {
    switch (type) {
        case T_DEFAULT: return "T_DEFAULT";
        default: return "Unknown";
    }
}


struct TOKEN {
    ssize_t t_id;
    std::optional<std::string> t_lexeme;
    TOKEN_CLASS t_class;
    int line_number;
    int column_number;

    TOKEN(ssize_t id=-1,const std::optional<std::string>& lexeme = std::nullopt, TOKEN_CLASS tclass = TOKEN_CLASS::ERROR, int line = 0, int column = 0)
        : t_id(id), t_lexeme(lexeme), t_class(tclass), line_number(line), column_number(column) {}


    std::string toString() const{

        return "<"+ (t_id == -1 ? "" : std::to_string(t_id) + ", ")+ 
        (t_lexeme.has_value()? t_lexeme.value() +  ", " : "")+ tokenClassToString(t_class)+  ">";
    }
};
struct SYMBOL_TABLE_ENTRY {
    TOKEN_CLASS token_class;
    std::string lexeme;
    DATA_TYPE datatype;
    size_t memory_location;

    SYMBOL_TABLE_ENTRY(TOKEN_CLASS _class = TOKEN_CLASS::ERROR, const std::string& _lexeme = "", DATA_TYPE _datatype = DATA_TYPE::T_DEFAULT, size_t _memory_location = 0)
        : token_class(_class), lexeme(_lexeme), datatype(_datatype), memory_location(_memory_location) {}

    std::string toString() const{
        return lexeme 
           + ", " + tokenClassToString(token_class)
           + ", " + dataTypeToString(datatype);
    }
};

struct LITERAL_TABLE_ENTRY {
    std::string value;
    DATA_TYPE datatype;

    LITERAL_TABLE_ENTRY(const std::string& _value = "", DATA_TYPE _datatype = DATA_TYPE::T_DEFAULT)
        : value(_value), datatype(_datatype) {}

    std::string toString() const{
        return  value 
           + ", " + dataTypeToString(datatype);
    }
};


template <typename T>
class TABLE {
    std::unordered_map<std::string, size_t> lexeme_to_index;
    std::vector<T> entries;

public:
    T* find(const std::string& lexeme) const {
        auto it = lexeme_to_index.find(lexeme);
        if (it != lexeme_to_index.end()) {
            return &entries[it->second];
        }
        return nullptr;
    }

    size_t insert(const std::string& lexeme, const T& entry) {
        auto it = lexeme_to_index.find(lexeme);
        if (it != lexeme_to_index.end()) {
            return it->second;
        }

        size_t index = entries.size();
        entries.push_back(entry);
        lexeme_to_index[lexeme] = index;
        return index;
    }

    

    bool writeToFile(const std::string& filename) const {
        int fd = open(filename.c_str(), O_CREAT|O_WRONLY,0644);

        if(fd == -1) {
            std::cerr << "TABLE:writeToFile() unable to open file\n";
            return false;
        }

        int i=0;
        std::string buffer;
        for(const auto& e:entries) { 
            buffer = std::to_string(i)+ "\t"+e.toString() + "\n";
            ssize_t bytes_written = write(fd, buffer.c_str(),buffer.size());
            if (bytes_written == -1) {
                std::cerr << "TABLE:writeToFile() unable to write to file\n";
                close(fd);
                return false;
            }
            ++i;
        }
        close(fd);

        return true;
    }
};




class Transitions {

    STATE error_state = STATE::ERROR_STATE;
    std::unordered_map<char, STATE> transitions;
    std::unordered_map<std::string, STATE> keyword_transitions;
    std::vector<std::pair<std::function<bool(char)>, STATE>> transition_functions;
    static const std::set<char> valid_chars;

    

public:

    STATE operator [] (char character) {

        if (transitions.find(character) == transitions.end()) {
            for (auto& f : transition_functions) {
                if (f.first(character)) {
                    return f.second;
                }
            }
            return error_state;
        } 
        return transitions[character];
    }

    STATE operator [] (const std::string& keyword) {
        if (keyword_transitions.find(keyword) == keyword_transitions.end()) {
            return error_state;
        }
        return keyword_transitions[keyword];
    }

    STATE& operator () (char character) {
        return transitions[character];
    }

    STATE& operator () (const std::string& keyword) {
        return keyword_transitions[keyword];
    }

    STATE& operator () (const std::function<bool(char)> func) {
        transition_functions.push_back(std::pair<std::function<bool(char)>, STATE>(func, STATE::ERROR_STATE));
        return transition_functions.back().second;
    }

    static bool isValidCharacter(char c) {
        return isdigit(c) || c == __EOF__ ||isalpha(c) || isspace(c)|| valid_chars.count(c) > 0;
    }
    
};

// [review] double quotes
const std::set<char> Transitions::valid_chars {
    '(', ')', '{', '}', '[', ']', ':', '<', '>', '=', 
    '+', '-', '!', '|', '%', '&', '*', '/', '"'
};

class TRANSITION_TABLE 
{
    std::unordered_map<STATE, Transitions> table;
    std::unordered_set<STATE> final_states 
    {
        STATE::N_FINAL, 
        STATE::O_FINAL,
        STATE::P_FINAL,
        STATE::I_FINAL,
        STATE::K_FINAL,
        STATE::K_CHECK,
        STATE::SL_FINAL,
    };

    std::unordered_set<STATE> not_advance = {
        STATE::N1,
        STATE::N_DECIMAL_N,
        STATE::N_EXP_N,
        STATE::I1,
        STATE::I_UND,
        STATE::O_LT,
        STATE::O_GT,
        STATE::O_ET,
        STATE::O_ADD,
        STATE::O_SUBTRACT
    };

public:
    Transitions& operator [] (STATE state) {
        return table[state];
    }
    bool isFinal(STATE s) {
        return final_states.count(s) > 0;
    }
    bool advance(STATE previous_state, STATE new_state) {
        if(!isFinal(new_state)) {
            return true;
        }
        return not_advance.count(previous_state) == 0;
    }
    TOKEN_CLASS getTokenClass(STATE s) {
        switch (s) {
            case STATE::N_FINAL:
                return TOKEN_CLASS::Number;     
            case STATE::O_FINAL:
                return TOKEN_CLASS::Operator;    
            case STATE::P_FINAL:
                return TOKEN_CLASS::Punctuation;  
            case STATE::I_FINAL:
                return TOKEN_CLASS::Identifier;   
            case STATE::K_FINAL:
                return TOKEN_CLASS::Keyword;
            case STATE::SL_FINAL:
                return TOKEN_CLASS::String_Literal;       

            default:
                return TOKEN_CLASS::ERROR;      
        }
    }

};


class BUFFER {
private:
    int in_file_descriptor;  
    char buffer[2][BUFFER_SIZE+1];
    int buffer_in_use;  // 0 or 1, to track active buffer
    int bp, fp; 
    int current_lexeme_size = 0;
    ssize_t current_buffer_size; 

    bool loadBuffer() {
        if ( !isDescriptorSet() ) 
        {
            std::cerr << "FD for input buffer not set\n";
            return false;
        }
        bool last_size_zero = current_buffer_size == 0;

        current_buffer_size = read(in_file_descriptor, buffer[1-buffer_in_use], BUFFER_SIZE);
        if (current_buffer_size == -1) {
            std::cerr << "unable to load buffer\n";
            return false;
        }

        buffer_in_use = 1 - buffer_in_use; // Swap buffers (0,1)
        fp = 0;
        
        return current_buffer_size > 0;
    }

public:

    BUFFER(const char* filename = nullptr)
        : in_file_descriptor(-1), buffer_in_use(0), bp(0), fp(0), current_buffer_size(0) {
        if (filename) {
            if (!setFile(filename)) {
                std::cerr << "Error opening file: " << filename << "\n";
            }
        }
    }

    ~BUFFER() {
        if (in_file_descriptor >= 0) {
            close(in_file_descriptor);
        }
    }

    bool setFile(const char* filename) {
        in_file_descriptor = open(filename, O_RDONLY);
        return in_file_descriptor != -1 && loadBuffer();
    }

    bool isDescriptorSet() {
        return in_file_descriptor >= 0;
    }

    char peekNextCharacter() {
        return fp >= current_buffer_size? __EOF__: buffer[buffer_in_use][fp];
    }

    bool advance() {

        if(fp >= current_buffer_size) {
            return false;
        }
        current_lexeme_size++;

        if (fp >= current_buffer_size-1) {
            if(current_buffer_size < BUFFER_SIZE){
                fp = current_buffer_size;
                return true;
            }
            return loadBuffer();
        }
        fp++;

        if(current_lexeme_size >= BUFFER_SIZE) {
            std::cerr << "FATAL ERROR: lexeme buffer overflow" << std::endl;
            exit(EXIT_FAILURE);
        }
        return true;

    }


    bool advanceBp(){
        if(current_lexeme_size == 0) 
            return false;
        bp = (++bp) % BUFFER_SIZE;
        --current_lexeme_size;
        return true;
    }

    std::string peekLexeme() {

        if(bp == fp) return "";
        std::string lexeme;
        int t_bp = bp;
        if (bp + current_lexeme_size >= BUFFER_SIZE) {   // lexeme divided into 2 buffers
            lexeme += std::string(buffer[1-buffer_in_use] + bp, buffer[1-buffer_in_use]+BUFFER_SIZE);
            t_bp = 0;
        }
        lexeme += std::string(buffer[buffer_in_use]+t_bp, buffer[buffer_in_use] + fp);
        
        
        return lexeme;

    }

    std::string popLexeme() {
        std::string lexeme = BUFFER::peekLexeme();
        bp = fp;
        current_lexeme_size = 0;
        return lexeme;
    }
};
    


class Lexer {
private:
    BUFFER buffer;
    TABLE<SYMBOL_TABLE_ENTRY>& symbol_table;
    TABLE<LITERAL_TABLE_ENTRY>& literal_table;

    TRANSITION_TABLE transition_table;
    std::unordered_set<std::string>& keywords;

    int last_token_line_no=0;
    int last_token_column_number=0;
    int current_token_lines=0;
    int current_token_columns=0;

    bool loadTransitions() {
        transition_table[STATE::START]([](char c) { return isdigit(c); }) = STATE::N1;
        transition_table[STATE::START]([](char c) { return isspace(c); }) = STATE::P_FINAL;
        transition_table[STATE::START]('_') = STATE::I_UND;
        transition_table[STATE::START]([](char c) { return isalpha(c); }) = STATE::I1;
        transition_table[STATE::START]('[') = STATE::P_FINAL;
        transition_table[STATE::START](']') = STATE::P_FINAL;
        transition_table[STATE::START]('(') = STATE::P_FINAL;
        transition_table[STATE::START](')') = STATE::P_FINAL;
        transition_table[STATE::START]('{') = STATE::P_FINAL;
        transition_table[STATE::START]('}') = STATE::P_FINAL;
        transition_table[STATE::START](':') = STATE::P_COLON;
        transition_table[STATE::START]('<') = STATE::O_LT;
        transition_table[STATE::START]('>') = STATE::O_GT;
        transition_table[STATE::START]('=') = STATE::O_ET;
        transition_table[STATE::START]('+') = STATE::O_ADD;
        transition_table[STATE::START]('-') = STATE::O_SUBTRACT;
        transition_table[STATE::START]('!') = STATE::O_NOT;
        transition_table[STATE::START]('|') = STATE::O_OR;
        transition_table[STATE::START]('%') = STATE::O_FINAL;
        transition_table[STATE::START]('&') = STATE::O_AND;
        transition_table[STATE::START]('*') = STATE::O_FINAL;
        transition_table[STATE::START]('/') = STATE::O_FINAL;

        // [review] string valid characters
        transition_table[STATE::START]('"') = STATE::SL1;
        transition_table[STATE::SL1]([](char c) { return (c >= 32 || c == '\t' || c == '\n' || c == '\r'); }) = STATE::SL1;
        transition_table[STATE::SL1]('"') = STATE::SL_FINAL;


        transition_table[STATE::N1]([](char c) { return isdigit(c); }) = STATE::N1;
        transition_table[STATE::N1]('.') = STATE::N_DECIMAL;
        transition_table[STATE::N1]('E') = STATE::N_EXP;

        transition_table[STATE::N_DECIMAL]([](char c) { return isdigit(c); }) = STATE::N_DECIMAL_N;
        transition_table[STATE::N_DECIMAL_N]([](char c) { return isdigit(c); }) = STATE::N_DECIMAL_N;
        transition_table[STATE::N_DECIMAL_N]('E') = STATE::N_EXP;

        transition_table[STATE::N_EXP]([](char c) { return isdigit(c); }) = STATE::N_EXP_N;
        transition_table[STATE::N_EXP]('+') = STATE::N_EXP_ADD_SUB;
        transition_table[STATE::N_EXP]('-') = STATE::N_EXP_ADD_SUB;
        transition_table[STATE::N_EXP_ADD_SUB]([](char c) { return isdigit(c); }) = N_EXP_N;
        transition_table[STATE::N_EXP_N]([](char c) { return isdigit(c); }) = STATE::N_EXP_N;

        transition_table[STATE::I_UND]([](char c) { return isdigit(c); }) = STATE::I_UND;
        transition_table[STATE::I_UND]([](char c) { return isalpha(c); }) = STATE::I_UND;
        transition_table[STATE::I_UND]('-') = STATE::I_UND;
        transition_table[STATE::I1]([](char c) { return isdigit(c); }) = STATE::I1;
        transition_table[STATE::I1]([](char c) { return isalpha(c); }) = STATE::I1;
        transition_table[STATE::I1]('_') = STATE::I_UND;

        transition_table[STATE::P_COLON]('=') = STATE::O_FINAL;
        transition_table[STATE::P_COLON](':') = STATE::P_FINAL;
        transition_table[STATE::O_LT]('=') = STATE::O_FINAL;
        transition_table[STATE::O_LT]('<') = STATE::O_FINAL;
        transition_table[STATE::O_LT]('>') = STATE::O_FINAL;
        transition_table[STATE::O_GT]('=') = STATE::O_FINAL;
        transition_table[STATE::O_GT]('>') = STATE::O_FINAL;
        transition_table[STATE::O_ET]('=') = STATE::O_FINAL;
        transition_table[STATE::O_ADD]('=') = STATE::O_FINAL;
        transition_table[STATE::O_ADD]('+') = STATE::O_FINAL;
        transition_table[STATE::O_ADD]([](char c) {return isdigit(c);}) = STATE::N1;
        transition_table[STATE::O_SUBTRACT]([](char c) {return isdigit(c);}) = STATE::N1;
        transition_table[STATE::O_NOT]('=') = STATE::O_FINAL;
        transition_table[STATE::O_OR]('|') = STATE::O_FINAL;
        transition_table[STATE::O_AND]('&') = STATE::O_FINAL;


        //others

        transition_table[STATE::N1]([](char c) { return Transitions::isValidCharacter(c); }) = STATE::N_FINAL;
        transition_table[STATE::N_DECIMAL_N]([](char c) { return Transitions::isValidCharacter(c); }) = STATE::N_FINAL;
        transition_table[STATE::N_EXP_N]([](char c) { return Transitions::isValidCharacter(c); }) = STATE::N_FINAL;
        transition_table[STATE::I1]([](char c) { return Transitions::isValidCharacter(c); }) = STATE::K_CHECK;
        transition_table[STATE::I_UND]([](char c) { return Transitions::isValidCharacter(c); }) = STATE::I_FINAL;
        transition_table[STATE::O_LT]([](char c) { return Transitions::isValidCharacter(c); }) = STATE::O_FINAL;
        transition_table[STATE::O_GT]([](char c) { return Transitions::isValidCharacter(c); }) = STATE::O_FINAL;
        transition_table[STATE::O_ET]([](char c) { return Transitions::isValidCharacter(c); }) = STATE::O_FINAL;
        transition_table[STATE::O_ADD]([](char c) { return Transitions::isValidCharacter(c); }) = STATE::O_FINAL;
        transition_table[STATE::O_SUBTRACT]([](char c) { return Transitions::isValidCharacter(c); }) = STATE::O_FINAL;
    

        //keywords
        transition_table[STATE::K_CHECK]("output") = STATE::K_OUTPUT;
        transition_table[STATE::K_CHECK]("input") = STATE::K_INPUT;
        transition_table[STATE::K_INPUT]('-') = STATE::K_INPUT_SUB;
        transition_table[STATE::K_INPUT_SUB]('>') = K_FINAL;
        transition_table[STATE::K_OUTPUT]('<') = K_OUTPUT_LT;
        transition_table[STATE::K_OUTPUT_LT]('-') = K_FINAL;

        return true;
    }

    std::string getErrorStatement(STATE state, STATE new_state) {
        if (new_state != STATE::ERROR_STATE) {
            return "No Error";
        }
        switch (state) {
            case STATE::N_DECIMAL:
                return "Invalid floating point number.";
            case STATE::I1:
                return "Invalid identifier.";
            case STATE::O_NOT:
                return "Invalid logical NOT operator.";
            case STATE::SL1:
                return "Unterminated string literal.";
            case STATE::P_COLON:
                return "Unexpected character after ':' symbol.";
            case STATE::N_EXP:
                return "Malformed exponent in floating point number.";
            default:
                return "Unrecognized token encountered.";
        }
    }

public:
    Lexer(const char* filename, TABLE<SYMBOL_TABLE_ENTRY>& symTable, TABLE<LITERAL_TABLE_ENTRY>& litTable,std::unordered_set<std::string>& kw)
        : symbol_table(symTable), literal_table(litTable),keywords(kw) {
        loadTransitions();
        buffer.setFile(filename);
    }

    bool setBuffer(const char* filename) {
        return buffer.setFile(filename);
    }
    inline void transition(STATE &state, STATE &new_state,const STATE &next_state) {
        state=new_state;
        new_state=next_state;
    }
    TOKEN getNextToken() {
        char c;
        STATE state = STATE::START;
        STATE new_state = STATE::START;
        std::string t_lexeme;
        size_t token_id = -1;
        TOKEN_CLASS token_class;
    
        while (buffer.peekNextCharacter() != __EOF__) {
            t_lexeme = "";

            if (isspace(buffer.peekNextCharacter())) {
                buffer.advance();
                buffer.advanceBp();
            }
            while (!transition_table.isFinal(new_state) && new_state != STATE::ERROR_STATE ) {
                c=buffer.peekNextCharacter();
                transition(state, new_state,transition_table[new_state][c]);
                if (transition_table.advance(state, new_state))
                    buffer.advance();
            }
            if (new_state == STATE::K_CHECK) {
                t_lexeme = buffer.peekLexeme();
                if (keywords.count(t_lexeme) == 0) {
                    transition(state, new_state,transition_table[new_state][t_lexeme]);
                    if (new_state != STATE::ERROR_STATE)
                        continue;
                }
                transition(state, new_state,K_FINAL);
            } else if (new_state == STATE::ERROR_STATE || new_state == STATE::SL1) {

                //for string literal
                state = new_state==STATE::ERROR_STATE? state :new_state;
                new_state = STATE::ERROR_STATE;

                std::cout <<"[LEX ERROR]: " <<getErrorStatement(state, new_state) << std::endl;
                new_state = STATE::START;
                buffer.popLexeme();
                continue;
            }
            t_lexeme = buffer.popLexeme();
            token_class = transition_table.getTokenClass(new_state);
            if (token_class == TOKEN_CLASS::Identifier) {
                token_id = symbol_table.insert(t_lexeme, SYMBOL_TABLE_ENTRY(token_class, t_lexeme, DATA_TYPE::T_DEFAULT));
                return TOKEN(token_id,  std::nullopt, token_class, 0, 0);
            } else if (token_class == TOKEN_CLASS::Number || token_class == TOKEN_CLASS::String_Literal) {
                token_id = literal_table.insert(t_lexeme, LITERAL_TABLE_ENTRY(t_lexeme,DATA_TYPE::T_DEFAULT));
                return TOKEN(token_id, std::nullopt,token_class, 0, 0);
            }

            break;
            
        }
        return TOKEN(-1, t_lexeme, token_class, 0, 0);
    }

    bool isEmpty() {
        return buffer.peekNextCharacter() == __EOF__;
    }
};
    
int Scanner(const char *filename) {
    

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error opening file: " << filename << "\n";
        return -1;
    }

    const int buffer_size = BUFFER_SIZE;
    char buffer[buffer_size];
    char out_buffer[buffer_size];
    int bytes_read = 0;

    std::string scanned_filename = std::string(filename) + ".Meow";
    int out_fd = open(scanned_filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd == -1) {
        std::cerr << "Error opening output file: " << scanned_filename << "\n";
        close(fd);
        return -1;
    }

    bool lastSlash = false;
    bool remove_one_line = false;
    bool remove_multi_line = false;
    int out_file_index = 0;
    bool lastStar = false;
    bool lastWhitespace = false;
    bool last_space = false;
    bool last_comment = false;

    while ((bytes_read = read(fd, buffer, buffer_size)) > 0) {

        for (int i = 0; i < bytes_read; i++) {

            // write in out file buffer is full
            if (out_file_index >= buffer_size) {
                write(out_fd, out_buffer, out_file_index);
                out_file_index = 0;
            }

            // if last char was backslash check if comment starts
            if (lastSlash) {                             
                remove_multi_line = (buffer[i] == '*');
                remove_one_line = (buffer[i] == '/');
                if (remove_multi_line || remove_one_line) {
                    ++i;
                } else {
                    out_buffer[out_file_index++] = '/';
                }
                lastSlash = false;
            }

            if (remove_multi_line) {
                while (i < bytes_read - 1) {
                    
                    if (lastStar && buffer[i] == '/') {
                        remove_multi_line = false;
                        lastStar = false;
                        i++;            // Skip '/'

                        if(!last_space)                         //replace with white space
                            out_buffer[out_file_index++] = ' ';
                        last_comment=true;
                        break;
                    }
                    lastStar = buffer[i] == '*'; 
                    i++;
                }
                    
                continue; // Skip remaining comment characters
            }

            if (remove_one_line) {
                while (i < bytes_read && buffer[i] != '\n') {
                    i++;
                }
                if ( i < bytes_read)
                {
                    if(!last_space)
                        out_buffer[out_file_index++] = ' ';
                    remove_one_line = false;
                    last_comment=true;
                }
                continue;
            }

            if (buffer[i] == '/') {
                lastSlash = true;
                continue;
            }

            

            //skip whitespaces
            if(lastWhitespace) {
                while (isspace(buffer[i]) && i < bytes_read){
                    ++i;
                } 
                if(i < bytes_read) {
                    lastWhitespace = false;
                    i--;
                }

                continue;
            }

            if (isspace(buffer[i])) {
                if(!last_comment)
                    out_buffer[out_file_index++] = buffer[i];
                lastWhitespace = true;
                last_space=true;
                continue;
                
            }

            last_space=last_comment=false;

            out_buffer[out_file_index++] = buffer[i];
            
        }
    }

    if (out_file_index > 0) {
        write(out_fd, out_buffer, out_file_index);
    }

    close(fd);
    close(out_fd);
    return 1;
}


bool writeTokenToFile(std::vector<TOKEN>& token_stream, std::string& filename) {
    int fd = open(filename.c_str(), O_CREAT|O_WRONLY,0644);

    if(fd == -1) {
        std::cerr << "writeTokenToFile() unable to open file\n";
        return false;
    }

    int i=0;
    std::string buffer;
    for(const auto& e:token_stream) { 
        buffer = std::to_string(i)+ "\t"+ e.toString() + "\n";
        ssize_t bytes_written = write(fd, buffer.c_str(),buffer.size());
        if (bytes_written == -1) {
            std::cerr << "writeTokenToFile() unable to write to file\n";
            close(fd);
            return false;
        }
        ++i;
    }
    close(fd);

    return true;
}




int main() {


    clock_t start_time = clock(); 
    const char* input_filename = "src.ucc";
    std::unordered_set<std::string> keywords = {
        "asm", "Wagarna", "new", "this", "auto", "enum", "operator", "throw", "Mantiqi",
        "explicit", "private", "True", "break", "export", "protected", "try", "case", 
        "extern", "public", "typedef", "catch", "False", "register", "typeid", "Harf", 
        "Ashriya", "typename", "Adadi", "class", "for", "Wapas", "union", "const", 
        "dost", "short", "unsigned", "goto", "signed", "using", "continue", "Agar", 
        "sizeof", "virtual", "default", "inline", "static", "Khali", "delete", 
        "volatile", "do", "long", "struct", "double", "mutable", "switch", "while", 
        "namespace", "template", "Marqazi", "Matn", "output->", "input<-"
    };
    
    
    TABLE<SYMBOL_TABLE_ENTRY> symbol_table;
    TABLE<LITERAL_TABLE_ENTRY> literal_table;
    std::vector<TOKEN> token_stream;

    Scanner(input_filename);

    
    double scanner_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    std::cout <<"Scan complete. Time: " <<scanner_time << std::endl;
    

    Lexer lex((std::string(input_filename) + ".Meow").c_str(),symbol_table,literal_table,keywords);
    while (!lex.isEmpty())
    {
        token_stream.push_back(std::move(lex.getNextToken()));
    }


    std::cout << "\n\n--------TOKEN STREAM-------\n\n";
    for (const auto& token: token_stream) {
        std::cout << token.toString() << std::endl;
    }

    std::cout <<"Tokens generated. Time: " << (double)(clock() - start_time) / CLOCKS_PER_SEC - scanner_time << std::endl;
 

    std::cout << "Generated text files\n";
    symbol_table.writeToFile("symbol_table_"+ ROLL_NO +std::string(input_filename)+".txt");
    literal_table.writeToFile("literal_table_" + ROLL_NO +std::string(input_filename)+ +".txt");

    return EXIT_SUCCESS;    
}