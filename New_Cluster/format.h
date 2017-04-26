/**
 * Networking
 * CS 241 - Spring 2017
 */
#pragma once
#include <stdio.h>
#include <stdlib.h>

extern const char *
    err_bad_request; // Error message to send in response to a malformed request
extern const char *err_bad_file_size; // Error message to send if the client
                                      // sends too little or too much data
extern const char *err_no_such_file; // Error message if a client tries to GET
                                     // or DELETE a non existent file

void print_client_usage();

void print_client_help();

void print_connection_closed();

void print_error_message(char *err);

void print_invalid_response();

void print_recieved_too_much_data();

void print_too_little_data();

void print_success();

void print_temp_directory(char *temp_directory);
