#ifndef __PARSER_H
#define __PARSER_H 

/**
 * @file G-2301-03-P1-parser.h
 * @details Parser function.
 * @author Enrique Cabrerizo Fernandez and Guillermo Ruiz Alvarez
.*/


/**
 * @brief Check if any of an amount of strings is part of the beginning of another string.
 * @details The parser function receives an array of strings that could be the beginning of the
 * string "msg". If "msg" starts with any of these strings, parser returns the position on the strings[] array
 * where the pointer to the beginning of msg is.
 * 
 * @param total number of strings in strings[]
 * @param strings array of strings pointers
 * @param msg string to check the beginning
 * @return If "msg" starts with any of these strings, parser returns the position on the strings[] array
 * where the pointer to the beginning of msg is.
 */
int parser(int total, char* strings[], char* msg);

#endif