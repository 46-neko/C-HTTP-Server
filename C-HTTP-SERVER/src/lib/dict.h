#ifndef DICT_H
#define DICT_H

struct DictEntry
{
	char* key;
	void* value;

	struct DictEntry* left;
	struct DictEntry* right;
};

struct DictEntry* dictEntry(char* key, void* value);
struct DictEntry* addEntry(struct DictEntry* root, char* key, void* value);
struct DictEntry* search(struct DictEntry* root, char* key);
void getDictInOrder(struct DictEntry* root);

#endif // !DICT_H