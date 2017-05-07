#ifndef HASH_H
#define HASH_H

/*keywords and identify hash*/

int get_string_key(char *str, int len);

/*keywords*/
#define KEYWORD_HASHTABLE_NUM	100

typedef struct _keyword_ele_t {
	int key;
	int token;
}keyword_ele_t;


extern keyword_ele_t g_keywords_hashtable[KEYWORD_HASHTABLE_NUM];
void init_keywords_hash();
void insert_keyword_hash(char *keyword, int token);
int lookup_keywords(char *str, int len);

/*identify hash*/
typedef struct _hash_ele_t {
	int key;
	char *str;
	struct _hash_ele_t *next;
}hash_ele_t;

typedef struct _hashtable_t {
	int size;
	int ele_num;
	hash_ele_t *table;
}hashtable_t;


#define DEFAULT_HASHTABLE_NUM 200

extern hashtable_t g_identify_hashtable;
int hash_key_value(int key_value, int table_size);
void init_hashtable(hashtable_t *hashtable);
char* insert_hash(hashtable_t *hashtable, char *str, int len);
char* lookup_hash(hashtable_t *hashtable, char *str, int len);



#endif