#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

char* line = NULL;
char** words = NULL;
char* word = NULL;
int len = 0;
int wordsNumber = 0;
int words_len = 0;
int num = 0;
int word_len = 0;
int sum_len = 1;
int word_buffer_size = 8;

int compare( const void* a, const void* b ) {
	return( strcmp( *(char**)a, *(char**)b ) );
}

int isfree( int i ) {
	return ( ( isspace( (int)line[i] ) || line[i] == '\0' ) && !isspace( (int)line[i - 1] ) );
}

int islexeme( int i ) {
	int ans = 0;
	if( line[i] == ';' || line[i] == '&' || line[i] == '|'  )
		ans = 1;
	if( i + 1 < len && 
	( ( line[i] == '&' && line[i + 1] == '&' ) ||
	  ( line[i] == '|' && line[i + 1] == '|' ) ) )
		ans = 2;
	return ans;

}

int isquotes( int k ) {
	if( line[k] != '\'' && line[k] != '\"' )
		return 0;
	for(int i = k + 1; i < len; i++ )
		if( line[i] == line[k] )
			return (i - k);
	printf("\n Missing second quote of %c type!", line[k]);
	return 0;
}

void get_line() {
	line = (char*) malloc(8);
	int c;	
	int buffer_size = 8;
	while( (c = getchar() ) != EOF ) {
		line[len] = c;
		len++;
		if( len + 2 >= buffer_size ) {
			buffer_size *= 2;		
			line = (char*) realloc( line, buffer_size );
		}
	}
	line[len] = '\0';		
	len++;
}

void count_words() {
	for( int i = 1; i < len; ++i )
		if( isfree( i ) )
			wordsNumber++;
}

void add_char( int i ) {
	word[word_len] = line[i];
	word_len++;
	if( word_len + 2 >= word_buffer_size ) {
		word_buffer_size *= 2;
		word = (char*) realloc( word, word_buffer_size );
	}
}

void add_word() {
	if(word_len > 0) {
		word[word_len] = '\0';
		if ( words_len + word_len >= sum_len ) {			
			sum_len *= 2;
			words = realloc( words, sum_len*sizeof(char*) );
		}
		words[num] = word;
		num++;
		words_len += word_len;
		word = (char*) malloc(8);
		word_len = 0;
	}
} 

void split_and_sort() {
	word = (char*) malloc(8);
	word_buffer_size = 8;
	word_len = 0;
	if( !( isspace( (int)line[0] ) || line[0] == '\0' ) ) 
		word[word_len++] = line[0];
	int lexeme = 0;
	int quotes = 0;
	int i = 1;
	while( i < len ) {
		lexeme = islexeme(i);
		quotes = isquotes(i);
		if(quotes > 0) {
			int i0 = i;
			for( i = i0 + 1; i < i0 + quotes; i++ )
				add_char(i);
			i++;
		}
		if( isfree(i) || lexeme != 0 ) {
			add_word();	
			if( lexeme != 0 ) {
				word[0] = line[i];
				word_len = 1;
				if( lexeme == 2) {
					word[1] = line[i + 1];
					word_len = 2;
				}
				add_word();
				i += lexeme - 1;		
			}
		}
		else if ( !isspace((int)line[i]) ) 
			add_char(i);
		i++;
	}
	qsort(words, num, sizeof(char**), compare);
	printf("\n");
	for(int i = 0; i < num; i++)
		printf("\"%s\"\n", words[i]);	
}

int main() {
	get_line();
	count_words();
	split_and_sort();
	return 0;
}
