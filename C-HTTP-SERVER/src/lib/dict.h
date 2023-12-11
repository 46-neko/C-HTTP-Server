#ifndef DICT_H
#define DICT_H

struct DictEntry
{
	void* key;
	void* value;

	struct DictEntry* left;
	struct DictEntry* right;
};

struct DictEntry* dictEntry(void* key, void* value);
struct DictEntry* addEntry(struct DictEntry* root, void* key, void* value);
struct DictEntry* search(struct DictEntry* root, void* key);
void getDictInOrder(struct DictEntry* root);

#endif // !DICT_H