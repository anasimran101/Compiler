#ifndef LEXER_HPP
#define LEXER_HPP

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

std::string getStateName(STATE state);

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

std::string tokenClassToString(TOKEN_CLASS token);
std::string dataTypeToString(DATA_TYPE type);

struct TOKEN {
    ssize_t t_id;
    std::optional<std::string> t_lexeme;
    TOKEN_CLASS t_class;
    int line_number;
    int column_number;

    TOKEN(ssize_t id=-1, const std::optional<std::string>& lexeme = std::nullopt, TOKEN_CLASS tclass = TOKEN_CLASS::ERROR, int line = 0, int column = 0);
    std::string toString() const;
};

struct SYMBOL_TABLE_ENTRY {
    TOKEN_CLASS token_class;
    std::string lexeme;
    DATA_TYPE datatype;
    size_t memory_location;

    SYMBOL_TABLE_ENTRY(TOKEN_CLASS _class = TOKEN_CLASS::ERROR, const std::string& _lexeme = "", DATA_TYPE _datatype = DATA_TYPE::T_DEFAULT, size_t _memory_location = 0);
    std::string toString() const;
};

struct LITERAL_TABLE_ENTRY {
    std::string value;
    DATA_TYPE datatype;

    LITERAL_TABLE_ENTRY(const std::string& _value = "", DATA_TYPE _datatype = DATA_TYPE::T_DEFAULT);
    std::string toString() const;
};

template <typename T>
class TABLE {
    std::unordered_map<std::string, size_t> lexeme_to_index;
    std::vector<T> entries;

public:
    const T* find(const std::string& lexeme) const {
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

    bool writeToFile(const std::string& filename) const;
};

class Transitions {
    STATE error_state = STATE::ERROR_STATE;
    std::unordered_map<char, STATE> transitions;
    std::unordered_map<std::string, STATE> keyword_transitions;
    std::vector<std::pair<std::function<bool(char)>, STATE>> transition_functions;
    static const std::set<char> valid_chars;

public:
    STATE operator[](char character);
    STATE operator[](const std::string& keyword);
    STATE& operator()(char character);
    STATE& operator()(const std::string& keyword);
    STATE& operator()(const std::function<bool(char)> func);
    static bool isValidCharacter(char c);
};

class TRANSITION_TABLE {
    std::unordered_map<STATE, Transitions> table;
    std::unordered_set<STATE> final_states{
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
    Transitions& operator[](STATE state);
    bool isFinal(STATE s);
    bool advance(STATE previous_state, STATE new_state);
    TOKEN_CLASS getTokenClass(STATE s);
};

class BUFFER {
private:
    int in_file_descriptor;  
    char buffer[2][BUFFER_SIZE+1];
    int buffer_in_use;  // 0 or 1, to track active buffer
    int bp, fp; 
    int current_lexeme_size = 0;
    ssize_t current_buffer_size; 

    bool loadBuffer();

public:
    BUFFER(const char* filename = nullptr);
    ~BUFFER();
    bool setFile(const char* filename);
    bool isDescriptorSet();
    char peekNextCharacter();
    bool advance();
    bool advanceBp();
    std::string peekLexeme();
    std::string popLexeme();
};

class Lexer {
private:
    BUFFER buffer;
    TABLE<SYMBOL_TABLE_ENTRY>& symbol_table;
    TABLE<LITERAL_TABLE_ENTRY>& literal_table;

    TRANSITION_TABLE transition_table;
    std::unordered_set<std::string>& keywords;

    int last_token_line_no = 0;
    int last_token_column_number = 0;
    int current_token_lines = 0;
    int current_token_columns = 0;

    bool loadTransitions();
    std::string getErrorStatement(STATE state, STATE new_state);

public:
    Lexer(const char* filename, TABLE<SYMBOL_TABLE_ENTRY>& symTable, TABLE<LITERAL_TABLE_ENTRY>& litTable, std::unordered_set<std::string>& kw);
    bool setBuffer(const char* filename);
    inline void transition(STATE &state, STATE &new_state, const STATE &next_state);
    TOKEN getNextToken();
    bool isEmpty();
};

int Scanner(const char *filename);
bool writeTokenToFile(std::vector<TOKEN>& token_stream, std::string& filename);

#endif // LEXER_HPP