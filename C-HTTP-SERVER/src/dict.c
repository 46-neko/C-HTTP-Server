#include <dict.h>

#include <stdlib.h>
#include <string.h>

struct DictEntry* dictEntry(char* key, void* value)
{
	struct DictEntry* dictEntry = (struct DictEntry*)malloc(sizeof(struct DictEntry));

	dictEntry->key = key;
	dictEntry->value = value;

	dictEntry->left = dictEntry->right = NULL;

	return dictEntry;
}

struct DictEntry* addEntry(struct DictEntry* root, char* key, void* value)
{
	if (!root) {
		return dictEntry(key, value);
	}

	if (strcmp(key, root->key) == 0) {
		printf("The %s value already exists! Replacing...", key);
		root->value = value;
		return root;
	}

	if (strcmp(key, root->key) > 0) {
		root->right = addEntry(root->right, key, value);
	}

	else {
		root->left = addEntry(root->left, key, value);
	}

	return root;
}

struct DictEntry* search(struct DictEntry* root, char* key)
{
	if (!root) {
		return NULL;
	}

	if (strcmp(key, root->key) == 0) {
		return root;
	}

	if (strcmp(key, root->key) > 0) {
		return search(root->right, key);
	}

	if (strcmp(key, root->key) < 0) {
		return search(root->left, key);
	}
}

void getDictInOrder(struct DictEntry* root)
{
	if (root) {
		getDictInOrder(root->left);
		printf("%s:", root->key);
		printf("\"%s\"\n", root->value);
		getDictInOrder(root->right);
	}
	return;
}