
#ifndef DOCUMENT_GENERATOR_HPP
#define DOCUMENT_GENERATOR_HPP

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

using namespace std;


class DocumentGenerator
{
public:
	unordered_map<string, unordered_map<string, int>*> dg; 
	//break a line over whitespace separators
	//caller must delete returned vector
	vector<string> * tokenize(const string & line) {
	        vector<string> * words = new vector<string>();
	        vector<char> word;
	        for (unsigned char c : line) {
	        		if (UNWANTED_CHARACTERS.find(c) != string::npos){
	        			continue;
	        		}
	                if (whitespace.find(c) == string::npos) {
	                        word.push_back(c);
	                } else{
		                if (word.size() > 0) {
		                        words->push_back(string(word.begin(), word.end()));
		                        word.clear();
		                }
		                if (PUNCTUATION.find(c) != string::npos) {
		                	words->push_back(string(c));
		                }	
	                } 
	        }
	        if (word.size() > 0) {
	                words->push_back(string(word.begin(), word.end()));
	        }
	        return words;
	}

	//load a file and return a vector of all the words in that file
	//this function only splits the contents of the file over whitespace
	//and does not make exceptions for punctuation or unwanted characters
	//caller must delete returned vector
	vector<string> * getWords(const string & fileLocation) {
	        vector<string> * words = new vector<string>();

	        ifstream infile;
	        infile.open(fileLocation);

	        if (!infile) {
	                cerr << "Couldn't open file: " << fileLocation << endl;
	                exit(1);
	        }

	        string line;
	        while(getline(infile, line)) {
	                vector<string> * wordsInLine = tokenize(line);
	                words->insert(words->end(), wordsInLine->begin(), wordsInLine->end());
	                delete wordsInLine;
	        }

	        return words;
	}
 
 /**
 * Builds a document generator from the documents in the given directory
 * This should open all files in the directory, read them in, tokenize them into words,
 * and build the datastructure from that stream of words.
 *
 * To tokenize a document, you are required to do the following,
 * in this order (or in a manner that is logically equivalent
 * to doing them in this order):
 * * remove all UNWANTED_CHARACTERS from the document
 * * split the document into different tokens based on whitespace
 *  (treat all whitespace breaks equally - newlines and spaces should be treated the same way. Whitespace is thrown away and not included in tokens. There should be no empty tokens.)
 * * additionally split each PUNCTUATION character into its own token
 *   (equivalently - treat all punctuation as if it was surrounded on both sides by a space)
 * * characters that are neither UNWANTED_CHARACTERS nor PUNCTUATION should be treated normally, the same way as any alphabetic character. This includes single apostrophes and accented characters.
 * * from here on, we'll use the word "word" to refer to all tokenized strings, such as "hello", "." or ","
 *
 * So, for instance, the short sentence
 * "I like the man's $10,000 trains. Sally jumped $ ov^er the moon, I think? I. I think."
 * Would be tokenized into ["I", "like", "the", "man's", "10", ",", "000", "trains", ".", "Sally",
 *   "jumped", "over", "the", "moon", ",", "I", "think", "?", "I", ".", "I" "think", "."]
 * and the frequencies of the words after "I" would be;
 *  like - 1
 *  think - 2
 *  . - 1
 *
 * A few notes:
 * 1) you must treat words with different capitalizatoins differently
 * * (so "foo" and "Foo" are different words, and the frequencies of the words that follow (or precede)
 * * "foo" will be different than the frequencies of the words that follow (or precede) "Foo" )
 * 2) pretend that the first word in each document is preceeded by a periood (That way, it is considered when starting any new sentence)
 */
//	{"I": {"say":5, "meet":2, "cry":1}, "you": {"meet":3, "hate":5}}
  DocumentGenerator(const string & documentsDirectory){
  	struct dirent *dp;
    const char *path= &documentsDirectory; // Directory target
    DIR *dir = opendir(path); // Open the directory - dir contains a pointer to manage the dir
    while (dp=readdir(dir)) // if dp is null, there's no more content to read
    {
        char *fileName = dp->d_name;
        string str(fileName);
        fileLocation = documentsDirectory + fileName;
        vector<string> * words = getWords(fileLocation);
        for (int i = 0; i < words->size()-1; ++i){
        	if (dg.count(words[i]) == 0){
        		dg[words[i]] = new unordered_map<string, int>;
        	}
        	*dg[words[i]][words[i+1]] ++;
        }

    }
    closedir(dir); // close the handle (pointer)
  }

  /**
 * Suppose you're in the middle of generating a document and that you've just added the word prevWord
 * to the document. generateNextWord(prevWord) should generate the next word at random for the document,
 * according to the frequencies of all the words that followed prevWord in the input documents
 *
 * So, for instance, if in all the input documents the word "foo" was followed by the word "bar" twice and "baz" 8 times,
 * then 80% of the time generateNextWord("foo") should return "baz" and 20% of the time you should return "bar".
 *
 * This method should return punctuation words like "." and "," as if they were any other word.
 * Likewise, it should be able to take punctuation words as input (generateNextWord(","))
 *
 * You can assume that prevWord is always a word that's present in one of the documents you've read in.
 */
  string generateNextWord(const string & prevWord){
  	string nextWord = ".";
  	if (dg.count(prevWord) == 0){
  		return nextWord;
  	}
  	unordered_map<string, int>* tmpMap = dg[prevWord];
  	int total = 0;
  	vector<string> * nextWords = new vector<string>;
  	vector<int> * freqs = new vector<int>;
	for (auto it : *tmpMap){
		nextWords->push_back(it.first); 
		freqs->push_back(it.second);
	    total = total + it.second;
	} 
	int randNum = rand() % total;
	int tmp = 0;
	for (int i = 0; i < nextWords->size(); i++){
		if (randNum < tmp+freqs[i]) {
			nextWord = nextWords[i];
			break;
		}
		tmp = tmp+freqs[i];
	}
	return nextWord;
  }

  /**
 * Generate a document with numWords words in it.
 *
 * See the docs on generateNextWord for a description of what it means to generate a word.
 * After generating all the words - concatenate them all into a single string representing the entire document, and return that string.
 *
 * Notes:
 * The first word you generate should be as if the previous word was '.'
 * Your document should not contain whitespace except for spaces.
 * Your should insert a single space in front of each word except:
 * * Don't insert spaces in front of punctuation marks
 * * Don't insert a space at the start of the document
 * Punctuation "words" count against this word total.
 * If you generate a word which has no successor in the documents you
 *   are mimicing (because the word only appeared at the ends of documents)
 *   generate '.' as the next word.
 *
 * The document will likely not end at the end of a sentence. That's okay.
 */
  string generateDocument(const int numWords){
  	string prevWord = ".";
  	int i = 0;
  	string document = "";
  	while (i < numWords) {
  		string next = generateNextWord(prevWord);
  		if (PUNCTUATION.find(next) == string::npos) {
  			document.append(" ");
  		}
  		document.append(next);
  		prevWord = next;
  		i++;
  	}
  	return document;
  }

  ~DocumentGenerator(){
	for (auto it : dg){
		delete it.second;
	}   	
  }

private:
  const string PUNCTUATION = ".!,?";
  const string UNWANTED_CHARACTERS = ";:\"~()[]{}\\/^_<>*=&%@$+|`";
  const string whitespace = " \t\r\n\v\f";
};

#endif //DOCUMENT_GENERATOR_HPP