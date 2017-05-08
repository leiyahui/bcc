#include "bcc.h"

/*keywords hash*/

int get_string_key(char *str, int len)
{
	unsigned int hash_value;
	int i;

	hash_value = 0;
	for (i = 0; i < len; i++) {
		hash_value = (*str) + (hash_value << 6) + (hash_value << 16) - hash_value;
		str++;
	}

	return (hash_value & 0x7FFFFFFF);
}

int hash_key(int key_value)
{
	return key_value % KEYWORD_HASHTABLE_NUM;
}

void insert_keyword_hash(char *keyword, int token)
{
	int key_value;
	int hash_value;

	key_value = get_string_key(keyword, bcc_strlen(keyword));
	hash_value = hash_key(key_value);

	while (g_keywords_hashtable[hash_value].key != 0) {
		hash_value++;
	}

	g_keywords_hashtable[hash_value].key = key_value;
	g_keywords_hashtable[hash_value].token = token;
}

int lookup_keywords(char *str, int len)
{
	int key_value, hash_value;

	key_value = get_string_key(str, len);
	hash_value = hash_key(key_value);

	while (g_keywords_hashtable[hash_value].key != 0) {
		if (g_keywords_hashtable[hash_value].key == key_value) {
			return g_keywords_hashtable[hash_value].token;
		}
		hash_value++;
	}
	return 0;
}

void init_keywords_hash()
{
	int i;

	for (i = 0; i < KEYWORD_HASHTABLE_NUM; i++) {
		g_keywords_hashtable[i].key = 0;
		g_keywords_hashtable[i].token = 0;
	}

	insert_keyword_hash("auto", TK_AUTO);
	insert_keyword_hash("extern", TK_EXTERN);
	insert_keyword_hash("register", TK_REGISTER);
	insert_keyword_hash("static", TK_STATIC);
	insert_keyword_hash("typedef", TK_TYPEDEF);
	insert_keyword_hash("const", TK_CONST);
	insert_keyword_hash("volatile", TK_VOLATILE);
	insert_keyword_hash("void", TK_VOID);
	insert_keyword_hash("char", TK_CHAR);
	insert_keyword_hash("short", TK_SHORT);
	insert_keyword_hash("int", TK_INT);
	insert_keyword_hash("long", TK_LONG);
	insert_keyword_hash("float", TK_FLOAT);
	insert_keyword_hash("double", TK_DOUBLE);
	insert_keyword_hash("signed", TK_SIGNED);
	insert_keyword_hash("unsigned", TK_UNSIGNED);
	insert_keyword_hash("struct", TK_STRUCT);
	insert_keyword_hash("union", TK_UNION);
	insert_keyword_hash("enum", TK_ENUM);
	insert_keyword_hash("break", TK_BREAK);
	insert_keyword_hash("case", TK_CASE);
	insert_keyword_hash("continue", TK_CONTIUE);
	insert_keyword_hash("default", TK_DEFAULT);
	insert_keyword_hash("if", TK_IF);
	insert_keyword_hash("else", TK_ELSE);
	insert_keyword_hash("do", TK_DO);
	insert_keyword_hash("for", TK_FOR);
	insert_keyword_hash("while", TK_WHILE);
	insert_keyword_hash("goto", TK_GOTO);
	insert_keyword_hash("return", TK_RETURN);
	insert_keyword_hash("switch", TK_SWITCH);
}


/*identify hash*/
int hash_key_value(int key_value, int table_size)
{
	return key_value % table_size;
}
void init_hashtable(hashtable_t *hashtable, hashtable_t *parent)
{
	hashtable->ele_num = 0;
	hashtable->size = DEFAULT_HASHTABLE_SIZE;
	hashtable->table = (hash_ele_t *)bcc_calloc(DEFAULT_HASHTABLE_SIZE, sizeof(hash_ele_t));
	hashtable->parent = parent;
}

void insert_key_to_hash(hashtable_t *hashtable, char *key_value, char *str)
{
	int hash_value;
	hash_ele_t *curr_ele;

	hash_value = hash_key_value(key_value, hashtable->size);

	hashtable->ele_num++;
	curr_ele = &hashtable->table[hash_value];
	if (curr_ele->key != NULL) {
		while (curr_ele->next != NULL) {
			curr_ele = curr_ele->next;
		}
		curr_ele->next = (hash_ele_t*)bcc_malloc(sizeof(hash_ele_t));
		curr_ele = curr_ele->next;
	}
	curr_ele->key = key_value;
	curr_ele->str = str;
	curr_ele->next = NULL;
}

void resize_hashtable(hashtable_t *hashtable)
{
	int i;
	int old_table_size, new_table_size;
	hash_ele_t *curr_ele, *prev_ele;
	hash_ele_t *old_table;

	old_table_size = hashtable->size;
	old_table = hashtable->table;

	new_table_size = old_table_size * 2;

	hashtable->size = new_table_size;
	hashtable->table = (hash_ele_t *)bcc_calloc(new_table_size, sizeof(hash_ele_t));

	for (i = 0; i <= old_table_size; i++) {
		curr_ele = &old_table[i];
		while (curr_ele) {
			insert_key_to_hash(hashtable, curr_ele->key, curr_ele->str);
			curr_ele = curr_ele->next;
		}
		/*free node that additional malloc*/
		curr_ele = &old_table[i];
		curr_ele = curr_ele->next;
		while (curr_ele != NULL) {
			prev_ele = curr_ele;
			curr_ele = curr_ele->next;
			bcc_free(prev_ele);
		}
	}
	bcc_free(old_table);
}

char* insert_hash(hashtable_t *hashtable, char *str, int len)
{
	int key_value;
	int hash_value;
	hash_ele_t *curr_ele;


	if (hashtable->ele_num >= hashtable->size) {		//load factor over 1
		resize_hashtable(hashtable);
	}

	key_value = get_string_key(str, len);
	hash_value = hash_key_value(key_value, hashtable->size);

	hashtable->ele_num++;
	curr_ele = &hashtable->table[hash_value];
	if (curr_ele->key != NULL) {
		while (curr_ele->next != NULL) {
			curr_ele = curr_ele->next;
		}
		curr_ele->next = (hash_ele_t*)bcc_malloc(sizeof(hash_ele_t));
		curr_ele = curr_ele->next;
	}
	curr_ele->key = key_value;
	curr_ele->str = (char*)bcc_malloc(len + 1);
	bcc_strncpy(curr_ele->str, str, len);
	curr_ele->str[len] = '\0';
	curr_ele->next = NULL;

	return curr_ele->str;
}

char* lookup_hash(hashtable_t *hashtable, char *identify, int len)
{
	int key_value;
	int hash_value;
	hash_ele_t *curr_ident_ele;

	key_value = get_string_key(identify, len);
	hash_value = hash_key_value(key_value, hashtable->size);

	curr_ident_ele = &hashtable->table[hash_value];
	while (curr_ident_ele != NULL) {
		if (curr_ident_ele->key == key_value) {
			return curr_ident_ele->str;
		}
		curr_ident_ele = curr_ident_ele->next;
	}
	return NULL;
}

