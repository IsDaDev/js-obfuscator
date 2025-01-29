#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include <unordered_map>
#include <array>
#include <random>

using namespace std;

// function to get n random letters
string gen_random(const int len) {
    static random_device rd;
    static mt19937 mt(rd());
    static uniform_int_distribution<int> dist(0, 25);

    string result;
    for (int i = 0; i < len; ++i) {
        result += 'a' + dist(mt);
    }
    return result;
}

// checks if the provided word contains () => is a function call
bool isFunctionCall(const string& word) {
    return word.find("()") != string::npos;
}

// checks if it is a whole word and there are no letters or numbers following or infornt
bool isWholeWord(const string& word, const string& variable, const size_t& pos) {
    return (pos == 0, !isalnum(word[pos - 1])) &&(pos + variable.length() == word.length() || !isalnum(word[pos + variable.length()]));
}

// function to find and replace all the variables and function calls
string findAndReplace(string word, const unordered_map<string, string>& uniqueVars) {
    // loops through every variable
    for (const auto& variable : uniqueVars) {
        // attempts to find the looped variable
        size_t pos = word.find(variable.second);
        // checks if the variable has been found
        while (pos != string::npos) {
            // checks if the function is a function call
            bool isFunction = isFunctionCall(word) && word.substr(pos, variable.second.length() + 2) == variable.second + "()";

            // checks weather the function is a whole word or an function
            if (isWholeWord(word, variable.second, pos) || isFunction) {
                // if its a function it adds the parenthesis at the end
                if (isFunction) {
                    word.replace(pos, variable.second.length() + 2, variable.first + "()");
                } else {
                    // else it just replaces
                    word.replace(pos, variable.second.length(), variable.first);
                }
            }
            // tries to find another instance of the same variable
            pos = word.find(variable.second, pos + 1);
        }
    }
    return word;
}

// function to extract all the variables
void extractVariables(vector<vector<string>>& fileContent, unordered_map<string, string>& uniqueVars, int fuzzerLength)
{
    string prefixes[] = {"let", "const"};

    // loops through every line of fileContent
    for (const auto& line : fileContent) {
        // loops trough every word of each line
        for (size_t i = 0; i < line.size(); i++) {
            // loops through each prefix
            for (const auto& prefix : prefixes) {
                // tries to find the prefix
                if (line[i].find(prefix) != string::npos && i + 1 < line.size()) {
                    string element = line[i + 1];
                    // erases ";" and "," 
                    element.erase(remove(element.begin(), element.end(), ';'), element.end());
                    element.erase(remove(element.begin(), element.end(), ','), element.end());
                    
                    // creates a random value and inserts it into unique vars as an key:value pair
                    // checks for duplicates
                    string random;   
                    do {
                        random = gen_random(fuzzerLength);
                    } while (uniqueVars.find(random) != uniqueVars.end());

                    uniqueVars.insert({random, element});
                }
            }
        }
    }
}


int main(int argc, char const* argv[]) {
    cout << "This is a little JS fuzzer tool created by IsDaDev (www.isdadev.at).\n" 
            "Since it is only a small sideproject it does not work perfectly\n" 
            "My main motivation behind this tool is to get more familiar with C++" << endl;
    
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <in_file> <out_file>" << endl;
        return 1;
    }

    int fuzzerLength = 10;
    regex splitRegex("\\S+");
    vector<vector<string>> fileContent;
    unordered_map<string, string> uniqueVars;

    ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        cerr << "Error: Could not open file " << argv[1] << endl;
        return 1;
    }

    string buffer;
    while (getline(inputFile, buffer)) {
        auto wordsBegin = sregex_iterator(buffer.begin(), buffer.end(), splitRegex);
        auto wordsEnd = sregex_iterator();

        vector<string> line;
        for (auto it = wordsBegin; it != wordsEnd; ++it) {
            line.push_back(it->str());
        }
        fileContent.push_back(line);
    }
    inputFile.close();

    extractVariables(fileContent, uniqueVars, fuzzerLength);

    for (auto& line : fileContent) {
        for (auto& word : line) {
            if (!word.empty()) {
                word = findAndReplace(word, uniqueVars);
            }
        }
    }
    
    ofstream outFile(argv[2]);
    if (!outFile.is_open()) {
        cerr << "Error: Could not create output file" << endl;
        return 1;
    }

    for (const auto& line : fileContent) {
        string res;
        for (size_t i = 0; i < line.size(); ++i) {
            res += line[i];
            if (i < line.size() - 1) {
                res += " ";
            }
        }
        outFile << res << endl;
    }
    outFile.close();

    return 0;
}