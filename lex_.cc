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


#define BUFFER_SIZE 25
#define __EOF__ '\0'



class LITERAL_TABLE {

};

/*state to integer maping*/
enum STATE {
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
    K_CHECK
};

const char* getStateName(STATE state) {
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
        case K_CHECK: return "K_CHECK";
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



class TOKEN {
    
};


struct SYMBOL_TABLE_ENTERY {
    size_t t_id;
    TOKEN_CLASS t_class;
    DATA_TYPE t_type;
    int t_line_number;

    //lexeme is stored as key in hash map
};

class SYMBOL_TABLE {
    std::unordered_map<std::string, TOKEN>  table;

public:



};

class Transitions {
    STATE error_state = STATE::ERROR_STATE;
    std::unordered_map<char, STATE> transitions;
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

    STATE& operator () (char character) {
        return transitions[character];
    }

    STATE& operator () (const std::function<bool(char)> func) {
        transition_functions.push_back(std::pair<std::function<bool(char)>, STATE>(func, STATE::ERROR_STATE));
        return transition_functions.back().second;
    }
    static bool isValidCharacter(char c) {
        return isdigit(c) || isalpha(c) || isspace(c)||valid_chars.count(c) > 0;

    }

    
};

// [review] double quotes
const std::set<char> Transitions::valid_chars {
    '(', ')', '{', '}', '[', ']', ':', '<', '>', '=', 
    '+', '-', '!', '|', '%', '&', '*', '/'
};

class TRANSITION_TABLE 
{
    std::unordered_map<STATE, Transitions> table;
    std::unordered_set<STATE> final_states 
    {STATE::N_FINAL, STATE::O_FINAL,STATE::P_FINAL,STATE::I_FINAL};
    //std::unordered_set<STATE> non_advance_state {};

    
    

    // [review] double quotes



public:

    Transitions& operator [] (STATE state) {
        return table[state];
    }
    bool isFinal(STATE s) {
        return final_states.count(s) > 0;
    }
    bool advance(STATE s) {
        return s != STATE::I_FINAL || s != STATE::K_CHECK;
    }
    TOKEN_CLASS getTokenClass(STATE s) {
        switch (s)
        {
        case STATE::O_FINAL:
            return TOKEN_CLASS::Operator;
            break;
        
        default:
            break;
        };
    }
};



class BUFFER {
private:
    int in_file_descriptor;  
    char buffer[2][BUFFER_SIZE];
    int buffer_in_use;  // 0 or 1, to track active buffer
    int bp, fp; 
    int bp_buffer;
    int fp_buffer;
    int current_lexeme_size = 0;
    ssize_t current_buffer_size; 

    bool loadBuffer() {
        if ( !isDescriptorSet() ) 
        {
            std::cerr << "FD for input buffer not set\n";
            return false;
        }

        buffer_in_use = 1 - buffer_in_use; // Swap buffers (0,1)
        current_buffer_size = read(in_file_descriptor, buffer[buffer_in_use], BUFFER_SIZE);
        if (current_buffer_size == -1) {
            std::cerr << "unable to load buffer\n";
            return false;
        }

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
        if(current_buffer_size == 0) {
            return __EOF__;
        }
        return buffer[buffer_in_use][fp];
    }
    bool advance() {
        if (fp >= current_buffer_size) {
            return loadBuffer();
        }
        fp++;
        current_lexeme_size++;
        if(current_lexeme_size >= BUFFER_SIZE) {
            std::cerr << "FATAL ERROR: lexeme buffer overflow" << std::endl;
            exit(1);
        }
        return true;

    }


    bool advanceBp(){
        if(current_lexeme_size == 0) 
            return false;
        bp = (bp + 1) % BUFFER_SIZE;
        current_lexeme_size--;
        return true;
    }

    std::string getLexeme() {

        if(current_lexeme_size == 0) return "l";
        std::string lexeme;
        int t_bp = bp;
        if (bp + current_lexeme_size > BUFFER_SIZE ) {   // lexeme divided into 2 buffers
            lexeme += std::string(buffer[1-buffer_in_use] + bp, buffer[1-buffer_in_use]+BUFFER_SIZE-1);
            t_bp = 0;
        }
        lexeme += std::string(buffer[buffer_in_use]+t_bp, buffer[buffer_in_use] + fp-1);
        bp = fp;
        current_lexeme_size = 0;
        
        return lexeme;

    }
};


std::vector<TOKEN> Lexer
    (BUFFER& buffer, 
    SYMBOL_TABLE& symbol_table, 
    TRANSITION_TABLE& transition_table, 
    std::unordered_set<std::string>& keywords) 
{
    std::vector<TOKEN> tokens;
    char c;
    STATE state = STATE::START;
    STATE new_state = STATE::START;
    while ( buffer.peekNextCharacter() != __EOF__) {

        c = buffer.peekNextCharacter();
        std::cout << getStateName(state) << " " << c << std::endl;
        

        new_state = transition_table[state][c];

        if(state == STATE::ERROR_STATE){
            //std::cout << getStateName(state) << " " << c << std::endl;
            std::cerr << "\nerror\n";
            state = STATE::START;
            buffer.advance();
            continue;
        }
        if (transition_table.isFinal(state) || state == STATE::K_CHECK) {
            //if(transition_table.advance(state))
            std::cout << getStateName(state) << " " << c << std::endl;
            std::cout << buffer.getLexeme() << "|"<<std::endl;
            buffer.advance();
            if(isspace(c)){
                buffer.advanceBp();
            }
            //skip space
            c = buffer.peekNextCharacter();
            if(isspace(c)){
                buffer.advance();
                buffer.advanceBp();
            }
            //push in st if not there already st
            
            state = STATE::START;
        }
        else {
            buffer.advance();
        }
        state = new_state;
    }


}

int Scanner(const char *filename) {
    
    //[review] remove initial whitespace

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
                        //out_buffer[out_file_index++] = '\n';
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
                    remove_one_line = false;
                    //out_buffer[out_file_index++] = '\n';
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
                out_buffer[out_file_index++] = buffer[i];
                lastWhitespace = true;
                continue;
                
            }
            

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




bool loadTransitions(TRANSITION_TABLE& transition_table) {
    transition_table[STATE::START]([](char c) { return isdigit(c); }) = STATE::N1;
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

    // [review] string quotes
    transition_table[STATE::START]('“') = STATE::O_FINAL;
    transition_table[STATE::START]('”') = STATE::O_FINAL;

    transition_table[STATE::N1]([](char c) { return isdigit(c); }) = STATE::N1;
    transition_table[STATE::N1]('.') = STATE::N_DECIMAL;
    transition_table[STATE::N1]('E') = STATE::N_EXP;

    transition_table[STATE::N_DECIMAL]([](char c) { return isdigit(c); }) = STATE::N_DECIMAL_N;
    transition_table[STATE::N_DECIMAL_N]([](char c) { return isdigit(c); }) = STATE::N_DECIMAL_N;
    transition_table[STATE::N_DECIMAL_N]('E') = STATE::N_EXP;

    transition_table[STATE::N_EXP]([](char c) { return isdigit(c); }) = STATE::N_EXP_N;
    transition_table[STATE::N_EXP]('+') = STATE::N_EXP_ADD_SUB;
    transition_table[STATE::N_EXP]('-') = STATE::N_EXP_ADD_SUB;
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
    transition_table[STATE::O_GT]('=') = STATE::O_FINAL;
    transition_table[STATE::O_ET]('=') = STATE::O_FINAL;
    transition_table[STATE::O_ADD]('=') = STATE::O_FINAL;
    transition_table[STATE::O_SUBTRACT]('=') = STATE::O_FINAL;
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
 
    return true;

}


int main() {

    const char* input_filename = "_src.ucc";
    double a = +123.322E+123;
    std::unordered_set<std::string> keywords = {
        "asm", "Wagarna", "new", "this", "auto", "enum", "operator", "throw", "Mantiqi",
        "explicit", "private", "True", "break", "export", "protected", "try", "case", 
        "extern", "public", "typedef", "catch", "False", "register", "typeid", "Harf", 
        "Ashriya", "typename", "Adadi", "class", "for", "Wapas", "union", "const", 
        "dost", "short", "unsigned", "goto", "signed", "using", "continue", "Agar", 
        "sizeof", "virtual", "default", "inline", "static", "Khali", "delete", 
        "volatile", "do", "long", "struct", "double", "mutable", "switch", "while", 
        "namespace", "template", "Marqazi", "Matn"
    };

    TRANSITION_TABLE transition_table;

    loadTransitions(transition_table);
    
    //Define all transitions
    SYMBOL_TABLE symbol_table;


    Scanner(input_filename);
    BUFFER buffer((std::string(input_filename) + ".Meow").c_str());

    Lexer(buffer,symbol_table,transition_table, keywords );



    // int i = 0;
    // while(buffer.peekNextCharacter() != __EOF__) {
    //     ++i;
    //     char c = buffer.peekNextCharacter();
    //     if(i == 3) {
    //         i = 0;
    //         std::cout << buffer.getLexeme() << std::endl;
    //     }
    //     buffer.advance();

    // }
    

    
}