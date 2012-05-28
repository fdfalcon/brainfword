// brainfword.cpp : Defines the entry point for the console application.
//

/*
 ------------------------------------------------------------------------
 Copyright (c) 2012, Francisco Falcon
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the copyright holder nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ------------------------------------------------------------------------
*/

#include "stdafx.h"
#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef __linux__
    #include <stdlib.h>
    #include <string.h>
#endif


#define MEM_SIZE 30000

unsigned char *memory = (unsigned char *)malloc(MEM_SIZE);
unsigned char *program;

unsigned long program_size;
unsigned long pointer = 0;
unsigned long pc = 0;


void clean_memory(){
    memset(memory, 0, MEM_SIZE);
}


void reset_pointer(){
    pointer = 0;
}


void free_memory(){
    if (memory){
        free(memory);
    }

    if (program){
        free(program);
    }
}


int is_balanced(unsigned char *code, unsigned long codesize){
    unsigned long counter = 0;
    unsigned char c;
    int balance = 0;

    while (counter < codesize){
        c = code[counter++];
        switch (c){
            case '[':
                balance++;
                break;
            case ']':
                if (!balance){
                    return 0;
                }
                else{
                    balance--;
                }
                break;
        }
    }
    return !balance;

}


void load_program(char *filename){
    FILE *pFile;

#ifdef _WIN32
    //fopen_s returns 0 if successful
    if (fopen_s(&pFile, filename, "rb")){
        printf("ERROR: could not open file %s, exiting...\n", filename);
        free_memory();
        exit(EXIT_FAILURE);
    }
#endif

#ifdef __linux__
    pFile = fopen(filename, "rb");
    if (!pFile){
        printf("ERROR: could not open file %s, exiting...\n", filename);
        free_memory();
        exit(EXIT_FAILURE);
    }
#endif

    /* Calculate file size */
    fseek(pFile, 0, SEEK_END);                        //seek to end of file
    program_size = ftell(pFile);                    //get current file pointer
    fseek(pFile, 0, SEEK_SET);                        //seek back to beginning of file

    program = (unsigned char*)malloc(program_size);
    if (!program){
        fclose(pFile);
        printf("ERROR: Could not allocate %d bytes to load the program from file %s.\n", program_size, filename);
        free_memory();
        exit(EXIT_FAILURE);
    }
    fread(program, 1, program_size, pFile);

    fclose(pFile);
    pc = 0;                        //Reset the program counter
}


void execute_program(){
    unsigned char instr;

    while (pc < program_size){
        instr = program[pc++];
        switch(instr){
            case '>':
                    //Avoid setting the pointer after the end of the memory buffer
                    if (pointer < MEM_SIZE - 1){
                        pointer++;
                    }
                    break;
            case '<':
                    //Avoid setting the pointer before the beginning of the memory buffer
                    if (pointer > 0){
                        pointer--;
                    }
                    break;
            case '+':
                    memory[pointer]++;
                    break;
            case '-':
                    memory[pointer]--;
                    break;
            case '.':
                    printf("%c", memory[pointer]);
                    break;
            case ',':
                    fread(&memory[pointer], 1, 1, stdin);
                    break;
            case '[':
                    //When memory[pointer] == 0, skip to the matching ']'
                    if (!memory[pointer]){
                        unsigned int balance = 0;
                        unsigned char temp_instr;
                        int found = 0;
                        while ((pc < program_size) && (!found)){
                            temp_instr = program[pc++];
                            switch (temp_instr){
                                case '[':
                                    balance++;
                                    break;
                                case ']':
                                    if (balance){
                                        balance--;
                                    }
                                    else{
                                        found = 1;
                                    }
                                    break;

                            }
                        }
                    }
                    break;
            case ']':
                    //If memory[pointer] != 0, repeat the loop by jumping back to the matching '['
                    if (memory[pointer]){
                        unsigned int balance = 0;
                        unsigned char temp_instr;
                        int found = 0;
                        pc--;                //Fetching of 'instr' made pc++ at the very beginning of the while loop
                        while ((pc > 0) && (!found)){
                            temp_instr = program[--pc];
                            switch (temp_instr){
                                case ']':
                                    balance++;
                                    break;
                                case '[':
                                    if (balance){
                                        balance--;
                                    }
                                    else{
                                        found = 1;
                                    }
                            }
                        }
                    }
                    break;

        }
    }
}


int main(int argc, char* argv[])
{
    if (argc < 2){
        printf("\nUsage: %s <brainfuck-code.txt>\n", argv[0]);
        free_memory();
        exit(EXIT_FAILURE);
    }
    clean_memory();
    reset_pointer();
    
    load_program(argv[1]);
    if (!is_balanced(program, program_size)){
        printf("ERROR: Unbalanced brackets found in the supplied Brainfuck code.\n");
        free_memory();
        exit(EXIT_FAILURE);
    }
    execute_program();
    free_memory();
    return 0;
}

