#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int);
//add additional prototypes here
void reverse_string(char *, int);
int print_words(char *, int);

int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
  char *src = user_str;
    char *dest = buff;
    int char_count = 0;
    int in_whitespace = 1;
    // input length
    while (*src != '\0') {
        char_count++;
        if (char_count > len) return -1;
        src++;
    }
    src = user_str;
    char_count = 0; 
    while (*src != '\0') {
        char c = (*src == '\t') ? ' ' : *src;
        if (c == ' ') {
            if (!in_whitespace) {
                *dest++ = ' ';
                char_count++;
                in_whitespace = 1;
            }
        } else {
            *dest++ = c;
            char_count++;
            in_whitespace = 0;
        }
        src++;
    }
    if (char_count > 0 && *(dest-1) == ' ') {
        dest--;
        char_count--;
    }
    int remaining = len - char_count;
    if (remaining > 0) {
        memset(dest, '.', remaining);
    }
    
    return char_count;
}   
//     return 0; //for now just so the code compiles. 
// }

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int str_len){
    //YOU MUST IMPLEMENT
     int word_count = 0;
    int in_word = 0;
    char *p = buff;

    for (int i = 0; i < str_len; i++) {
        if (*p == ' ') {
            in_word = 0;
        } else {
            if (!in_word) {
                word_count++;
                in_word = 1;
            }
        }
        p++;
    }
    return word_count;
    // return 0;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS
void reverse_string(char *buff, int str_len) {
    char *start = buff;
    char *end = buff + str_len - 1;
    
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}
int print_words(char *buff, int str_len) {
    int word_count = 0;
    char *current = buff;
    char *word_start = buff;
    int in_word = 0;

    printf("Word Print\n----------\n");

    for (int i = 0; i < str_len; i++) {
        if (*current == ' ') {
            if (in_word) {
                printf("%d. ", ++word_count);
                for (char *p = word_start; p < current; p++) putchar(*p);
                printf(" (%ld)\n", current - word_start);
                in_word = 0;
            }
        } else {
            if (!in_word) {
                word_start = current;
                in_word = 1;
            }
        }
        current++;
    }

    if (in_word) {
        printf("%d. ", ++word_count);
        for (char *p = word_start; p < current; p++) putchar(*p);
        printf(" (%ld)\n", current - word_start);
    }

    return word_count;
}

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    //ANS: This check is safe as it ensures that the program only proceeds 
    //if the first argument is valid.
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if sta=tement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    //ANS:The user failed to supply the required input 
    //string following the option flag if argc < 3.
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
buff = (char *)malloc(BUFFER_SZ); //malloc allocation
if(!buff){
    exit(2);
}

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
        reverse_string(buff, user_str_len);//string reversal
        printf("reversed: ");
        for(int i = 0; i<user_str_len; i++)putchar(buff[i]);
        printf("\n");
        break;
        case 'w'://word printing
            rc = print_words(buff, user_str_len);
            if (rc < 0) {
                printf("Error printing words\n");
                free(buff);
                exit(3);
            }
            break;
        case 'x':  
            if (argc != 5) {
                printf("error: -x requires 3 arguments\n");
                free(buff);
                exit(3);
            }
            printf("Not implemented!\n");
            free(buff);
            exit(3);
        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          PLACE YOUR ANSWER HERE
//It is best practice to pass both buffer and length because:
// 1. Functions can be reused since they are not required to know or hardcode buffer size.
//2. Data and padding may be present in the buffer; they must be distinguished.
//3. By determining the valid data length, it avoids buffer overflows.
//4. Makes it possible to work with smaller buffer sections.
//5. Self-documenting function interfaces.
