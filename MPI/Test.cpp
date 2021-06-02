#pragma warning(disable : 4996)
#include <conio.h>
#include <time.h>
#include <mpi.h>
#include <iostream>
#include<fstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

using namespace std;

string text_generator() {
    cout << "Put word number\n";
    int word_num;
    cin >> word_num;
    string final_text = "";
    while (word_num--) {
        int stringLen = (rand() % 10) + 1;
        string s = "";
        for (int i = 0; i < stringLen; i++) {
                s += 'a' + (rand() % 26);
        }
        final_text += s + " ";
    }
    return final_text;
}

int hashF(string word, int n)
{
    int modulo = pow(2, 31);
    int sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += ((int)abs((((int)word[n - i - 1]) * pow(10, i)))) % modulo;
    }
    return sum;
}

int main(int argc, char* argv[])
{
    int size, rank, res, must_send_in_one_msg = 100000, is_end, in_sent;
    int namelen;
    string text, part;
    int wordHash, length, textHash, text_length;
    char* char_array;
    double startwtime = 0.0, endwtime;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        text = text_generator();
        cout << "Put the word\n";
        string word;
        cin >> word;
        startwtime = MPI_Wtime();

        text_length = text.length();
        length = word.length();
        wordHash = hashF(word, length);
    }
    
    MPI_Bcast(&wordHash, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&length, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int n = 0;
    int found = 0;
    is_end = 0;
    while (is_end == 0 && size > 1){
        if (rank == 0) {
            for (int i = 1; i < size; i++) {
                if (text_length-n >= must_send_in_one_msg) {
                    in_sent = must_send_in_one_msg + length;
                    part = text.substr(n, in_sent);
                    char_array = (char*)malloc(in_sent + 1);
                    strcpy(char_array, part.c_str());

                    is_end = 0;
                    MPI_Send(&is_end, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                    MPI_Send(&in_sent, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                    MPI_Send(char_array, in_sent, MPI_CHAR, i, 0, MPI_COMM_WORLD);

                    free(char_array);

                    n += must_send_in_one_msg;

                }
                else if (text_length-n < must_send_in_one_msg && text_length-n > 0) {
                    part = text.substr(0, text_length - n);
                    char_array = (char*)malloc(text_length - n + 1);
                    strcpy(char_array, part.c_str());

                    in_sent = text_length - n;
                    is_end = 0;
                    MPI_Send(&is_end, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                    MPI_Send(&in_sent, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
                    MPI_Send(char_array, in_sent, MPI_CHAR, i, 0, MPI_COMM_WORLD);

                    free(char_array);

                    n += text_length - n;
                }
                else if (text_length - n == 0) {
                    is_end = 1;
                    for (int m = 1; m < size; m++) {
                        MPI_Send(&is_end, 1, MPI_INT, m, 1, MPI_COMM_WORLD);
                    }
                    break;
                }
                else { cout << "PROBLEMS = "<< text_length - n << endl; }
            }
        }
        else {
            MPI_Recv(&is_end, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (is_end == 0) {
                MPI_Recv(&in_sent, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                char* char_array;
                char_array = (char*)malloc(in_sent + 1);
                MPI_Recv(char_array, in_sent, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                string str(char_array);

                for (int i = 0; i < in_sent - length; i++)
                {
                    int bufferHash = hashF(str.substr(i, length), length);
                    if (bufferHash == wordHash) { found++;  }
                }
            }
        }
    }

    MPI_Reduce(&found, &res, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        endwtime = MPI_Wtime();
        cout << "The word happens " << res << "\n";
        cout << "Time " << (endwtime - startwtime);
    }
    MPI_Finalize();
    return 0;
}