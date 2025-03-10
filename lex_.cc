#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>
#include <functional>


#define BUFFER_SIZE 25




class LITERAL_TABLE {

};

/*state to integer maping*/
enum STATE {
    ERROR_STATE,
    Start,
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
    O_FINAL

};

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
    T_DEFAULT,
};



class TOKEN {
    size_t t_id;
    TOKEN_CLASS t_class;
    std::string t_lexeme;
    DATA_TYPE t_type;
    int t_line_number;

};

class SYMBOL_TABLE {
    std::unordered_map<std::string, TOKEN>  table;



};

class Transitions {

    std::unordered_map<char, STATE> transitions;
    std::vector<std::pair<std::function<bool(char)>, STATE>> transition_functions;

    STATE& operator [] (char character) {
        if (transitions.find(character) == transitions.end()) {

            STATE* Error_State = new STATE(ERROR_STATE);
            return *Error_State;
        } 
        return transitions[character];
    }

    
};

class TRANSITION_TABLE 
{
    std::unordered_map<STATE, Transitions> table;

public:

    Transitions& operator [] (STATE state) {
        if ( table.find(state) == table.end()){
            throw std::runtime_error("No transitions define for state");
        } 
        return table[state];
    }
};


// class to wrap the logic of Buffer Pair
class BUFFER {

private:

    char input_buffer_1[BUFFER_SIZE];
    char input_buffer_2[BUFFER_SIZE];

    char* current_buffer = nullptr;

    //pointers
    int bp;
    int fp;

    int in_file_descriptor;

    bool is_buffer1_inuse;   // boolean to keep track of current buffer

    bool loadBuffer() {
        if(isDescriptorSet()) {
            
            //set current buffer

            char* buffer = bufferOneInUse() ? input_buffer_1 : input_buffer_2;
            int bytesread = 0;
            if(read(bytesread=in_file_descriptor,buffer, BUFFER_SIZE) > 0)
            {
                //set sentinal
                buffer[bytesread] = EOF;
                return true;
            }
               
            

        }
        return false;
    }

public:

    BUFFER(const char* filename = nullptr):
        in_file_descriptor(-1), is_buffer1_inuse(true),bp(0),fp(0),current_buffer(input_buffer_1) {
        if(nullptr != filename) {
            if ( !setFile(filename)){
                std::cerr << "Error opening file: " << filename << "\n";
            }
        }
    }

    ~BUFFER(){
        close(in_file_descriptor);
    }

    bool setFile(const char* filename) {
        in_file_descriptor = open(filename, O_RDONLY);
        if (in_file_descriptor == -1) {
            return false;
        }
        return true;
    }


    bool isDescriptorSet() {
        return in_file_descriptor >= 0;
    }

    bool bufferOneInUse() {
        return is_buffer1_inuse;
    }

    

    char getNextCharacter() {

    }

    bool retract() {

    }


    char lookAheadCharacter() {

    }

    void resetBp() {
        bp = fp;
    }

    std::string getLexeme() {

        if(bufferOneInUse() ? std::string(buffer) : "";
    }

    bool isEOF() {
        
    }




};

TOKEN Lexer
    (BUFFER& buffer, 
    SYMBOL_TABLE& symbol_table, 
    TRANSITION_TABLE& transition_table, 
    std::unordered_set<std::string>& keywords) 
{



}

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

int main() {

    double a = +123.322E+123;
    std::unordered_set <std::string> Keywords {};

    TRANSITION_TABLE transaction_table;
    transaction_table[STATE::START]['_'] = 
    SYMBOL_TABLE symbol_table;


    Scanner("src.ucc");
}




