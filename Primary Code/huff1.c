#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEAPSIZE 257

struct HuffmanCode {
    char ch;
    char code[256];
};
typedef struct HuffmanCode HCNODE;

struct minHeapNode {
    char ch;
    int freq;
    struct minHeapNode *llink, *rlink;
};
typedef struct minHeapNode MHNODE;


MHNODE* getElem(char ch, int freq)
{
    MHNODE* elem = (MHNODE*)malloc(sizeof(MHNODE));
    elem->ch = ch;
    elem->freq = freq;
    elem->llink = NULL;
    elem->rlink = NULL;
    return elem;
}

int heapEmpty(int count) {
    return ((count == 0) ? 1 : 0);
}

int heapFull(int count) {
    return ((count == HEAPSIZE) ? 1 : 0);
}

void pushHeap(MHNODE* heap[], MHNODE* elem, int *count)
{
    if (heapFull(*count))
    {
        printf("Heap is Full\n");
        return;
    }
    int i = ++(*count);
    while ((i != 1) && (elem->freq < heap[i / 2]->freq))
    {
        heap[i] = heap[i / 2];
        i /= 2;
    }
    heap[i] = elem;
}

MHNODE* popHeap(MHNODE* heap[], int *count)  
{
    int parent, child;
    MHNODE *elem, *temp;
    if (heapEmpty(*count))
    {
        printf("Heap is Empty\n");
        return NULL;
    }
    elem = heap[1];
    temp = heap[(*count)--];
    parent = 1;
    child = 2;
    while (child <= *count)
    {
        if ((child < *count) && (heap[child]->freq > heap[child + 1]->freq))
            child++;
        if (temp->freq <= heap[child]->freq)
            break;
        heap[parent] = heap[child];
        parent = child;
        child *= 2;
    }
    heap[parent] = temp;
    return elem;
}

MHNODE buildHuffmanTree(MHNODE* heap[], int *count)
{
    while (*count > 1)
    {
        MHNODE* left = popHeap(heap, count);
        MHNODE* right = popHeap(heap, count);

        MHNODE* internalNode = getElem('\0', left->freq + right->freq);
        internalNode->llink = left;
        internalNode->rlink = right;
        pushHeap(heap, internalNode, count);
        /*printf("\nTree Structure:\n");
        for (int i = 1; i <= *count; i++)
        {
            printf("%c\t%d\n", heap[i]->ch, heap[i]->freq);
        }
        */
    }
    return *heap[1];
}

void printHuffmanCodes(MHNODE* root, char *code, int depth, HCNODE hCodes[], int *hCodesCount)
{
    if (root == NULL)
        return;

    if ((root->llink == NULL) && (root->rlink == NULL))
    {
        code[depth] = '\0'; 
        printf("%c: %s\n", root->ch, code);
        
        hCodes[*hCodesCount].ch = root->ch;
        strcpy(hCodes[*hCodesCount].code, code);
        (*hCodesCount)++;
        
        return;
    }

    // Traverse left
    if (root->llink != NULL)
    {
        code[depth] = '0';
        printHuffmanCodes(root->llink, code, depth + 1, hCodes, hCodesCount);
    }

    // Traverse right
    if (root->rlink != NULL)
    {
        code[depth] = '1';
        printHuffmanCodes(root->rlink, code, depth + 1, hCodes, hCodesCount);
    }
}


void buildHeapFromString(MHNODE* heap[], char str[], int *count)
{
    int freqs[256] = {0};
    for (int i = 0; i < strlen(str); i++)
        freqs[(int)str[i]]++;
    for (int i = 0; i < 256; i++)
    {
        if (freqs[i] > 0)
        {
            MHNODE* elem = getElem((char)i, freqs[i]);
            pushHeap(heap, elem, count);
        }
    }
}

int calculateTreeHeight(MHNODE* root) {
    if (root == NULL)
        return 0;
    int leftHeight = calculateTreeHeight(root->llink);
    int rightHeight = calculateTreeHeight(root->rlink);
    return 1 + (leftHeight > rightHeight ? leftHeight : rightHeight);
}

void encodeAndStore(char* content, MHNODE* root, FILE* outputFile) {
    HCNODE hCodes[256];
    int hCodesCount = 0;
    char code[256]; 
    printHuffmanCodes(root, code, 0, hCodes, &hCodesCount);

    unsigned char buffer = 0;
    int bufferIndex = 0;  

    for (int i = 0; i < strlen(content); i++) {
        char ch = content[i];
        for (int j = 0; j < hCodesCount; j++) {
            if (hCodes[j].ch == ch) {
                for (int k = 0; k < strlen(hCodes[j].code); k++) {
                    buffer = (buffer << 1) | (hCodes[j].code[k] - '0');
                    bufferIndex++;
                    if (bufferIndex == 8) {
                        fwrite(&buffer, sizeof(unsigned char), 1, outputFile);
                        buffer = 0;
                        bufferIndex = 0;
                    }
                }
                break;
            }
        }
    }
    if (bufferIndex > 0) {
        buffer = buffer << (8 - bufferIndex);
        fwrite(&buffer, sizeof(unsigned char), 1, outputFile);
    }
}


void decodeAndStore(FILE* inputFile, MHNODE* root, FILE* outputFile) {
    MHNODE* currentNode = root;
    char byte;
    int bitCount = 0;

    while (fread(&byte, sizeof(char), 1, inputFile) > 0) {
        for (int i = 7; i >= 0; i--) {
            char bit = (byte >> i) & 1;

            if (bit == 0) {
                currentNode = currentNode->llink;
            } else if (bit == 1) {
                currentNode = currentNode->rlink;
            } else {
                printf("Invalid bit in encoded message!\n");
                return;
            }

            if (currentNode->llink == NULL && currentNode->rlink == NULL) {
                fputc(currentNode->ch, outputFile);
                currentNode = root;
            }
        }
    }
}


int main()
{
    char filename[100];
    char *content = NULL;
    int length = 0;
    FILE *file;

    printf("Please select a .txt file:\n");
    printf("Enter the full path of the selected .txt file: ");
    scanf("%s", filename);
    printf("%s", filename);

    file = fopen(filename, "r");
    if (file == NULL) 
    {
        printf("Error opening file!\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    printf("%d", length);

    content = (char *)malloc((length + 1) * sizeof(char));
    if (content == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    int i = 0;
    char c;
    while ((c = fgetc(file)) != EOF) {
        content[i++] = c;
    }
    content[i] = '\0';
    fclose(file);
    //printf("\nContents of the file:\n%s\n", content);
    MHNODE* heap[HEAPSIZE];
    int count = 0;
    buildHeapFromString(heap, content, &count);
    printf("\nHeap Structure:\n");
    for (int i = 1; i <= count; i++)
    {
        printf("%c\t%d\n", heap[i]->ch, heap[i]->freq);
    }

    MHNODE root = buildHuffmanTree(heap, &count);
    int height = calculateTreeHeight(&root);
    char code[height + 1];
    printf("\nSize = %ld", sizeof(code));
    printf("\nHuffman Codes:\n");

    for (int i = 1; i <= count; i++)
    {
        free(heap[i]);
    }

    FILE* outputFile = fopen("encoded_message.bin", "wb");
if (outputFile == NULL) {
    printf("Error opening output file!\n");
    return 1;
}
encodeAndStore(content, &root, outputFile);
fclose(outputFile);

FILE* inputFile = fopen("encoded_message.bin", "rb");
if (inputFile == NULL) 
{
    printf("Error opening input file!\n");
    return 1;
}
FILE* outFile = fopen("decoded_message.txt", "w");
if (outFile == NULL) 
{
    printf("Error opening output file!\n");
    fclose(inputFile);
    return 1;
}
decodeAndStore(inputFile, &root, outFile);
fclose(inputFile);
fclose(outFile);

return 0;
}
